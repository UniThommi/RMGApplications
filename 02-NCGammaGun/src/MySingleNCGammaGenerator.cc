#include "MySingleNCGammaGenerator.hh"
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
#include <random>

std::atomic<G4int> MySingleNCGammaGenerator::fGlobalNCIndex{0};
G4String MySingleNCGammaGenerator::fInputFilePath = "";

std::vector<G4int>   MySingleNCGammaGenerator::fMuonIDs       = {};
std::vector<G4int>   MySingleNCGammaGenerator::fNCIDs         = {};
std::vector<G4double> MySingleNCGammaGenerator::fNCx          = {};
std::vector<G4double> MySingleNCGammaGenerator::fNCy          = {};
std::vector<G4double> MySingleNCGammaGenerator::fNCz          = {};
std::vector<G4double> MySingleNCGammaGenerator::fNCTimes      = {};

std::vector<G4int>   MySingleNCGammaGenerator::fGammaMuonIDs  = {};
std::vector<G4int>   MySingleNCGammaGenerator::fGammaNCIDs    = {};
std::vector<G4int>   MySingleNCGammaGenerator::fGammaIDs      = {};
std::vector<G4double> MySingleNCGammaGenerator::fGammaPx      = {};
std::vector<G4double> MySingleNCGammaGenerator::fGammaPy      = {};
std::vector<G4double> MySingleNCGammaGenerator::fGammaPz      = {};
std::vector<G4double> MySingleNCGammaGenerator::fGammaEnergies= {};
std::vector<G4double> MySingleNCGammaGenerator::fGammaPolX    = {};
std::vector<G4double> MySingleNCGammaGenerator::fGammaPolY    = {};
std::vector<G4double> MySingleNCGammaGenerator::fGammaPolZ    = {};

std::map<std::pair<G4int,G4int>, std::vector<size_t>>
    MySingleNCGammaGenerator::fNCToGammaIndices                = {};

bool MySingleNCGammaGenerator::fDataLoaded                     = false;

namespace u = CLHEP;

MySingleNCGammaGenerator::MySingleNCGammaGenerator() : RMGVGenerator("SingleNCGamma") {
  this->DefineCommands();
}

MySingleNCGammaGenerator::~MySingleNCGammaGenerator() {}

void MySingleNCGammaGenerator::SetMergedNCDir(G4String pathToDir) {
  fInputFilePath = pathToDir;
}

void MySingleNCGammaGenerator::BeginOfRunAction(const G4Run*) {  
  static std::once_flag loadFlag;
  std::call_once(loadFlag, [this]() {
    RMGLog::Out(RMGLog::summary, "call_once: entering LoadNCData");
    LoadNCData();
    RMGLog::Out(RMGLog::summary, "call_once: LoadNCData done, fNCIDs.size()=", fNCIDs.size());
  });
}

void MySingleNCGammaGenerator::LoadNCData() {
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
    
    // Parse: muon_id,nc_id,nc_x,nc_y,nc_z,nc_time,run_id,orig_muon_id
    std::getline(ss, token, ','); fMuonIDs.push_back(std::stoi(token));
    std::getline(ss, token, ','); fNCIDs.push_back(std::stoi(token));
    std::getline(ss, token, ','); fNCx.push_back(std::stod(token));
    std::getline(ss, token, ','); fNCy.push_back(std::stod(token));
    std::getline(ss, token, ','); fNCz.push_back(std::stod(token));
    std::getline(ss, token, ','); fNCTimes.push_back(std::stod(token));
    std::getline(ss, token, ','); // skip run_id
    std::getline(ss, token, ','); // skip orig_muon_id
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
  
  RMGLog::Out(RMGLog::debug, "Built gamma index map for ", fNCToGammaIndices.size(), " unique NCs");
}

void MySingleNCGammaGenerator::GeneratePrimaries(G4Event* event) {
  if (fNCIDs.empty()) {
    RMGLog::Out(RMGLog::fatal, "GeneratePrimaries called but fNCIDs is empty!");
    return;
  }
  // Thread-safe fetch-and-increment
  G4int currentIndex = fGlobalNCIndex.fetch_add(1);
  if (currentIndex >= static_cast<G4int>(fNCIDs.size())) {
    RMGLog::Out(RMGLog::error, "Reached end of NC data. Requested ", 
                currentIndex + 1, " but only have ", fNCIDs.size(), " NCs");
    return;
  }
  
  // Get NC data for current event
  G4int ncID = fNCIDs[currentIndex];
  G4int muonID = fMuonIDs[currentIndex];
  G4ThreeVector ncPos(fNCx[currentIndex] * u::m, 
                      fNCy[currentIndex] * u::m, 
                      fNCz[currentIndex] * u::m);
  G4double ncTime = fNCTimes[currentIndex] * u::ns;
  
  // Get gammas for this NC using (muonID, ncID) pair
  auto key = std::make_pair(muonID, ncID);
  auto it = fNCToGammaIndices.find(key);
  if (it == fNCToGammaIndices.end()) {
    RMGLog::Out(RMGLog::warning, "No gammas found for NC (MuonID=", muonID, ", NCID=", ncID, ")");
    return;
  }
  
  const auto& gammaIndices = it->second;
  
  RMGLog::OutDev(RMGLog::debug, "Event ", event->GetEventID(), 
                 ": Generating ", gammaIndices.size(), " gammas for NC ", ncID);
  
  // Create separate vertex for each gamma (like old code)
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

void MySingleNCGammaGenerator::DefineCommands() {
  fMessenger = std::make_unique<G4GenericMessenger>(this, "/My/Generator/SingleNCGamma/",
                                                      "Commands for single NC gamma generator");
  
  fMessenger->DeclareMethod("SetMergedDir", &MySingleNCGammaGenerator::SetMergedNCDir)
      .SetGuidance("Set path to directory containing merged_ncs.csv and merged_gammas.csv")
      .SetParameterName("dirpath", false)
      .SetToBeBroadcasted(true)
      .SetStates(G4State_PreInit, G4State_Idle);
}

// vim: tabstop=2 shiftwidth=2 expandtab