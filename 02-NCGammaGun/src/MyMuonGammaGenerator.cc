#include "MyMuonGammaGenerator.hh"
#include "MyPrimaryGammaUserInfo.hh"

#include "G4Event.hh"
#include "G4Gamma.hh"
#include "G4ParticleDefinition.hh"
#include "G4PrimaryParticle.hh"
#include "G4PrimaryVertex.hh"
#include "G4ThreeVector.hh"
#include "RMGLog.hh"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <set>

std::atomic<G4int> MyMuonGammaGenerator::fGlobalMuonIndex{-1};

namespace u = CLHEP;

MyMuonGammaGenerator::MyMuonGammaGenerator() : RMGVGenerator("MuonGamma") {
  this->DefineCommands();
}

MyMuonGammaGenerator::~MyMuonGammaGenerator() {}

void MyMuonGammaGenerator::SetMergedNCDir(G4String pathToDir) {
  fInputFilePath = pathToDir;
  RMGLog::Out(RMGLog::summary, "MyMuonGammaGenerator: Set input directory to ", pathToDir);
}

void MyMuonGammaGenerator::BeginOfRunAction(const G4Run*) {
  if (fInputFilePath.empty()) {
    RMGLog::Out(RMGLog::fatal, "MyMuonGammaGenerator: No input directory specified!");
    throw std::runtime_error("No merged NC directory specified");
  }
  
  LoadNCData();
  
  RMGLog::Out(RMGLog::summary, "MyMuonGammaGenerator: Loaded ", fUniqueMuonIDs.size(), 
              " unique muons with ", fNCIDs.size(), " NCs and ", fGammaIDs.size(), " total gammas");
}

void MyMuonGammaGenerator::LoadNCData() {
  // Construct file paths
  G4String ncFile = fInputFilePath + "/merged_ncs.csv";
  G4String gammaFile = fInputFilePath + "/merged_gammas.csv";
  
  // Load NC data
  RMGLog::Out(RMGLog::debug, "Loading NC data from ", ncFile);
  std::ifstream ncStream(ncFile);
  if (!ncStream.is_open()) {
    RMGLog::Out(RMGLog::fatal, "Cannot open NC file: ", ncFile);
    throw std::runtime_error("Cannot open NC CSV file");
  }
  
  std::string line;
  std::getline(ncStream, line); // Skip header
  
  while (std::getline(ncStream, line)) {
    std::stringstream ss(line);
    std::string token;
    
    // Parse: muon_id,nc_id,nc_x,nc_y,nc_z,nc_time
    std::getline(ss, token, ','); fMuonIDs.push_back(std::stoi(token));
    std::getline(ss, token, ','); fNCIDs.push_back(std::stoi(token));
    std::getline(ss, token, ','); fNCx.push_back(std::stod(token));
    std::getline(ss, token, ','); fNCy.push_back(std::stod(token));
    std::getline(ss, token, ','); fNCz.push_back(std::stod(token));
    std::getline(ss, token, ','); fNCTimes.push_back(std::stod(token));
  }
  ncStream.close();
  
  // Load Gamma data
  RMGLog::Out(RMGLog::debug, "Loading Gamma data from ", gammaFile);
  std::ifstream gammaStream(gammaFile);
  if (!gammaStream.is_open()) {
    RMGLog::Out(RMGLog::fatal, "Cannot open Gamma file: ", gammaFile);
    throw std::runtime_error("Cannot open Gamma CSV file");
  }
  
  std::getline(gammaStream, line); // Skip header
  
  while (std::getline(gammaStream, line)) {
    std::stringstream ss(line);
    std::string token;
    
    // Parse: muon_id,nc_id,gamma_id,gamma_px,gamma_py,gamma_pz,gamma_E,gamma_pol_x,gamma_pol_y,gamma_pol_z
    std::getline(ss, token, ','); fGammaMuonIDs.push_back(std::stoi(token));
    std::getline(ss, token, ','); fGammaNCIDs.push_back(std::stoi(token));
    std::getline(ss, token, ','); fGammaIDs.push_back(std::stoi(token));
    std::getline(ss, token, ','); fGammaPx.push_back(std::stod(token));
    std::getline(ss, token, ','); fGammaPy.push_back(std::stod(token));
    std::getline(ss, token, ','); fGammaPz.push_back(std::stod(token));
    std::getline(ss, token, ','); fGammaEnergies.push_back(std::stod(token));
    std::getline(ss, token, ','); fGammaPolX.push_back(std::stod(token));
    std::getline(ss, token, ','); fGammaPolY.push_back(std::stod(token));
    std::getline(ss, token, ','); fGammaPolZ.push_back(std::stod(token));
  }
  gammaStream.close();
  
  // Build map: (MuonID, NCID) -> gamma indices
  for (size_t i = 0; i < fGammaNCIDs.size(); ++i) {
    G4int muon_id = fGammaMuonIDs[i];
    G4int nc_id = fGammaNCIDs[i];
    auto key = std::make_pair(muon_id, nc_id);
    fNCToGammaIndices[key].push_back(i);
  }
  
  // Build map: MuonID -> NC indices
  for (size_t i = 0; i < fMuonIDs.size(); ++i) {
    G4int muon_id = fMuonIDs[i];
    fMuonToNCIndices[muon_id].push_back(i);
  }
  
  // Extract unique muon IDs in sorted order
  std::set<G4int> uniqueMuons(fMuonIDs.begin(), fMuonIDs.end());
  fUniqueMuonIDs.assign(uniqueMuons.begin(), uniqueMuons.end());
  
  RMGLog::Out(RMGLog::debug, "Built gamma index map for ", fNCToGammaIndices.size(), " unique NCs");
  RMGLog::Out(RMGLog::debug, "Built muon index map for ", fMuonToNCIndices.size(), " unique muons");
}

