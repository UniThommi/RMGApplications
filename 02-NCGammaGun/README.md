# 02-NCGammaGun

Optical re-simulation of the neutron captures (NCs) of the MUSUN simulation of
[01-FullCosmogenics](../01-FullCosmogenics). The application shoots the recorded capture gammas again
from the capture position and time, with optical physics, in the SSD geometry or in a geometry with
PMTs. Every optical hit carries the muon, NC and gamma IDs, which link it to the NC truth of the first
simulation.

The physics list, the `-r` seed modes and `docs/Rerun.md` are identical copies from
[EricMEsch/RMGApplications](https://github.com/EricMEsch/RMGApplications) and are described there.

## Workflow

1. `merge_nc_data_for_gun.py` merges the NC and gamma tables of the 01-FullCosmogenics output into
   `merged_ncs.csv` and `merged_gammas.csv`. Colliding muon IDs of different runs are re-indexed;
   `--ge77-only` keeps only muons with at least one <sup>77</sup>Ge-producing NC.
2. `run.mac` sets the directory of these files (`SetMergedDir`) and selects the user-defined generator.
3. `--generator` chooses the generator:
   - `single` (`MySingleNCGammaGenerator`): one NC per event; used for all thesis simulations,
   - `muon` (`MyMuonGammaGenerator`): one muon with all its NCs per event; included for completeness.

   Both load the CSV files once into memory shared by all threads and distribute the NCs or muons over
   the threads with an atomic counter, so none is shot twice. Every gamma gets its own vertex,
   labelled with `MyPrimaryGammaUserInfo`.
4. `MySteppingAction` copies the label to the gamma tracks (`MyTrackInfo` of the
   [remage fork](https://github.com/UniThommi/remage/tree/sim/OptPhotonsPrismaSensitiveSurface)) and
   passes it on to all secondaries; the optical detector of the fork writes it to each hit.
   `DebugVertexOutputScheme` writes the vertex labels to `DebugVertices`.

Known limitation: every primary gamma takes the label of vertex 0 of its event (see the
[repository README](../README.md#known-limitation)). In single mode, muon and NC IDs are correct and only
the gamma ID of the optical hits is affected, which the downstream tools do not use.

## Usage

```sh
python merge_nc_data_for_gun.py -i /path/to/sim1 -o /path/to/merged --nested
mkdir build && cd build && cmake .. && make && cd ..
./build/FullCosmogenics -m run.mac -g gdml/currentDistZylSSD300PMTs.gdml --generator single -o ./build/
```

With `--generator muon`, use `/My/Generator/MuonGamma/SetMergedDir` in `run.mac`. The executable keeps
the name `FullCosmogenics`. The Gd cascade tables are not included, and the macros expect the geometry
directory as `./gdml` in the working directory.

The notebooks `checkSingleNCGammaGunConsistency.ipynb` and `checkMuonGammaGunConsistency.ipynb` check
the event structure of the output: event ID and NC (or muon) change together, gamma IDs are complete,
`DebugVertices` matches the remage vertex table, and no event ID is missing.
