#include "LowFidelityOutputScheme.hh"

#include <set>
#include <algorithm>
#include <iostream>
#include <string>


#include "G4AnalysisManager.hh"
#include "G4Event.hh"
#include "G4EventManager.hh"
#include "G4HCtable.hh"
#include "G4OpticalPhoton.hh"
#include "G4SDManager.hh"

#include "MyPrimaryGammaUserInfo.hh"
#include "RMGHardware.hh"
#include "RMGLog.hh"
#include "RMGManager.hh"

namespace u = CLHEP;

LowFidelityOutputScheme::LowFidelityOutputScheme() { 
  this->DefineCommands(); 
}


void LowFidelityOutputScheme::ClearBeforeEvent() {
  // Neutron Capture Info
  muonID = -1;
  nCGlobalTime = 0;
  nCxPosition = -1;
  nCyPosition = -1;
  nCzPosition = -1;
  nCPhysVolumeID = -1;
  nCMaterialID = -1;
  nCfGe77 = -1;
  nCGammaAmount = -1;
  nCGammaTotalEnergy = 0;
  // PMT Info
  hitPMTUIDs.clear();
  hitTimes.clear();
  // hitxPositions.clear();
  // hityPositions.clear();
  // hitzPositions.clear();
  hitWaveLengths.clear();
};

// invoked in RMGRunAction::SetupAnalysisManager()
void LowFidelityOutputScheme::AssignOutputNames(G4AnalysisManager* ana_man) {  
  G4cout << "Debug: AssignOutputNames" << G4endl;

  auto rmg_man = RMGManager::Instance();
  auto id = rmg_man->RegisterNtuple(OutputRegisterID,
      ana_man->CreateNtuple("PMTHitsOutput", "Event data"));

  ana_man->CreateNtupleIColumn(id, "evtid");
  // Create column structure to safe data
  // Neutron Capture Info
  ana_man->CreateNtupleIColumn(id, "nC_muon_id");
  ana_man->CreateNtupleDColumn(id, "nC_global_time");
  ana_man->CreateNtupleDColumn(id, "nC_x_position_in_m");
  ana_man->CreateNtupleDColumn(id, "nC_y_position_in_m");
  ana_man->CreateNtupleDColumn(id, "nC_z_position_in_m");
  ana_man->CreateNtupleIColumn(id, "nC_physical_volume_id_of_N_creation");
  ana_man->CreateNtupleIColumn(id, "nC_material_id_of_N_creation");
  ana_man->CreateNtupleIColumn(id, "nC_Ge77_produced");
  ana_man->CreateNtupleIColumn(id, "nC_gamma_amount");
  ana_man->CreateNtupleDColumn(id, "nC_gamma_total_energy");

  // PMT Info
  ana_man->CreateNtupleIColumn(id, "PMT_uid");
  ana_man->CreateNtupleDColumn(id, "hit_time_in_s");
  // ana_man->CreateNtupleDColumn(id, "x_hit_position_in_m");
  // ana_man->CreateNtupleDColumn(id, "y_hit_position_in_m");
  // ana_man->CreateNtupleDColumn(id, "z_hit_position_in_m");
  ana_man->CreateNtupleDColumn(id, "hit_wavelength_in_nm");
  // ana_man->CreateNtupleDColumn(id, "hit_energy_deposition_in_keV");
  // Wurde der Capture mit den PMTs registriert? Muss man das im Post-Processing machen?
  //ana_man->CreateNtupleIColumn(id, "nC_detected");
}

