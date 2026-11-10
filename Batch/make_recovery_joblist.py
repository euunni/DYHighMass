#!/usr/bin/env python3
"""
Build a recovery joblist from suspicious Condor .err file sizes.

Run this script from the directory one level above log/:
  ./make_recovery_joblist.py

It reads:
  - joblist.txt
  - log/<era>_<sample>_<id>.err

and writes:
  - joblist_recovery.txt
"""

import argparse
from pathlib import Path


ERAS = ["2016_preVFP", "2016_postVFP", "2017", "2018"]


def parse_err_name(path):
    name = path.name
    if not name.endswith(".err"):
        return None

    stem = name[:-4]
    for era in ERAS:
        prefix = f"{era}_"
        if not stem.startswith(prefix):
            continue

        rest = stem[len(prefix) :]
        sample, separator, job_id = rest.rpartition("_")
        if not separator or not sample or not job_id.isdigit():
            return None

        return era, sample, job_id

    return None


def suspicious_err_keys(log_dir, min_bad_size, max_bad_size):
    bad_keys = set()
    skipped = []

    for err_path in sorted(log_dir.glob("*.err")):
        size = err_path.stat().st_size
        if not (size <= min_bad_size or size >= max_bad_size):
            continue

        key = parse_err_name(err_path)
        if key is None:
            skipped.append(err_path.name)
            continue

        bad_keys.add(key)
        print(f"bad err: {err_path.name} size={size}")

    if skipped:
        print("Skipped err files with unrecognized names:")
        for name in skipped:
            print(f"  {name}")

    return bad_keys


def read_joblist(joblist_path):
    rows = []
    for line_number, line in enumerate(joblist_path.read_text().splitlines(), start=1):
        stripped = line.strip()
        if not stripped or stripped.startswith("#"):
            continue

        parts = stripped.split()
        if len(parts) != 4:
            raise RuntimeError(f"Malformed joblist line {line_number}: {line}")

        config, era, sample, job_id = parts
        rows.append((config, era, sample, job_id, line))

    return rows


def write_recovery_joblist(joblist_path, output_path, bad_keys):
    matched = []
    unmatched = set(bad_keys)

    for config, era, sample, job_id, original_line in read_joblist(joblist_path):
        key = (era, sample, job_id)
        if key in bad_keys:
            matched.append(original_line)
            unmatched.discard(key)

    output_path.write_text("\n".join(matched) + ("\n" if matched else ""))

    print(f"Wrote {output_path}")
    print(f"Recovery jobs: {len(matched)}")

    if unmatched:
        print("Bad err files not found in joblist:")
        for era, sample, job_id in sorted(unmatched):
            print(f"  {era} {sample} {job_id}")


def parse_args():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--joblist", default="joblist.txt", help="Original joblist path")
    parser.add_argument("--log-dir", default="log", help="Directory containing .err files")
    parser.add_argument("--output", default="joblist_recovery.txt", help="Output recovery joblist")
    parser.add_argument("--min-bad-size", type=int, default=200, help="Select err files with size <= this value")
    parser.add_argument("--max-bad-size", type=int, default=500, help="Select err files with size >= this value")
    return parser.parse_args()


def main():
    args = parse_args()
    joblist_path = Path(args.joblist)
    log_dir = Path(args.log_dir)
    output_path = Path(args.output)

    if not joblist_path.exists():
        raise RuntimeError(f"Missing joblist: {joblist_path}")
    if not log_dir.is_dir():
        raise RuntimeError(f"Missing log directory: {log_dir}")

    bad_keys = suspicious_err_keys(log_dir, args.min_bad_size, args.max_bad_size)
    write_recovery_joblist(joblist_path, output_path, bad_keys)


if __name__ == "__main__":
    main()
