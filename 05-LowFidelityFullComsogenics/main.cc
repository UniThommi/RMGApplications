#include "RMGHardware.hh"
#include "RMGLog.hh"
#include "RMGManager.hh"

#include "CosmogenicPhysics.hh"
#include "CustomIsotopeFilter.hh"
#include "CustomMUSUNGenerator.hh"
#include "CustomGammaGenerator.hh"
#include "HardwareQEOverride.hh"
#include "RNGTrackingAction.hh"
#include "RMGIsotopeFilterOutputScheme.hh"
#include "CosmogenicOutputScheme.hh"
#include "LowFidelityOutputScheme.hh"
#include "G4VUserEventInformation.hh"

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <nlohmann/json.hpp>

#include "CLI11.hpp"

using json = nlohmann::json;

// The names can also be hardcoded when following a strict name convention
// But as the number of rows and columns can change in the future this is better
// The PMT name needs to start with "PMT"!
// Schreibe die PMT Zuordnung zur Detektor UID in ein JSON File um hinterher zu wissen wo der PMT war der getroffen wurde.

// Struktur für PMT-Daten
struct PMTInfo {
  std::string name;
  double posX, posY, posZ;
  double rotX, rotY, rotZ;
};

// Mapping für PMT-Daten
std::map<int, PMTInfo> PMTMapping;

// Funktion zur Ausgabe der Mapping-Informationen als JSON
void WritePMTMappingToJson(const std::string& filename) {
  json jsonData;
  for (const auto& [uid, info] : PMTMapping) {
      jsonData[std::to_string(uid)] = {
          {"name", info.name},
          {"position", {
              {"x", info.posX},
              {"y", info.posY},
              {"z", info.posZ}
          }},
          {"rotation", {
              {"x", info.rotX},
              {"y", info.rotY},
              {"z", info.rotZ}
          }}
      };
  }
  std::ofstream outFile("./build/" + filename);
  outFile << jsonData.dump(4); // Schön formatierte JSON-Ausgabe
}


// PMT-Namen und Mapping auslesen und registrieren
std::vector<std::string> getPMTNamesAndRegister(const std::string& filename, RMGManager& man) {
  std::vector<std::string> PMTnames;
  std::ifstream gdmlfile(filename);
  if (!gdmlfile) {
      throw std::runtime_error("Error opening file: " + filename);
  }

  std::string key = "physvol name=\"PMT"; 
  std::string line;
  int id = 0;

  while (std::getline(gdmlfile, line)) {
      size_t pos = line.find(key);
      if (pos != std::string::npos) {
          PMTInfo pmtInfo;

          line.erase(0, pos + key.length());
          pos = line.find("0x"); 
          pmtInfo.name = "PMT" + line.substr(0, pos); 
          PMTnames.push_back(pmtInfo.name);

          // Suche Position und Rotation
          while (std::getline(gdmlfile, line) && line.find("</physvol>") == std::string::npos) {
              if (line.find("<position") != std::string::npos) {
                  sscanf(line.c_str(), "<position name=\"%*s\" unit=\"mm\" x=\"%lf\" y=\"%lf\" z=\"%lf\"/>",
                         &pmtInfo.posX, &pmtInfo.posY, &pmtInfo.posZ);
              } else if (line.find("<rotation") != std::string::npos) {
                  sscanf(line.c_str(), "<rotation name=\"%*s\" unit=\"deg\" x=\"%lf\" y=\"%lf\" z=\"%lf\"/>",
                         &pmtInfo.rotX, &pmtInfo.rotY, &pmtInfo.rotZ);
              }
          }

          PMTMapping[id] = pmtInfo;
          man.GetDetectorConstruction()->RegisterDetector(RMGHardware::kOptical, pmtInfo.name, id);
          id++;
      }
  }
  WritePMTMappingToJson("PMTs.json");
  return PMTnames;
}

int main(int argc, char **argv) {
  CLI::App app{"FullCosmogenics"};
  int nThreads = 256;
  std::string macroName;
  int rngFlag = 0;
  bool useCosmogenicOutputScheme = false;
  bool fLowFidelity = false;

  app.add_option("-m,--macro", macroName,
                 "<Geant4 macro filename> Default: None");
  app.add_option("-t, --nthreads", nThreads,
                 "<number of threads to use> Default: 256");
  app.add_option("-r,--rng", rngFlag, "RNG restoration mode: 0 deactivated, 1 for prerun, 2 for restoration run");
  app.add_flag("-c,--cosmogenic", useCosmogenicOutputScheme, "Use CosmogenicOutputScheme");
  app.add_flag("-l, --lowfidelity", fLowFidelity , "Low Fidelity Output Scheme");

  CLI11_PARSE(app, argc, argv);

  // RMGLog::SetLogLevel(RMGLog::debug);

  std::string filename = "gdml/L1000V0.gdml";

  std::string outputfilename = "build/output.hdf5";

  RMGManager man("FullCosmogenics", argc, argv);  // RMGManager ist ein singleton.
  // Overwrite the standard Hardware with one that reads
  // in the PMT QE from datasheet
  man.SetUserInit(new HardwareQEOverride());

  // Overwrite RMGPhysics to use own Optical Processes
  man.GetDetectorConstruction()->IncludeGDMLFile(filename);
  
  // Get the physical volume names of the PMTs and register them
  getPMTNamesAndRegister(filename, man);
  
  // Register the germanium volume as germanium detector.
  man.GetDetectorConstruction()->RegisterDetector(RMGHardware::kGermanium,
                                                      "Ge_phys", 1000);

  // Custom User init
  auto user_init = man.GetUserInit();
  auto *run_man = man.GetG4RunManager();

  if (rngFlag != 0) {
    user_init->AddOptionalOutputScheme<CustomIsotopeFilter>(
        "CustomIsotopeFilter");
    user_init->AddTrackingAction<RNGTrackingAction>();
    user_init->SetUserGenerator<CustomMUSUNGenerator>();
    run_man->SetNumberOfThreads(16);
    man.SetUserInit(new CosmogenicPhysics());
    if(rngFlag == 1)
      outputfilename = "build/output.csv";
    else
      outputfilename = "build/RestoredOutput.hdf5";
  }

  if (useCosmogenicOutputScheme) {
    user_init->AddOptionalOutputScheme<CosmogenicOutputScheme>("CosmogenicOutputScheme");
  }

  if (fLowFidelity) {
    std::cout << "registering low fidelity options" << std::endl;
    user_init->AddOptionalOutputScheme<LowFidelityOutputScheme>("LowFidelityOutputScheme");
    user_init->SetUserGenerator<CustomGammaGenerator>();
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
