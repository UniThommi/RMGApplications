#!/bin/bash

# Make sure the directories exist to avoid undefined behaviour
if [[ ! -d "random" ]]; then
    echo "Error: 'random/' directory not found."
    exit 1
fi

if [[ ! -d "temp" ]]; then
    echo "Error: 'temp/' directory not found."
    exit 1
fi

if [[ ! -d "musun" ]]; then
    echo "Error: 'musun/' directory not found."
    exit 1
fi

# Step 0: Prepare new musun input file to filter only the events of interest
output_files="temp/output_nt_musun_t*.csv"
temp_file="musun/temp_musun.dat"
concatenated_file="musun/all_output_nt_musun.csv"


> "$temp_file"
> "$concatenated_file"

# Concatenate all output threads into one file
for file in $output_files; do
    if [[ -f "$file" ]]; then
        # Remove lines that start with '#' (headers) and append to the concatenated file
        grep -v '^#' "$file" >> "$concatenated_file"
    else
        echo "Warning: File $file not found."
    fi
done

echo "Concatenated all files into $concatenated_file"


# Step 1: Copy and rename files from /random directory into working directory (thanks GEANT4)
count=0
for file in random/run*evt*.rndm; do
    if [[ -f "$file" ]]; then
        new_name="run0evt${count}.rndm"
        cp "$file" "$new_name"
        #Copy the corresponding event muon to the temporary musun file
        evt_number=$(echo "$file" | grep -oP '(?<=evt)\d+')
        echo "Processing file: $file with evt_number: $evt_number"

        # Find the corresponding line in the concatenated file where evt_number matches the first column
        matching_line=$(awk -F, -v evt="$evt_number" '$1 == evt {print $0}' "$concatenated_file")

        if [[ -n "$matching_line" ]]; then
            echo "Matching line found for evt_number $evt_number: $matching_line"

            # Reformat the matching line to the desired format
            IFS=',' read -r evtid type Ekin x y z px py pz <<< "$matching_line"
            formatted_line=$(printf "%8d %2d %8.1f %8.1f %8.1f %8.1f %8.6f %8.6f %8.6f\n" "$count" "$type" "$Ekin" "$x" "$y" "$z" "$px" "$py" "$pz")

            # Write the formatted line to the temp file
            echo "$formatted_line" >> "$temp_file"
        else
            echo "No matching line found for evt_number $evt_number in $concatenated_file"
        fi


        ((count++))
    fi
done

# Step 2: Create new run file that keeps properties of rerun.mac
cp rerun.mac RestoreSeedRun.mac

# Set the newly generated (temporary) musun file as input file
sed -i "s|^/RMG/Generator/Select.*|/RMG/Generator/Select UserDefined|" RestoreSeedRun.mac
sed -i "s|^/RMG/Generator/MUSUNCosmicMuons/SetMUSUNFile.*|/Cosmogenics/Generator/SetMUSUNFile $temp_file|" RestoreSeedRun.mac
# Remove the lines of RestoreSeedRun.mac that we want different
sed -i '/^\/run\/beamOn/d' RestoreSeedRun.mac
sed -i '/^\/random\/setDirectoryName/d' RestoreSeedRun.mac
sed -i '/^\/random\/setSavingFlag/d' RestoreSeedRun.mac
sed -i 's|^/RMG/Output/ActivateOutputScheme CustomIsotopeFilter|/RMG/Output/ActivateOutputScheme IsotopeFilter|' RestoreSeedRun.mac
sed -i '/^\/RMG\/Output\/ActivateOutputScheme IsotopeFilter/a /RMG/Output/ActivateOutputScheme CosmogenicOutputScheme' RestoreSeedRun.mac
# Uncomment the optical processes
sed -i 's|^#\(/RMG/Processes/OpticalPhysics true\)|\1|' RestoreSeedRun.mac

# Add the command that reads in the seeds and only generate those events
echo "/random/resetEngineFromEachEvent true" >> RestoreSeedRun.mac
echo "/run/beamOn $count" >> RestoreSeedRun.mac

# Step 3: Start simulation
./build/FullCosmogenics -m RestoreSeedRun.mac -r 2 -c

# Step 4: Delete copied files to clean the working directory
# Leave RestoreSeedRun.mac to be able to check it for issues later
for (( i=0; i<$count; i++ )); do
    rm "run0evt${i}.rndm"
done
rm RestoreSeedRun.mac
rm "$concatenated_file"
