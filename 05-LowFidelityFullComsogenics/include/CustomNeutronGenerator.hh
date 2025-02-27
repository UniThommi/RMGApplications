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
class CustomNeutronGenerator : public RMGVGenerator {

public:
  CustomNeutronGenerator();
  ~CustomNeutronGenerator();

  CustomNeutronGenerator(CustomNeutronGenerator const &) = delete;
  CustomNeutronGenerator &operator=(CustomNeutronGenerator const &) = delete;
  CustomNeutronGenerator(CustomNeutronGenerator &&) = delete;
  CustomNeutronGenerator &operator=(CustomNeutronGenerator &&) = delete;

  void GeneratePrimaries(G4Event *event) override;
  void SetParticlePosition(G4ThreeVector) override{};

  void BeginOfRunAction(const G4Run*) override;

private:
  void DefineCommands();
  void SetNeutronsFile(G4String pathToFile);

  std::unique_ptr<G4ParticleGun> fGun = nullptr;
  std::unique_ptr<G4GenericMessenger> fMessenger = nullptr;
  std::ifstream fInputFile;

  std::vector<G4int> evtid; 
  std::vector<G4double> x; 
  std::vector<G4double> y; 
  std::vector<G4double> z; 
  std::vector<G4double> px; 
  std::vector<G4double> py; 
  std::vector<G4double> pz; 
  std::vector<G4double> eKin;
  std::vector<G4int> physVolID;
  std::vector<G4int> matID;
  std::vector<G4int> fGe77; 
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab
