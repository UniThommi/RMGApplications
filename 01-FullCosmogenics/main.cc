#include "RMGHardware.hh"
#include "RMGLog.hh"
#include "RMGManager.hh"

#include "CosmogenicPhysics.hh"
#include "CustomIsotopeFilter.hh"
#include "CustomMUSUNGenerator.hh"
#include "HardwareQEOverride.hh"
#include "RNGTrackingAction.hh"
#include "RMGIsotopeFilterOutputScheme.hh"
#include "CosmogenicOutputScheme.hh"
#include "NeutronCaptureOutputScheme.hh"
#include "MyTrackInfo.hh"
#include "G4VUserEventInformation.hh"

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

#include "CLI11.hpp"

// The names can also be hardcoded when following a strict name convention
// But as the number of rows and columns can change in the future this is better
// Still the PMT name needs to start with "PMT"!
// Not needed since no PMT is in optical Map Setup.
// std::vector<std::string> getPMTNames(std::string filename) {
//   std::vector<std::string> PMTnames;
//   std::ifstream gdmlfile;
//   gdmlfile.open(filename);
//   std::string key = "physvol name=\"PMT"; // The physical volume names have this
//                                           // as indicator before them
//   if (!gdmlfile) {
//     throw std::runtime_error("Error opening file: " + filename);
//   }
//   // Search the file for a physical volume that starts with "PMT"
//   std::string line;
//   while (std::getline(gdmlfile, line)) {
//     size_t pos = line.find(key);
//     if (pos != std::string::npos) {
//       line.erase(0, pos + key.length());
//       pos = line.find("0x"); // Start of the hexadecimal pointer that will be
//                              // ignored by geant4
//       std::string name =
//           "PMT" + line.substr(0, pos); // Deleted the "PMT" out of the name
//                                        // previously so add it again
//       PMTnames.push_back(name);
//     }
//   }
//   return PMTnames;
// }

int main(int argc, char **argv) {
  CLI::App app{"Cosmogenic Simulations"};
  int nThreads = 256;
  std::string macroName;
  int rngFlag = 0;
  bool useCosmogenicOutputScheme = false;
  bool useSensitiveSurfaceOutputScheme = false;

  app.add_option("-m,--macro", macroName,
                 "<Geant4 macro filename> Default: None");
  app.add_option("-t, --nthreads", nThreads,
                 "<number of threads to use> Default: 256");
  app.add_option("-r,--rng", rngFlag, "RNG restoration mode: 0 deactivated, 1 for prerun, 2 for restoration run");
  app.add_flag("-c,--cosmogenic", useCosmogenicOutputScheme, "Use CosmogenicOutputScheme");
  app.add_flag("-n,--sensitiveSurface", useSensitiveSurfaceOutputScheme, "Use SensitiveSurfaceOutputScheme");

  CLI11_PARSE(app, argc, argv);

  // RMGLog::SetLogLevel(RMGLog::debug);

  // Anpassen!
  std::string filename = "gdml/test.gdml";

  std::string outputfilename = "build/output.hdf5";

  RMGManager man("FullCosmogenics", argc, argv);  // RMGManager ist ein singleton.
  // Overwrite the standard Hardware with one that reads
  // in the PMT QE from datasheet
  man.SetUserInit(new HardwareQEOverride());

  // Overwrite RMGPhysics to use own Optical Processes
  man.GetDetectorConstruction()->IncludeGDMLFile(filename);

  // Get the physical volume names of the PMTs to register them
  // std::vector<std::string> PMTnames = getPMTNames(filename);
  //int id = 0;
  // Register all of the PMTs
  // for (const auto &name : PMTnames) {
  //   man.GetDetectorConstruction()->RegisterDetector(RMGHardware::kOptical,
  //                                                       name, id);
  //   id++;
  // }

  // Register optical Sensitive Surface Detector

  man.GetDetectorConstruction()->RegisterDetector(RMGHardware::kOptical,
                                                      "Name", 999);
  }
  // Register the germanium volume as germanium detector.
  man.GetDetectorConstruction()->RegisterDetector(RMGHardware::kGermanium,
                                                      "Ge_phys", 1000);
                                                      //"Ge_phys", id + 1000);

  // Custom User init
  auto user_init = man.GetUserInit();
  auto *run_man = man.GetG4RunManager();

  if (rngFlag != 0) {
    user_init->AddOptionalOutputScheme<CustomIsotopeFilter>(
        "CustomIsotopeFilter");
    user_init->AddTrackingAction<RNGTrackingAction>();
    user_init->SetUserGenerator<CustomMUSUNGenerator>();
    run_man->SetNumberOfThreads(nThreads);
    man.SetUserInit(new CosmogenicPhysics());
    if(rngFlag == 1)
      outputfilename = "build/output.csv";
    else
      outputfilename = "build/RestoredOutput.hdf5";
  }

  if (useCosmogenicOutputScheme) {
    user_init->AddOptionalOutputScheme<CosmogenicOutputScheme>("CosmogenicOutputScheme");
  }

  if (useNeutronCaptureOutputScheme) {
    user_init->AddSteppingAction<MySteppingAction>();
    user_init->AddOptionalOutputScheme<NeutronCaptureOutputScheme>("NeutronCaptureOutputScheme");
  }

  // Interactive or batch mode?
  if (!macroName.empty())
    man.IncludeMacroFile(macroName);
  else
    man.SetInteractive(true);

  // Outputfilename and Threads. Then run
  
  man.SetOutputFileName(outputfilename);
  man.EnablePersistency();
  man.SetNumberOfThreads(nThreads);
  man.Initialize();
  man.Run();

  return 0;
}
// vim: tabstop=2 shiftwidth=2 expandtab
