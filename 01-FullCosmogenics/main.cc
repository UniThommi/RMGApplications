#include "RMGHardware.hh"
#include "RMGLog.hh"
#include "RMGManager.hh"

#include "CosmogenicPhysics.hh"
#include "CustomIsotopeFilter.hh"
#include "CustomMUSUNGenerator.hh"
#include "HardwareQEOverride.hh"
#include "RNGTrackingAction.hh"
#include "CosmogenicOutputScheme.hh"
#include "NeutronsOutputScheme.hh"

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

#include "CLI11.hpp"

// The names can also be hardcoded when following a strict name convention
// But as the number of rows and columns can change in the future this is better
// Still the PMT name needs to start with "PMT"!
std::vector<std::string> getPMTNames(std::string filename) {
  std::vector<std::string> PMTnames;
  std::ifstream gdmlfile;
  gdmlfile.open(filename);
  std::string key = "physvol name=\"PMT"; // The physical volume names have this
                                          // as indicator before them
  if (!gdmlfile) {
    throw std::runtime_error("Error opening file: " + filename);
  }
  // Search the file for a physical volume that starts with "PMT"
  std::string line;
  while (std::getline(gdmlfile, line)) {
    size_t pos = line.find(key);
    if (pos != std::string::npos) {
      line.erase(0, pos + key.length());
      pos = line.find("0x"); // Start of the hexadecimal pointer that will be
                             // ignored by geant4
      std::string name =
          "PMT" + line.substr(0, pos); // Deleted the "PMT" out of the name
                                       // previously so add it again
      PMTnames.push_back(name);
    }
  }
  return PMTnames;
}

int main(int argc, char **argv) {
  CLI::App app{"Cosmogenic Simulations"};
  int nthreads = 16;
  std::string macroName;
  int rngFlag = 0;
  bool useCosmogenicOutputScheme = false;
  bool useNeutronsOutputScheme = false;

  app.add_option("-m,--macro", macroName,
                 "<Geant4 macro filename> Default: None");
  app.add_option("-t, --nthreads", nthreads,
                 "<number of threads to use> Default: 16");
  app.add_option("-r,--rng", rngFlag, "RNG restoration mode: 0 deactivated, 1 for prerun, 2 for restoration run");
  app.add_flag("-c,--cosmogenic", useCosmogenicOutputScheme, "Use CosmogenicOutputScheme");
  app.add_flag("-n,--neutrons", useNeutronsOutputScheme, "Use NeutronsOutputScheme");

  CLI11_PARSE(app, argc, argv);

  // RMGLog::SetLogLevel(RMGLog::debug);

  std::string filename = "gdml/L1000V0.gdml";

  std::string outputfilename = "build/output.hdf5";

  RMGManager manager("FullCosmogenics", argc, argv);
  // Overwrite the standard Hardware with one that reads
  // in the PMT QE from datasheet
  manager.SetUserInit(new HardwareQEOverride());

  // Overwrite RMGPhysics to use own Optical Processes
  manager.GetDetectorConstruction()->IncludeGDMLFile(filename);

  // Get the physical volume names of the PMTs to register them
  std::vector<std::string> PMTnames = getPMTNames(filename);
  int id = 0;
  // Register all of the PMTs
  for (const auto &name : PMTnames) {
    manager.GetDetectorConstruction()->RegisterDetector(RMGHardware::kOptical,
                                                        name, id);
    id++;
  }
  // Register the germanium volume as germanium detector.
  manager.GetDetectorConstruction()->RegisterDetector(RMGHardware::kGermanium,
                                                      "Ge_phys", id + 1000);

  // Custom User init
  auto user_init = manager.GetUserInit();
  if (rngFlag != 0) {
    user_init->AddOptionalOutputScheme<CustomIsotopeFilter>(
        "CustomIsotopeFilter");
    user_init->AddTrackingAction<RNGTrackingAction>();
    user_init->SetUserGenerator<CustomMUSUNGenerator>();
    auto *RunManager = manager.GetG4RunManager();
    RunManager->SetNumberOfThreads(16);
    manager.SetUserInit(new CosmogenicPhysics());
    if(rngFlag == 1)
      outputfilename = "build/output.csv";
    else
      outputfilename = "build/RestoredOutput.hdf5";
  }

  if (useCosmogenicOutputScheme) {
    user_init->AddOptionalOutputScheme<CosmogenicOutputScheme>("CosmogenicOutputScheme");
  }

  if (useNeutronsOutputScheme) {
    user_init->AddOptionalOutputScheme<NeutronsOutputScheme>("NeutronsOutputScheme");
  }

  // Interactive or batch mode?
  if (!macroName.empty())
    manager.IncludeMacroFile(macroName);
  else
    manager.SetInteractive(true);

  // Outputfilename and Threads. Then run
  
  manager.SetOutputFileName(outputfilename);
  manager.SetNumberOfThreads(16);
  manager.Initialize();
  manager.Run();

  return 0;
}
// vim: tabstop=2 shiftwidth=2 expandtab
