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

// Primary generator for --generator muon: every event re-shoots the gammas of ALL
// neutron captures (NCs) of ONE muon of the first simulation, so the NCs of a muon keep
// their time structure within one event. Input as for MySingleNCGammaGenerator
// (merged_ncs.csv and merged_gammas.csv); one vertex per gamma at the capture position
// and time, labelled with MyPrimaryGammaUserInfo.
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
  // Index into fUniqueMuonIDs of the last muon taken: the worker threads together shoot
  // every muon exactly once.
  static std::atomic<G4int> fGlobalMuonIndex;  // Shared across threads

  void DefineCommands();
  void SetMergedNCDir(G4String pathToDir);
  void LoadNCData();

  std::unique_ptr<G4GenericMessenger> fMessenger = nullptr;

  static G4String fInputFilePath;

  // The input data are static, i.e. loaded once and shared by all worker threads
  // instead of one copy per thread (avoids running out of memory).
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
  
  // List of unique muon IDs in order (ascending)
  static std::vector<G4int> fUniqueMuonIDs;

  // BEGIN UNUSED CODE — fDataLoaded is never set or read (loading uses std::call_once)
  static bool fDataLoaded;
  // END UNUSED CODE
};

#endif
// vim: tabstop=2 shiftwidth=2 expandtab