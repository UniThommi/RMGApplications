#include "MySteppingAction.hh"

#include "RMGHardware.hh"
#include "RMGLog.hh"
#include "RMGManager.hh"

#include "CosmogenicPhysics.hh"
#include "RNGTrackingAction.hh"
#include "RMGIsotopeFilterOutputScheme.hh"
#include "G4VUserEventInformation.hh"

#include "HardwareQEOverride.hh"

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

#include "CLI11.hpp"


int main(int argc, char **argv) {
  CLI::App app{"Cosmogenic Simulations"};
  int nThreads = 256;
  std::string macroName;
  std::string gdmlFilePath;
  std::string surfaceType = "";
  std::string outputdir = "./build/";
  int rngFlag = 0;
  bool useSensitiveSurfaceOutputScheme = false;

  app.add_option("-m,--macro", macroName,
                 "<Geant4 macro filename> Default: None");
      ->required();
  app.add_option("-g,--gdml", gdmlFilePath,
                  "<Geant4 GDML filename> Default: None")
      ->required();
  app.add_option("-t, --nthreads", nThreads,
                 "<number of threads to use> Default: 256");
  app.add_option("-o, --outputdir", outputdir,
                 "<Output Directory> Default: ./build");
  app.add_option("-r,--rng", rngFlag, 
                 "RNG restoration mode: 0 deactivated, 1 for prerun, 2 for restoration run");
  app.add_option("--surface", surfaceType, "Surface type to override (SSD or PMT)")
      ->check(CLI::IsMember({"SSD", "PMT"}))
      ->required();
  app.add_flag("-s,--sensitiveSurface", useSensitiveSurfaceOutputScheme, "Use SensitiveSurfaceOutputScheme");

  CLI11_PARSE(app, argc, argv);

  // std::string outputfilename = "build/output.hdf5";

  std::string outputfilename = outputdir + std::string("/") + std::string("output.hdf5");

  RMGManager man("FullCosmogenics", argc, argv);  // RMGManager ist ein singleton.
  // Overwrite the standard Hardware with one that reads
  // in the PMT QE from datasheet
  man.SetUserInit(new HardwareQEOverride(surfaceType));

  // Overwrite RMGPhysics to use own Optical Processes
  man.GetDetectorConstruction()->IncludeGDMLFile(gdmlFilePath);

  // Custom User init
  auto user_init = man.GetUserInit();
  auto *run_man = man.GetG4RunManager();

  if (rngFlag != 0) {
    user_init->AddTrackingAction<RNGTrackingAction>();
    man.SetUserInit(new CosmogenicPhysics());
    run_man->SetNumberOfThreads(nThreads);
    if(rngFlag == 1)
      outputfilename = "build/output.csv";
    else
      outputfilename = "build/RestoredOutput.hdf5";
  }

  if (useSensitiveSurfaceOutputScheme) {
    user_init->AddSteppingAction<MySteppingAction>();
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