void MyMuonGammaGenerator::GeneratePrimaries(G4Event* event) {
  // Thread-safe fetch-and-increment
  G4int currentMuonIndex = ++fGlobalMuonIndex;
  
  if (currentMuonIndex >= static_cast<G4int>(fUniqueMuonIDs.size())) {
    RMGLog::Out(RMGLog::error, "Reached end of muon data. Requested ", 
                currentMuonIndex + 1, " but only have ", fUniqueMuonIDs.size(), " muons");
    return;
  }
  
  // Get muon ID for this event
  G4int muonID = fUniqueMuonIDs[currentMuonIndex];
  
  // Get all NCs for this muon
  auto muonIt = fMuonToNCIndices.find(muonID);
  if (muonIt == fMuonToNCIndices.end()) {
    RMGLog::Out(RMGLog::error, "No NCs found for Muon ID ", muonID);
    return;
  }
  
  const auto& ncIndices = muonIt->second;
  
  RMGLog::OutDev(RMGLog::debug, "Event ", event->GetEventID(), 
                 ": Processing Muon ", muonID, " with ", ncIndices.size(), " NCs");
  
  // Iterate over all NCs for this muon
  for (size_t ncIdx : ncIndices) {
    G4int ncID = fNCIDs[ncIdx];
    G4ThreeVector ncPos(fNCx[ncIdx] * u::m, 
                        fNCy[ncIdx] * u::m, 
                        fNCz[ncIdx] * u::m);
    G4double ncTime = fNCTimes[ncIdx] * u::ns;
    
    // Get gammas for this NC
    auto key = std::make_pair(muonID, ncID);
    auto gammaIt = fNCToGammaIndices.find(key);
    if (gammaIt == fNCToGammaIndices.end()) {
      RMGLog::Out(RMGLog::warning, "No gammas found for NC (MuonID=", muonID, ", NCID=", ncID, ")");
      continue;
    }
    
    const auto& gammaIndices = gammaIt->second;
    
    // Create one vertex per gamma (identical to SingleNC)
    for (size_t idx : gammaIndices) {
      G4int gammaID = fGammaIDs[idx];
      
      // Momentum und Energie
      G4ThreeVector momDir(fGammaPx[idx], fGammaPy[idx], fGammaPz[idx]);
      momDir = momDir.unit();
      G4double energy = fGammaEnergies[idx] * u::keV;
      
      // Polarization
      G4ThreeVector pol(fGammaPolX[idx], fGammaPolY[idx], fGammaPolZ[idx]);
      if (pol.mag() > 0) pol = pol.unit();
      
      // Vertex erzeugen
      auto* vertex = new G4PrimaryVertex(ncPos, ncTime);
      auto* vertexUserInfo = new MyPrimaryGammaUserInfo(muonID, ncID, gammaID);
      vertex->SetUserInformation(vertexUserInfo);
      
      // Particle manuell erzeugen
      auto* particle = new G4PrimaryParticle(G4Gamma::Definition());
      particle->SetMomentumDirection(momDir);
      particle->SetKineticEnergy(energy);
      particle->SetPolarization(pol);
      
      // Particle zum Vertex hinzufügen
      vertex->SetPrimary(particle);
      
      // Vertex zum Event hinzufügen
      event->AddPrimaryVertex(vertex);
    }
  }
}

void MyMuonGammaGenerator::DefineCommands() {
  fMessenger = std::make_unique<G4GenericMessenger>(this, "/My/Generator/MuonGamma/",
                                                      "Commands for muon gamma generator");
  
  fMessenger->DeclareMethod("SetMergedDir", &MyMuonGammaGenerator::SetMergedNCDir)
      .SetGuidance("Set path to directory containing merged_ncs.csv and merged_gammas.csv")
      .SetParameterName("dirpath", false)
      .SetToBeBroadcasted(true)
      .SetStates(G4State_PreInit, G4State_Idle);
}

// vim: tabstop=2 shiftwidth=2 expandtab