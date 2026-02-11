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

  static G4String fInputFilePath;
  
  // NC-indexed data structures
  static std::vector<G4int> fMuonIDs;         // MuonID for each NC
  static std::vector<G4int> fNCIDs;           // NC IDs
  static std::vector<G4double> fNCx;          // NC positions
  static std::vector<G4double> fNCy;
  static std::vector<G4double> fNCz;
  static std::vector<G4double> fNCTimes;      // NC times in ns
  
  // Gamma data
  static std::vector<G4int> fGammaMuonIDs;
  static std::vector<G4int> fGammaNCIDs;
  static std::vector<G4int> fGammaIDs;
  static std::vector<G4double> fGammaPx;
  static std::vector<G4double> fGammaPy;
  static std::vector<G4double> fGammaPz;
  static std::vector<G4double> fGammaEnergies;
  static std::vector<G4double> fGammaPolX;
  static std::vector<G4double> fGammaPolY;
  static std::vector<G4double> fGammaPolZ;
  
  // Map: (MuonID, NCID) -> gamma indices (identical to SingleNC)
  static std::map<std::pair<G4int, G4int>, std::vector<size_t>> fNCToGammaIndices;
  
  // Map: MuonID -> NC indices
  static std::map<G4int, std::vector<size_t>> fMuonToNCIndices;
  
  // List of unique muon IDs in order
  static std::vector<G4int> fUniqueMuonIDs;

  static bool fDataLoaded;
};

#endif
// vim: tabstop=2 shiftwidth=2 expandtab