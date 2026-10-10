#!/usr/bin/env python3

import sys
import re
from pathlib import Path

INCLUDE_REGEX = re.compile(r'^\s*#\s*include\s*[<"]([^">]+)[">]')

EXTENSIONS = {'.c', '.cc', '.cpp', '.h', '.hpp'}

# Third-party and generated code is not ours to police.
IGNORED_DIRS = {'subprojects', 'build', 'dist'}

def check_file(path: Path) -> int:
    errors = 0
    with path.open("r", encoding="utf-8", errors="ignore") as f:
        for lineno, line in enumerate(f, 1):
            match = INCLUDE_REGEX.match(line)
            if not match:
                continue

            include_path = match.group(1)
            parts = include_path.replace("\\", "/").split("/")

            if ".." in parts:
                print(
                    f"ERROR: Parent directory include detected\n"
                    f"  File: {path}\n"
                    f"  Line: {lineno}\n"
                    f"  Include: {include_path}",
                    file=sys.stderr
                )
                errors += 1
    return errors


def main() -> int:
    if len(sys.argv) != 2:
        print(f"Usage: {sys.argv[0]} <directory>", file=sys.stderr)
        return 2

    root = Path(sys.argv[1])
    if not root.is_dir():
        print(f"ERROR: Not a directory: {root}", file=sys.stderr)
        return 2

    total_errors = 0
    for path in root.rglob('*'):
        if path.suffix in EXTENSIONS and not IGNORED_DIRS.intersection(path.relative_to(root).parts):
            total_errors += check_file(path)

    return 1 if total_errors > 0 else 0


if __name__ == "__main__":
    sys.exit(main())
