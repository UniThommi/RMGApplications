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

  CustomNeutronenerator(CustomNeutronGenerator const &) = delete;
  CustomNeutronenerator &operator=(CustomNeutronGenerator const &) = delete;
  CustomNeutronenerator(CustomNeutronGenerator &&) = delete;
  CustomNeutronenerator &operator=(CustomNeutronGenerator &&) = delete;

  void GeneratePrimaries(G4Event *event) override;
  void SetParticlePosition(G4ThreeVector) override{};

private:
  void DefineCommands();
  void SetNeutronFile(G4String pathToFile);

  std::unique_ptr<G4ParticleGun> fGun = nullptr;
  std::unique_ptr<G4GenericMessenger> fMessenger = nullptr;
  std::ifstream fInputFile;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab
