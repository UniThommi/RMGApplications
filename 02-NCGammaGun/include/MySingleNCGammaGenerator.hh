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

  static G4String fInputFilePath;
  
  // NC-indexed data structures
  static std::vector<G4int> fMuonIDs;         // MuonID (evtid) for each NC
  static std::vector<G4int> fNCIDs;           // NC IDs (nc_id/track_id)
  static std::vector<G4double> fNCx;          // NC positions
  static std::vector<G4double> fNCy;
  static std::vector<G4double> fNCz;
  static std::vector<G4double> fNCTimes;      // NC times in ns
  
  // Gamma data (all gammas)
  static std::vector<G4int> fGammaMuonIDs;    // Muon ID for each gamma
  static std::vector<G4int> fGammaNCIDs;      // Which NC does this gamma belong to
  static std::vector<G4int> fGammaIDs;        // Original gamma IDs
  static std::vector<G4double> fGammaPx;      // Momentum direction
  static std::vector<G4double> fGammaPy;
  static std::vector<G4double> fGammaPz;
  static std::vector<G4double> fGammaEnergies; // in keV
  static std::vector<G4double> fGammaPolX;    // Polarization
  static std::vector<G4double> fGammaPolY;
  static std::vector<G4double> fGammaPolZ;
  
  // Map: (MuonID, NCID) -> indices of gammas belonging to this NC
  static std::map<std::pair<G4int, G4int>, std::vector<size_t>> fNCToGammaIndices;
  static bool fDataLoaded;
};

#endif
// vim: tabstop=2 shiftwidth=2 expandtab