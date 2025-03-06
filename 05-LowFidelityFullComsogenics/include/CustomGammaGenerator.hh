#ifndef _CUSTOM_NEUTRON_GENERATOR_HH_
#define _CUSTOM_NEUTRON_GENERATOR_HH_

#include <filesystem>
#include <fstream>

#include "CLHEP/Units/SystemOfUnits.h"
#include "G4CsvAnalysisReader.hh"
#include "G4GenericMessenger.hh"
#include "G4ParticleGun.hh"

#include "RMGVGenerator.hh"

namespace u = CLHEP;

class G4Event;
class CustomGammaGenerator : public RMGVGenerator {

public:
  CustomGammaGenerator();
  ~CustomGammaGenerator();

  CustomGammaGenerator(CustomGammaGenerator const &) = delete;
  CustomGammaGenerator &operator=(CustomGammaGenerator const &) = delete;
  CustomGammaGenerator(CustomGammaGenerator &&) = delete;
  CustomGammaGenerator &operator=(CustomGammaGenerator &&) = delete;

  void GeneratePrimaries(G4Event *event) override;
  void SetParticlePosition(G4ThreeVector) override{};

  void BeginOfRunAction(const G4Run*) override;

private:
  void DefineCommands();
  void SetGammasFile(G4String pathToFile);

  std::unique_ptr<G4ParticleGun> fGun = nullptr;
  std::unique_ptr<G4GenericMessenger> fMessenger = nullptr;
  std::ifstream fInputFile;

  std::vector<G4int> muonIDs; 
  std::vector<G4int> neutronIDs; // Werden zur EventID
  std::vector<G4double> xs; 
  std::vector<G4double> ys; 
  std::vector<G4double> zs; 
  std::vector<G4double> pxs; 
  std::vector<G4double> pys; 
  std::vector<G4double> pzs;
  std::vector<G4double> nCTimes; 
  std::vector<G4double> eKins;
  std::vector<G4int> physVolIDs;
  std::vector<G4int> matIDs;
  std::vector<G4int> fGe77s; 
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab
