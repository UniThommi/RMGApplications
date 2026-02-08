#ifndef _MY_SINGLE_NC_GAMMA_GENERATOR_HH_
#define _MY_SINGLE_NC_GAMMA_GENERATOR_HH_

#include <vector>
#include <map>
#include <atomic>
#include "CLHEP/Units/SystemOfUnits.h"
#include "G4GenericMessenger.hh"
#include "G4ParticleGun.hh"
#include "RMGVGenerator.hh"

namespace u = CLHEP;

class G4Event;

class MySingleNCGammaGenerator : public RMGVGenerator {
public:
  MySingleNCGammaGenerator();
  ~MySingleNCGammaGenerator();
  
  MySingleNCGammaGenerator(MySingleNCGammaGenerator const &) = delete;
  MySingleNCGammaGenerator &operator=(MySingleNCGammaGenerator const &) = delete;
  MySingleNCGammaGenerator(MySingleNCGammaGenerator &&) = delete;
  MySingleNCGammaGenerator &operator=(MySingleNCGammaGenerator &&) = delete;

  void GeneratePrimaries(G4Event *event) override;
  void SetParticlePosition(G4ThreeVector) override {};
  void BeginOfRunAction(const G4Run*) override;

private:
  static std::atomic<G4int> fGlobalNCIndex;  // Shared across threads
  void DefineCommands();
  void SetMergedNCDir(G4String pathToDir);
  void LoadNCData();

  std::unique_ptr<G4GenericMessenger> fMessenger = nullptr;

  G4String fInputFilePath;
  
  // NC-indexed data structures
  std::vector<G4int> fMuonIDs;         // MuonID (evtid) for each NC
  std::vector<G4int> fNCIDs;           // NC IDs (nc_id/track_id)
  std::vector<G4double> fNCx;          // NC positions
  std::vector<G4double> fNCy;
  std::vector<G4double> fNCz;
  std::vector<G4double> fNCTimes;      // NC times in ns
  
  // Gamma data (all gammas)
  std::vector<G4int> fGammaMuonIDs;    // Muon ID for each gamma
  std::vector<G4int> fGammaNCIDs;      // Which NC does this gamma belong to
  std::vector<G4int> fGammaIDs;        // Original gamma IDs
  std::vector<G4double> fGammaPx;      // Momentum direction
  std::vector<G4double> fGammaPy;
  std::vector<G4double> fGammaPz;
  std::vector<G4double> fGammaEnergies; // in keV
  std::vector<G4double> fGammaPolX;    // Polarization
  std::vector<G4double> fGammaPolY;
  std::vector<G4double> fGammaPolZ;
  
  // Map: (MuonID, NCID) -> indices of gammas belonging to this NC
  std::map<std::pair<G4int, G4int>, std::vector<size_t>> fNCToGammaIndices;
};

#endif
// vim: tabstop=2 shiftwidth=2 expandtab