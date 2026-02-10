#!/usr/bin/env python3
# merge_nc_data_for_gun.py
"""
Merge multiple Geant4 HDF5 output files into CSV files for particle gun.
Creates two CSV files: one for NCs, one for Gammas.
"""

import h5py
import numpy as np
import pandas as pd
import glob
import argparse
from pathlib import Path
from tqdm import tqdm

def read_dataset_chunked(files, dataset_path):
    """Read dataset from multiple files."""
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

def merge_nc_files(input_pattern, output_dir, nested=False):
    """Merge NC and Gamma data from multiple HDF5 files into CSVs."""
    
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
    
    # NC datasets to read
    nc_datasets = {
        'muon_id': 'hit/MyNeutronCaptureOutput/evtid/pages',
        'nc_id': 'hit/MyNeutronCaptureOutput/nC_track_id/pages',
        'nc_x': 'hit/MyNeutronCaptureOutput/nC_x_position_in_m/pages',
        'nc_y': 'hit/MyNeutronCaptureOutput/nC_y_position_in_m/pages',
        'nc_z': 'hit/MyNeutronCaptureOutput/nC_z_position_in_m/pages',
        'nc_time': 'hit/MyNeutronCaptureOutput/nC_time_in_ns/pages',
    }
    
    # Gamma datasets to read
    gamma_datasets = {
        'muon_id': 'hit/CaptureGammas/evtid/pages',
        'nc_id': 'hit/CaptureGammas/nc_id/pages',
        'gamma_id': 'hit/CaptureGammas/gamma_id/pages',
        'gamma_px': 'hit/CaptureGammas/gamma_px/pages',
        'gamma_py': 'hit/CaptureGammas/gamma_py/pages',
        'gamma_pz': 'hit/CaptureGammas/gamma_pz/pages',
        'gamma_E': 'hit/CaptureGammas/gamma_E_in_keV/pages',
        'gamma_pol_x': 'hit/CaptureGammas/gamma_pol_x/pages',
        'gamma_pol_y': 'hit/CaptureGammas/gamma_pol_y/pages',
        'gamma_pol_z': 'hit/CaptureGammas/gamma_pol_z/pages'
    }
    
    print("\n=== Reading NC Data ===")
    nc_data = {}
    for name, path in nc_datasets.items():
        nc_data[name] = read_dataset_chunked(files, path)
    
    print("\n=== Reading Gamma Data ===")
    gamma_data = {}
    for name, path in gamma_datasets.items():
        gamma_data[name] = read_dataset_chunked(files, path)
    
    n_ncs = len(nc_data['nc_id'])
    n_gammas = len(gamma_data['gamma_id'])
    
    print(f"\n✅ Loaded {n_ncs} NCs and {n_gammas} Gammas")
    
    # Create DataFrames
    df_ncs = pd.DataFrame(nc_data)
    df_gammas = pd.DataFrame(gamma_data)
    
    # Write CSVs
    output_dir = Path(output_dir)
    output_dir.mkdir(parents=True, exist_ok=True)
    
    nc_file = output_dir / 'merged_ncs.csv'
    gamma_file = output_dir / 'merged_gammas.csv'
    
    print(f"\n=== Writing CSV files ===")
    df_ncs.to_csv(nc_file, index=False, float_format='%.10e')
    print(f"✅ Wrote {n_ncs} NCs to {nc_file}")
    
    df_gammas.to_csv(gamma_file, index=False, float_format='%.10e')
    print(f"✅ Wrote {n_gammas} Gammas to {gamma_file}")
    
    print(f"\n🎉 Success!")
    print(f"   NC file: {nc_file}")
    print(f"   Gamma file: {gamma_file}")
    
    return n_ncs, n_gammas

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Merge Geant4 NC output files to CSV')
    parser.add_argument('-i', '--input', required=True, help='Input path with files "output_t*.hdf5"')
    parser.add_argument('-o', '--output', required=True, help='Output directory')
    parser.add_argument('--nested', action='store_true', help='Search in run_*/ subdirs')
    args = parser.parse_args()
    
    merge_nc_files(args.input, args.output, nested=args.nested)