void LowFidelityOutputScheme::StoreEvent(const G4Event* event) {
  auto hce = event->GetHCofThisEvent();
  if (!hce) return;

  // Zugriff auf Primary Gamma User Info
  auto primaryVertex = event->GetPrimaryVertex(0);
  if (primaryVertex) {
      auto userInfo = dynamic_cast<MyPrimaryGammaUserInfo*>(primaryVertex->GetUserInformation());
      if (userInfo) {
        muonID = userInfo->GetMuonID();
        nCGlobalTime = primaryVertex->GetT0();
        nCxPosition = primaryVertex->GetX0();
        nCyPosition = primaryVertex->GetY0();
        nCzPosition = primaryVertex->GetZ0();
        nCPhysVolumeID = userInfo->GetnCPhysVolumeID();
        nCMaterialID = userInfo->GetnCMaterialID();
        nCfGe77 = userInfo->GetnCfGe77();
        nCGammaAmount = userInfo->GetnCGammaAmount();
        nCGammaTotalEnergy = userInfo->GetnCGammaTotalEnergy();  

      }
      else {
        G4cout << "Error: Keine UserInfo mit nC Daten vorhanden" << G4endl;
        return;
      }
  }

  // PMT Hits abrufen
  auto hit_coll = GetOptHitColl(event);
  // Optical hit collection can be empty!
  if (!hit_coll) {
    RMGLog::Out(RMGLog::error, "No optical hit collection!");
    return;
  }
  if (hit_coll->entries() <= 0) {
    RMGLog::OutDev(RMGLog::debug, "Optical hit collection is empty");
    return;
  } else {
    RMGLog::OutDev(RMGLog::debug, "Optical hit collection contains ", hit_coll->entries(), " hits");
  }

  if (hit_coll) {
    for (auto hit : *(hit_coll->GetVector())) {
      if (!hit) continue;
      hitPMTUIDs.push_back(hit->detector_uid);   // FIX Füge Name das PMTs hinzu. Mit Mapping?
      hitTimes.push_back(hit->global_time);
      hitWaveLengths.push_back(hit->photon_wavelength);
      // hitEnergie = FIX: Berechne Energie aus Wellenlänge
    }
  }
  else {
    G4cout << "StoreEvent: Could not find PMT hit collection associated with neutron event" << G4endl;
    return;
  }

  auto rmg_man = RMGManager::Instance();
  if (rmg_man->IsPersistencyEnabled()) { 
    RMGLog::OutDev(RMGLog::debug, "Filling persistent data vectors");
    const auto ana_man = G4AnalysisManager::Instance();
    auto ntupleid = rmg_man->GetNtupleID(OutputRegisterID);
    if (ntupleid < 0) {
        G4cerr << "❌ ERROR: Invalid Ntuple ID! Data will not be saved." << G4endl;
        // return;
    }   

    for (size_t i = 0; i < hitTimes.size(); ++i) {
      int col_id = 0;
      ana_man->FillNtupleIColumn(ntupleid, col_id++, event->GetEventID()); // Gleichzeitig Neutron ID
      // Neutron Capture Info:
      ana_man->FillNtupleIColumn(ntupleid, col_id++, muonID);
      ana_man->FillNtupleDColumn(ntupleid, col_id++, nCGlobalTime/u::s);
      ana_man->FillNtupleDColumn(ntupleid, col_id++, nCxPosition);
      ana_man->FillNtupleDColumn(ntupleid, col_id++, nCyPosition);
      ana_man->FillNtupleDColumn(ntupleid, col_id++, nCzPosition);
      ana_man->FillNtupleIColumn(ntupleid, col_id++, nCPhysVolumeID);
      ana_man->FillNtupleIColumn(ntupleid, col_id++, nCMaterialID);
      ana_man->FillNtupleIColumn(ntupleid, col_id++, nCfGe77);
      ana_man->FillNtupleIColumn(ntupleid, col_id++, nCGammaAmount);
      ana_man->FillNtupleDColumn(ntupleid, col_id++, nCGammaTotalEnergy);
      
      // PMT Info
      ana_man->FillNtupleIColumn(ntupleid, col_id++, hitPMTUIDs[i]);
      ana_man->FillNtupleDColumn(ntupleid, col_id++, hitTimes[i]/u::s);
      ana_man->FillNtupleDColumn(ntupleid, col_id++, hitWaveLengths[i]); // in nm

      // Startet neue Reihe in Output
      ana_man->AddNtupleRow(ntupleid);
    }
  }
}

// Could summarize these functions into one, but this is more readable i think
RMGOpticalDetectorHitsCollection* LowFidelityOutputScheme::GetOptHitColl(const G4Event* event) {  // Gets hit collection from sensitive Ge detectors with optical information.
auto sd_man = G4SDManager::GetSDMpointer();

auto hit_coll_id = sd_man->GetCollectionID("Optical/Hits");
if (hit_coll_id < 0) {
  G4cout << "Could not find hit collection Optical/Hits" << G4endl;
  return nullptr;
}

auto hit_coll =
    dynamic_cast<RMGOpticalDetectorHitsCollection*>(event->GetHCofThisEvent()->GetHC(hit_coll_id));

if (!hit_coll) {
  G4cout << "Could not find hit collection associated with event" << G4endl;
  return nullptr;
}

return hit_coll;
}

void LowFidelityOutputScheme::DefineCommands() {
  
}

// vim: tabstop=2 shiftwidth=2 expandtab