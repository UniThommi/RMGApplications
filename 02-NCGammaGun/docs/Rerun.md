# Guide to re-running simulations with same RNG
The macro `Speedrun.sh` runs the simulation once without optical properties, prepares all the temporary files and then re-runs the simulation with Optical processes on. The macro file used to run is `rerun.mac`.
The macro `RestoreSeeds.sh` can be used to only re-run the simulation with Optical processes on, but it fully relies on all of the temporary files from `Speedrun.sh`.
The temporary files necessary to run `RestoreSeeds.sh` are:
* The corresponding `output_nt_musun_t*.csv` files which contain the primary information, expected in the folder temp/
* The corresponding random event seeds in the folder random/
* As long as both are corresponding (have been created in the same simulation) it should work
