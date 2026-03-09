#!/usr/bin/env python3
# merge_nc_data_for_gun.py
"""
Merge multiple Geant4 HDF5 output files into CSV files for particle gun.
Creates two CSV files: one for NCs, one for Gammas.
Additionally writes merge_stats.txt with key counts for downstream scripts.
"""

import h5py
import numpy as np
import glob
import argparse
from pathlib import Path
from collections import defaultdict
from tqdm import tqdm


def read_dataset_chunked(files: list[str], dataset_path: str) -> np.ndarray:
    """Read and concatenate a dataset from multiple HDF5 files."""
    all_data = []

    for file_path in tqdm(files, desc=f"Reading {dataset_path}", leave=False):
        try:
            with h5py.File(file_path, 'r') as f:
                data = f[dataset_path][:]
                all_data.append(data)
        except KeyError:
            print(f"⚠️  Warning: {dataset_path} not found in {file_path}")
            continue

    if not all_data:
        raise ValueError(f"No data found for {dataset_path}")

    return np.concatenate(all_data)


def merge_nc_files(
    input_pattern: str,
    output_dir: str,
    nested: bool = False,
    ge77_only: bool = False,
) -> tuple[int, int]:
    """Merge NC and Gamma data from multiple HDF5 files into CSVs.

    Handles muon_id collisions across runs by re-indexing only
    conflicting muon_ids. Non-conflicting data is written first,
    then conflicting data with new unique muon_ids appended.

    Returns:
        Tuple of (n_ncs, n_gammas).
    """

    input_path = Path(input_pattern)
    if input_path.is_dir():
        pattern = "run_*/output_t*.hdf5" if nested else "output_t*.hdf5"
        files = sorted(input_path.glob(pattern))
        files = [str(f) for f in files]
    else:
        files = sorted(glob.glob(input_pattern))

    if not files:
        raise ValueError(f"No files found matching: {input_pattern}")

    print(f"Found {len(files)} files to merge")

    # --- Extract run_id from file path ---
    def get_run_id(file_path: str) -> int:
        """Extract run ID from path like .../run_003/output_t0.hdf5"""
        parts = Path(file_path).parts
        for part in parts:
            if part.startswith("run_"):
                try:
                    return int(part.split("_")[1])
                except (IndexError, ValueError):
                    pass
        return 0  # default for non-nested (single run)

    # NC datasets to read
    nc_dataset_paths = {
        'muon_id': 'hit/MyNeutronCaptureOutput/evtid/pages',
        'nc_id': 'hit/MyNeutronCaptureOutput/nC_track_id/pages',
        'nc_x': 'hit/MyNeutronCaptureOutput/nC_x_position_in_m/pages',
        'nc_y': 'hit/MyNeutronCaptureOutput/nC_y_position_in_m/pages',
        'nc_z': 'hit/MyNeutronCaptureOutput/nC_z_position_in_m/pages',
        'nc_time': 'hit/MyNeutronCaptureOutput/nC_time_in_ns/pages',
        'nc_ge77': 'hit/MyNeutronCaptureOutput/nC_flag_Ge77/pages',
    }

    # Gamma datasets to read
    gamma_dataset_paths = {
        'muon_id': 'hit/CaptureGammas/evtid/pages',
        'nc_id': 'hit/CaptureGammas/nc_id/pages',
        'gamma_id': 'hit/CaptureGammas/gamma_id/pages',
        'gamma_px': 'hit/CaptureGammas/gamma_px/pages',
        'gamma_py': 'hit/CaptureGammas/gamma_py/pages',
        'gamma_pz': 'hit/CaptureGammas/gamma_pz/pages',
        'gamma_E': 'hit/CaptureGammas/gamma_E_in_keV/pages',
        'gamma_pol_x': 'hit/CaptureGammas/gamma_pol_x/pages',
        'gamma_pol_y': 'hit/CaptureGammas/gamma_pol_y/pages',
        'gamma_pol_z': 'hit/CaptureGammas/gamma_pol_z/pages',
    }

    # --- Pass 1: Read all data per run ---
    print("\n=== Reading data per run ===")
    run_data_list = []

    files_by_run: dict[int, list[str]] = defaultdict(list)
    for f in files:
        rid = get_run_id(f)
        files_by_run[rid].append(f)

    for rid in sorted(files_by_run.keys()):
        run_files = files_by_run[rid]
        print(f"\n--- Run {rid}: {len(run_files)} files ---")

        nc_data = {}
        for name, path in nc_dataset_paths.items():
            nc_data[name] = read_dataset_chunked(run_files, path)

        gamma_data = {}
        for name, path in gamma_dataset_paths.items():
            gamma_data[name] = read_dataset_chunked(run_files, path)

        run_data_list.append({
            'run_id': rid,
            'nc': nc_data,
            'gamma': gamma_data,
        })
        print(f"   {len(nc_data['nc_id'])} NCs, {len(gamma_data['gamma_id'])} gammas")

    # --- Ge77 filter ---
    if ge77_only:
        print("\n=== Applying Ge77 filter (muon-level) ===")
        for rd in run_data_list:
            nc = rd['nc']
            gamma = rd['gamma']

            # Find muon_ids that have at least one NC with ge77 flag
            ge77_mask = nc['nc_ge77'].astype(bool)
            ge77_muon_ids = set(nc['muon_id'][ge77_mask].tolist())
            n_total_muons = len(set(nc['muon_id'].tolist()))

            # Filter NCs: keep all NCs of qualifying muons
            nc_keep = np.isin(nc['muon_id'], list(ge77_muon_ids))
            for key in nc:
                nc[key] = nc[key][nc_keep]

            # Filter Gammas: keep all gammas of qualifying muons
            gamma_keep = np.isin(gamma['muon_id'], list(ge77_muon_ids))
            for key in gamma:
                gamma[key] = gamma[key][gamma_keep]

            print(f"   Run {rd['run_id']}: {len(ge77_muon_ids)}/{n_total_muons} muons pass Ge77 filter → "
                  f"{len(nc['nc_id'])} NCs, {len(gamma['gamma_id'])} gammas kept")

    # --- Pass 2: Identify collisions and write ---
    print("\n=== Resolving muon_id collisions ===")

    seen_muon_ids: set[int] = set()
    non_colliding_runs = []
    colliding_runs = []

    for rd in run_data_list:
        run_muon_ids = set(rd['nc']['muon_id'].tolist())
        colliding = run_muon_ids & seen_muon_ids

        if not colliding:
            non_colliding_runs.append(rd)
            seen_muon_ids |= run_muon_ids
        else:
            non_colliding_ids = run_muon_ids - seen_muon_ids
            seen_muon_ids |= non_colliding_ids
            colliding_runs.append((rd, colliding))
            print(f"   Run {rd['run_id']}: {len(colliding)}/{len(run_muon_ids)} muon_ids collide")

    global_max_muon_id = max(seen_muon_ids) if seen_muon_ids else -1

    # --- Write CSVs ---
    out_path = Path(output_dir)
    out_path.mkdir(parents=True, exist_ok=True)
    nc_file = out_path / 'merged_ncs.csv'
    gamma_file = out_path / 'merged_gammas.csv'

    nc_header = 'muon_id,nc_id,nc_x,nc_y,nc_z,nc_time,run_id,orig_muon_id\n'
    gamma_header = 'muon_id,nc_id,gamma_id,gamma_px,gamma_py,gamma_pz,gamma_E,gamma_pol_x,gamma_pol_y,gamma_pol_z\n'

    n_ncs = 0
    n_gammas = 0

    print(f"\n=== Writing CSV files ===")

    with open(nc_file, 'w') as f_nc, open(gamma_file, 'w') as f_gamma:
        f_nc.write(nc_header)
        f_gamma.write(gamma_header)

        def write_run(rd, muon_id_map=None):
            """Write one run's data. muon_id_map remaps colliding muon_ids."""
            nonlocal n_ncs, n_gammas
            nc = rd['nc']
            gamma = rd['gamma']
            rid = rd['run_id']

            for i in range(len(nc['nc_id'])):
                orig_mid = int(nc['muon_id'][i])
                mid = muon_id_map.get(orig_mid, orig_mid) if muon_id_map else orig_mid
                f_nc.write(
                    f"{mid},{int(nc['nc_id'][i])},"
                    f"{nc['nc_x'][i]:.10e},{nc['nc_y'][i]:.10e},"
                    f"{nc['nc_z'][i]:.10e},{nc['nc_time'][i]:.10e},"
                    f"{rid},{orig_mid}\n"
                )
                n_ncs += 1

            for i in range(len(gamma['gamma_id'])):
                orig_mid = int(gamma['muon_id'][i])
                mid = muon_id_map.get(orig_mid, orig_mid) if muon_id_map else orig_mid
                f_gamma.write(
                    f"{mid},{int(gamma['nc_id'][i])},{int(gamma['gamma_id'][i])},"
                    f"{gamma['gamma_px'][i]:.10e},{gamma['gamma_py'][i]:.10e},"
                    f"{gamma['gamma_pz'][i]:.10e},{gamma['gamma_E'][i]:.10e},"
                    f"{gamma['gamma_pol_x'][i]:.10e},{gamma['gamma_pol_y'][i]:.10e},"
                    f"{gamma['gamma_pol_z'][i]:.10e}\n"
                )
                n_gammas += 1

        # Write non-colliding runs first
        for rd in non_colliding_runs:
            write_run(rd)

        # Write colliding runs with re-indexed muon_ids
        for rd, colliding_ids in colliding_runs:
            muon_id_map = {}
            for old_id in sorted(colliding_ids):
                global_max_muon_id += 1
                muon_id_map[old_id] = global_max_muon_id
            print(f"   Run {rd['run_id']}: remapped {len(muon_id_map)} muon_ids "
                  f"(new range: {min(muon_id_map.values())}–{max(muon_id_map.values())})")
            write_run(rd, muon_id_map)

    # --- Write stats file for downstream scripts ---
    stats_file = out_path / 'merge_stats.txt'
    with open(stats_file, 'w') as f:
        f.write(f"n_ncs={n_ncs}\n")
        f.write(f"n_gammas={n_gammas}\n")
        f.write(f"ge77_filter={ge77_only}\n")

    print(f"\n✅ Wrote {n_ncs} NCs to {nc_file}")
    print(f"✅ Wrote {n_gammas} Gammas to {gamma_file}")
    print(f"📊 Wrote stats to {stats_file}")
    print(f"🎉 Success!")

    return n_ncs, n_gammas


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Merge Geant4 NC output files to CSV')
    parser.add_argument('-i', '--input', required=True, help='Input path with files "output_t*.hdf5"')
    parser.add_argument('-o', '--output', required=True, help='Output directory')
    parser.add_argument('--nested', action='store_true', help='Search in run_*/ subdirs')
    parser.add_argument('--ge77-only', action='store_true',
                help='Keep only muons where at least one NC has Ge77 flag set')
    args = parser.parse_args()

    merge_nc_files(args.input, args.output, nested=args.nested, ge77_only=args.ge77_only)