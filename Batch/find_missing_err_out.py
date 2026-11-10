#!/usr/bin/env python3
"""
Find Condor jobs that have a .log file but are missing the matching .err or .out.

Run this script from the directory one level above log/:
  ./find_missing_err_out.py
"""

import argparse
from pathlib import Path


def find_missing_pairs(log_dir):
    missing_pairs = []

    for log_path in sorted(log_dir.glob("*.log")):
        missing = []
        for suffix in ("err", "out"):
            if not log_path.with_suffix(f".{suffix}").exists():
                missing.append(suffix)

        if missing:
            missing_pairs.append((log_path.stem, missing))

    return missing_pairs


def parse_args():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--log-dir", default="log", help="Directory containing .log/.err/.out files")
    return parser.parse_args()


def main():
    args = parse_args()
    log_dir = Path(args.log_dir)

    if not log_dir.is_dir():
        raise RuntimeError(f"Missing log directory: {log_dir}")

    missing_pairs = find_missing_pairs(log_dir)
    if not missing_pairs:
        print("All .log files have matching .err and .out files.")
        return

    print("Log files missing matching err/out files:")
    for stem, missing in missing_pairs:
        print(f"{stem}.log  missing: {', '.join(missing)}")

    print(f"Total log files with missing pairs: {len(missing_pairs)}")


if __name__ == "__main__":
    main()
