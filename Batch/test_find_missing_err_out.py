#!/usr/bin/env python3

import tempfile
import unittest
from pathlib import Path

from find_missing_err_out import find_missing_pairs


class FindMissingErrOutTest(unittest.TestCase):
    def test_finds_log_files_missing_err_or_out(self):
        with tempfile.TemporaryDirectory() as tmpdir:
            log_dir = Path(tmpdir)

            for name in [
                "complete.log",
                "complete.err",
                "complete.out",
                "missing_err.log",
                "missing_err.out",
                "missing_out.log",
                "missing_out.err",
                "missing_both.log",
            ]:
                (log_dir / name).write_text("")

            self.assertEqual(
                find_missing_pairs(log_dir),
                [
                    ("missing_both", ["err", "out"]),
                    ("missing_err", ["err"]),
                    ("missing_out", ["out"]),
                ],
            )


if __name__ == "__main__":
    unittest.main()
