#ifndef _MY_MUON_GAMMA_GENERATOR_HH_
#define _MY_MUON_GAMMA_GENERATOR_HH_

#include <vector>
#include <map>
#include <atomic>
#include "CLHEP/Units/SystemOfUnits.h"
#include "G4GenericMessenger.hh"
#include "G4ParticleGun.hh"
#include "RMGVGenerator.hh"

namespace u = CLHEP;

class G4Event;

class MyMuonGammaGenerator : public RMGVGenerator {
public:
  MyMuonGammaGenerator();
  ~MyMuonGammaGenerator();
  
  MyMuonGammaGenerator(MyMuonGammaGenerator const &) = delete;
  MyMuonGammaGenerator &operator=(MyMuonGammaGenerator const &) = delete;
  MyMuonGammaGenerator(MyMuonGammaGenerator &&) = delete;
  MyMuonGammaGenerator &operator=(MyMuonGammaGenerator &&) = delete;

  void GeneratePrimaries(G4Event *event) override;
  void SetParticlePosition(G4ThreeVector) override {};
  void BeginOfRunAction(const G4Run*) override;

private:
  static std::atomic<G4int> fGlobalMuonIndex;  // Shared across threads
  
  void DefineCommands();
  void SetMergedNCDir(G4String pathToDir);
  void LoadNCData();

  std::unique_ptr<G4GenericMessenger> fMessenger = nullptr;

  G4String fInputFilePath;
  
  // NC-indexed data structures
  std::vector<G4int> fMuonIDs;         // MuonID for each NC
  std::vector<G4int> fNCIDs;           // NC IDs
  std::vector<G4double> fNCx;          // NC positions
  std::vector<G4double> fNCy;
  std::vector<G4double> fNCz;
  std::vector<G4double> fNCTimes;      // NC times in ns
  
  // Gamma data
  std::vector<G4int> fGammaMuonIDs;
  std::vector<G4int> fGammaNCIDs;
  std::vector<G4int> fGammaIDs;
  std::vector<G4double> fGammaPx;
  std::vector<G4double> fGammaPy;
  std::vector<G4double> fGammaPz;
  std::vector<G4double> fGammaEnergies;
  std::vector<G4double> fGammaPolX;
  std::vector<G4double> fGammaPolY;
  std::vector<G4double> fGammaPolZ;
  
  // Map: (MuonID, NCID) -> gamma indices (identical to SingleNC)
  std::map<std::pair<G4int, G4int>, std::vector<size_t>> fNCToGammaIndices;
  
  // NEW: Map: MuonID -> NC indices
  std::map<G4int, std::vector<size_t>> fMuonToNCIndices;
  
  // NEW: List of unique muon IDs in order
  std::vector<G4int> fUniqueMuonIDs;
};

#endif
// vim: tabstop=2 shiftwidth=2 expandtab