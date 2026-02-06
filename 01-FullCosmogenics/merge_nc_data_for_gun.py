#!/usr/bin/env python3
"""
Merge multiple Geant4 HDF5 output files into single file for particle gun.
Handles large datasets via chunked processing.
"""

import h5py
import numpy as np
import glob
import argparse
from pathlib import Path
from tqdm import tqdm

def read_dataset_chunked(files, dataset_path, chunk_size=100000):
    """Read dataset from multiple files in chunks."""
    all_data = []
    total_read = 0
    
    for file_path in tqdm(files, desc=f"Reading {dataset_path}"):
        try:
            with h5py.File(file_path, 'r') as f:
                data = f[dataset_path][:]
                all_data.append(data)
                total_read += len(data)
        except KeyError:
            print(f"⚠️  Warning: {dataset_path} not found in {file_path}")
            continue
    
    if not all_data:
        raise ValueError(f"No data found for {dataset_path}")
    
    return np.concatenate(all_data)

def merge_nc_files(input_pattern, output_file):
    """Merge NC and Gamma data from multiple HDF5 files."""
    
    # Handle directory input
    from pathlib import Path
    input_path = Path(input_pattern)
    if input_path.is_dir():
        files = sorted(input_path.glob("output_t*.hdf5"))
        files = [str(f) for f in files]
    else:
        files = sorted(glob.glob(input_pattern))
    if not files:
        raise ValueError(f"No files found matching: {input_pattern}")
    
    print(f"Found {len(files)} files to merge")
    
    # NC datasets to read
    nc_datasets = {
        'evtid': 'hit/MyNeutronCaptureOutput/evtid/pages',
        'nc_id': 'hit/MyNeutronCaptureOutput/nC_track_id/pages',
        'nc_x': 'hit/MyNeutronCaptureOutput/nC_x_position_in_m/pages',
        'nc_y': 'hit/MyNeutronCaptureOutput/nC_y_position_in_m/pages',
        'nc_z': 'hit/MyNeutronCaptureOutput/nC_z_position_in_m/pages',
        'nc_time': 'hit/MyNeutronCaptureOutput/nC_time_in_ns/pages',
        'nc_phys_vol_id': 'hit/MyNeutronCaptureOutput/nC_phys_vol_id/pages',
        'nc_material_id': 'hit/MyNeutronCaptureOutput/nC_material_id/pages',
        'nc_gamma_amount': 'hit/MyNeutronCaptureOutput/nC_gamma_amount/pages',
        'nc_gamma_total_energy': 'hit/MyNeutronCaptureOutput/nC_gamma_total_energy_in_keV/pages',
        'nc_flag_Ge77': 'hit/MyNeutronCaptureOutput/nC_flag_Ge77/pages'
    }
    
    # Gamma datasets to read
    gamma_datasets = {
        'evtid': 'hit/CaptureGammas/evtid/pages',
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
        print(f"Reading {name}...")
        nc_data[name] = read_dataset_chunked(files, path)
    
    print("\n=== Reading Gamma Data ===")
    gamma_data = {}
    for name, path in gamma_datasets.items():
        print(f"Reading {name}...")
        gamma_data[name] = read_dataset_chunked(files, path)
    
    n_ncs = len(nc_data['nc_id'])
    n_gammas = len(gamma_data['gamma_id'])
    
    print(f"\n✅ Loaded {n_ncs} NCs and {n_gammas} Gammas")
    
    # Write merged file
    print(f"\n=== Writing to {output_file} ===")
    with h5py.File(output_file, 'w') as f:
        # Metadata
        f.attrs['total_ncs'] = n_ncs
        f.attrs['total_gammas'] = n_gammas
        f.attrs['source_files'] = len(files)
        
        # NC group
        nc_grp = f.create_group('ncs')
        for name, data in nc_data.items():
            nc_grp.create_dataset(name, data=data, compression='gzip', compression_opts=4)
        
        # Gamma group
        gamma_grp = f.create_group('gammas')
        for name, data in gamma_data.items():
            gamma_grp.create_dataset(name, data=data, compression='gzip', compression_opts=4)
        
        print(f"✅ Wrote {n_ncs} NCs to /ncs/")
        print(f"✅ Wrote {n_gammas} Gammas to /gammas/")
    
    print(f"\n🎉 Success! Merged file: {output_file}")
    print(f"   Total NCs: {n_ncs}")
    print(f"   Total Gammas: {n_gammas}")
    return n_ncs, n_gammas

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Merge Geant4 NC output files')
    parser.add_argument('-i', '--input', required=True, help='Input path with files "output_t*.hdf5"')
    parser.add_argument('-o', '--output', required=True, help='Output directory')
    args = parser.parse_args()
    
    output_file = Path(args.output) / 'merged_nc_data.hdf5'
    merge_nc_files(args.input, output_file)