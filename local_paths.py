#!/usr/bin/env python3
"""
Machine-specific paths, kept out of the repository.

The notebooks and the debugging scripts default to directories on the author's
machines. They only contain the part of these paths below a root directory; the
root directories are read from ``local_paths.json`` in the repository root, which
is git-ignored (format: ``local_paths.example.json``):

    {"home": "/path/to/code/directory", "cfs": "/path/to/project/directory"}

Shell scripts use the command-line form, which prints the full path:

    python3 local_paths.py <root> <subpath>
"""

from __future__ import annotations

import json
import sys
from pathlib import Path

LOCAL_PATHS_FILE = Path(__file__).resolve().parent / "local_paths.json"


def local_path(root: str, subpath: str) -> str | None:
    """Return ``<root directory>/<subpath>``, or None if ``root`` is not configured.

    Args:
        root: Key in ``local_paths.json`` ("home" or "cfs").
        subpath: Path below the root directory, with "/" separators.
    """
    if not LOCAL_PATHS_FILE.is_file():
        return None
    roots = json.loads(LOCAL_PATHS_FILE.read_text(encoding="utf-8"))
    base = roots.get(root)
    if not base:
        return None
    return base.rstrip("/") + "/" + subpath.lstrip("/")


def required_local_path(root: str, subpath: str) -> str:
    """Like :func:`local_path`, but raise an error if ``root`` is not configured."""
    path = local_path(root, subpath)
    if path is None:
        raise RuntimeError(
            f"No '{root}' directory configured: create {LOCAL_PATHS_FILE} "
            f"(see local_paths.example.json)."
        )
    return path


if __name__ == "__main__":
    if len(sys.argv) != 3:
        sys.exit("Usage: python3 local_paths.py <root> <subpath>")
    try:
        print(required_local_path(sys.argv[1], sys.argv[2]))
    except RuntimeError as err:
        sys.exit(str(err))
