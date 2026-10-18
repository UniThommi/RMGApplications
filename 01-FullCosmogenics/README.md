# 01-FullCosmogenics

Simulation of cosmic muons (MUSUN) or homogeneously distributed neutrons in the LEGEND-1000 water tank
that records the truth of every primary neutron capture (NC). It produces the MUSUN simulation (sim1)
whose NCs are re-simulated optically by [02-NCGammaGun](../02-NCGammaGun), and, with optical physics,
the homogeneous-NC data with NC truth and optical photons in the same file.

The application was started in [EricMEsch/RMGApplications](https://github.com/EricMEsch/RMGApplications);
the upstream physics list with RNG-isolated Cherenkov light, the `-r` seed modes and their documentation
(`docs/Rerun.md`) are described there. This README covers the NC recording added on this branch.

## NC recording (`-s`)

- `MySteppingAction` detects neutron captures (`nCapture`, or `RMGnCapture` with the remage Grabmayr
  gamma cascades). For each primary capture it collects all capture gammas, checks whether
  <sup>77</sup>Ge was produced and buffers the data for the output schemes. It labels the neutron, its
  gammas and all their secondaries with (muon ID, NC ID, gamma ID) via `MyTrackInfo` of the
  [remage fork](https://github.com/UniThommi/remage/tree/sim/OptPhotonsPrismaSensitiveSurface), so
  optical hits can be assigned to their capture. Neutrons that already carry a label descend from a
  recorded capture and are not recorded again.
- `MyNeutronCaptureOutputScheme` writes one row per NC to `MyNeutronCaptureOutput` (position, time,
  volume and material ID, gamma multiplicity and energy, <sup>77</sup>Ge flag, four most energetic
  gammas) and the ID-to-name tables `physVolumes` and `materials`.
- `MyGammaCaptureOutputScheme` writes every capture gamma to `CaptureGammas` (direction, energy,
  polarisation); these are the gammas that 02-NCGammaGun shoots again.

The muon ID is the event ID: each MUSUN event contains one muon.

## Macros

- `run.mac`: MUSUN simulation without optical physics; the generator block for homogeneously
  distributed neutrons is included as a commented alternative. The NC output schemes it activates are
  only registered with `-s`.
- `interactive.mac`: OpenGL view of optical photons shot onto the wall test geometry.

## Usage

```sh
mkdir build && cd build && cmake .. && make && cd ..
./build/FullCosmogenics -m run.mac -g gdml/currentDistZylSSD300PMTs.gdml -t 256 -o ./build/ -s
```

The MUSUN file `musun/combined_file.dat` and the Gd cascade tables are not included. The macros expect
the geometry directory as `./gdml` in the working directory. See the [repository README](../README.md)
for the requirements and the full workflow.

Other files: `analyze_unique_NCs.ipynb` counts the unique (event ID, NC track ID) pairs in the output;
`scene_l1000.json` is a viewer scene with local file paths; `debugging/gdb_container.sh` runs gdb in
the remage Apptainer image.
