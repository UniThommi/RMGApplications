#include "MySteppingAction.hh"
#include "MyPrimaryGammaUserInfo.hh"
#include "MySingleNCGammaGenerator.hh"
#include "MyMuonGammaGenerator.hh"
#include "DebugVertexOutputScheme.hh"

#include "RMGHardware.hh"
#include "RMGLog.hh"
#include "RMGManager.hh"

#include "CosmogenicPhysics.hh"
#include "RNGTrackingAction.hh"
#include "RMGIsotopeFilterOutputScheme.hh"
#include "G4VUserEventInformation.hh"

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
  std::string outputdir = "./build/";
  int rngFlag = 0;
  std::string generatorMode;  // "single" or "muon"
  bool useSensitiveSurfaceOutputScheme = false;

  app.add_option("-m,--macro", macroName,
                 "<Geant4 macro filename> Default: None")
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
  app.add_option("--generator", generatorMode,
                 "<Generator mode> 'single' for SingleNCGamma, 'muon' for MuonGamma")
      ->required();

  CLI11_PARSE(app, argc, argv);

  // std::string outputfilename = "build/output.hdf5";

  std::string outputfilename = outputdir + std::string("/") + std::string("output.hdf5");

  RMGManager man("FullCosmogenics", argc, argv);  // RMGManager ist ein singleton.
  // Overwrite the standard Hardware with one that reads
  // in the PMT QE from datasheet

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

  user_init->AddSteppingAction<MySteppingAction>();
  user_init->AddOptionalOutputScheme<DebugVertexOutputScheme>("DebugVertexOutputScheme");

  // Registriere Gamma Gun
  if (generatorMode == "single") {
    user_init->SetUserGenerator<MySingleNCGammaGenerator>();
    RMGLog::Out(RMGLog::summary, "Using SingleNCGammaGenerator");
  } else if (generatorMode == "muon") {
    user_init->SetUserGenerator<MyMuonGammaGenerator>();
    RMGLog::Out(RMGLog::summary, "Using MuonGammaGenerator");
  } else {
    throw std::runtime_error("Unknown generator mode: " + generatorMode + ". Use 'single' or 'muon'.");
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
