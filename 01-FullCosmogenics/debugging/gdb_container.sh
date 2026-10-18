#!/bin/bash
# Runs gdb inside the Apptainer image remage-base.sif; all arguments are passed to gdb.
# The image path is built from the "home" root in local_paths.json (repository root,
# git-ignored; format: local_paths.example.json).
SIF=$(python3 "$(dirname "$0")/../../local_paths.py" home sim/remage02/remage-base.sif) || exit 1
apptainer exec "$SIF" gdb "$@"
