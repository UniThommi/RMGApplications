#ifndef _CUSTOM_MUSUN_GENERATOR_HH_
#define _CUSTOM_MUSUN_GENERATOR_HH_

#include <filesystem>
#include <fstream>

#include "CLHEP/Units/SystemOfUnits.h"
#include "G4CsvAnalysisReader.hh"
#include "G4GenericMessenger.hh"
#include "G4ParticleGun.hh"

#include "RMGVGenerator.hh"

namespace u = CLHEP;

class G4Event;
class CustomMUSUNGenerator : public RMGVGenerator {

public:
  CustomMUSUNGenerator();
  ~CustomMUSUNGenerator();

  CustomMUSUNGenerator(CustomMUSUNGenerator const &) = delete;
  CustomMUSUNGenerator &operator=(CustomMUSUNGenerator const &) = delete;
  CustomMUSUNGenerator(CustomMUSUNGenerator &&) = delete;
  CustomMUSUNGenerator &operator=(CustomMUSUNGenerator &&) = delete;

  void GeneratePrimaries(G4Event *event) override;
  void SetParticlePosition(G4ThreeVector) override{};

private:
  void DefineCommands();
  void SetMUSUNFile(G4String pathToFile);

  std::unique_ptr<G4ParticleGun> fGun = nullptr;
  std::unique_ptr<G4GenericMessenger> fMessenger = nullptr;
  std::ifstream fInputFile;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab
