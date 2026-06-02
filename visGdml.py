#!/usr/bin/env python3

import argparse
from pathlib import Path

import pyg4ometry


def main():
    parser = argparse.ArgumentParser(
        description="Visualize a GDML geometry using pyg4ometry."
    )
    parser.add_argument(
        "gdml_file",
        type=Path,
        help="Path to the GDML file."
    )

    args = parser.parse_args()

    if not args.gdml_file.exists():
        parser.error(f"File does not exist: {args.gdml_file}")

    reader = pyg4ometry.gdml.Reader(str(args.gdml_file))

    world_volume = reader.getRegistry().getWorldVolume()

    viewer = pyg4ometry.visualisation.VtkViewerColouredMaterial(
        defaultColour="random"
    )

    # Increase visual detail
    viewer.sphereResolution = 100
    viewer.cylinderResolution = 100
    viewer.coneResolution = 100
    viewer.torusResolution = 100

    viewer.addLogicalVolume(world_volume)
    viewer.view()


if __name__ == "__main__":
    main()