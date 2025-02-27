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

#include "MyPrimaryNeutronUserInfo.hh"
#include "RMGHardware.hh"
#include "RMGLog.hh"
#include "RMGManager.hh"

namespace u = CLHEP;

LowFidelityOutputScheme::LowFidelityOutputScheme() { 
  this->DefineCommands(); 
}


void LowFidelityOutputScheme::ClearBeforeEvent() {
  hitTimes.clear();
  hitEnergieDepositions.clear();
  neutronPhysicalVolume = -1;
  neutronMaterial = -1;
  fNeutronGe77 = -1;
};

// invoked in RMGRunAction::SetupAnalysisManager()
void LowFidelityOutputScheme::AssignOutputNames(G4AnalysisManager* ana_man) {  
  G4cout << "Debug: AssignOutputNames" << G4endl;

  auto rmg_man = RMGManager::Instance();
  auto id = rmg_man->RegisterNtuple(OutputRegisterID,
      ana_man->CreateNtuple("PMTHitsOutput", "Event data"));

  ana_man->CreateNtupleIColumn(id, "evtid");
  // Create column structure to safe data
  ana_man->CreateNtupleIColumn(id, "PMT_uid");
  ana_man->CreateNtupleDColumn(id, "hit_time_in_s");
  ana_man->CreateNtupleDColumn(id, "x_hit_position_in_m");
  ana_man->CreateNtupleDColumn(id, "y_hit_position_in_m");
  ana_man->CreateNtupleDColumn(id, "z_hit_position_in_m");
  ana_man->CreateNtupleDColumn(id, "hit_wavelength_in_nm");
  // ana_man->CreateNtupleDColumn(id, "hit_energy_deposition_in_keV");
  ana_man->CreateNtupleIColumn(id, "physical_volume_id_of_N_creation");
  ana_man->CreateNtupleIColumn(id, "material_of_N_creation");
  ana_man->CreateNtupleIColumn(id, "Ge77_produced_in_muon_event");
}

void LowFidelityOutputScheme::StoreEvent(const G4Event* event) {
  auto hce = event->GetHCofThisEvent();
  if (!hce) return;

  // Zugriff auf Primary Neutron User Info
  auto primaryVertex = event->GetPrimaryVertex(0);
  if (primaryVertex) {
      auto userInfo = dynamic_cast<MyPrimaryNeutronUserInfo*>(primaryVertex->GetUserInformation());
      if (userInfo) {
        physVolumeID = userInfo->GetPhysVolumeID();
        materialID = userInfo->GetMaterialID();
        fGe77 = userInfo->GetPrimaryNeutronfGe77();
      }
  }

  // PMT Hits abrufen
  G4int collectionID = G4SDManager::GetSDMpointer()->GetCollectionID("Optical/Hits"); //FIX die CollectionID heißt sicher nicht PMTHitsCollection 
  if (collectionID < 0) {
    G4cout << "StoreEvent: Could not find hit collection Optical/Hits" << G4endl;
    return nullptr;
  }
  auto hitsCollection = static_cast<RMGOpticalDetectorHitsCollection*>(hce->GetHC(collectionID)); // FIX

  if (hitsCollection) {
      for (size_t i = 0; i < hitsCollection->entries(); i++) {
          auto hit = (*hitsCollection)[i];
          if (!hit) continue;

          hitPMTUID = hit->detector_uid;   // FIX Füge Name das PMTs hinzu. Mit Mapping?
          hitTimes = hit->global_time;
          hitWaveLengths = hit->photon_wavelength;
          // hitEnergie = FIX: Berechne Energie aus Wellenlänge
      }
  }
  else {
    G4cout << "StoreEvent: Could not find PMT hit collection associated with neutron event" << G4endl;
    return nullptr;
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
      // Output: Was Ge77 produced in this event?
      ana_man->FillNtupleIColumn(ntupleid, col_id++, event->GetEventID());
      ana_man->FillNtupleIColumn(ntupleid, col_id++, hitPMTUID[i]);
      ana_man->FillNtupleDColumn(ntupleid, col_id++, hitTimes[i]);
      ana_man->FillNtupleDColumn(ntupleid, col_id++, hitWaveLengths[i]/u::nm);
      // Neutroneninfo:
      ana_man->FillNtupleIColumn(ntupleid, col_id++, neutronPhysicalVolume);
      ana_man->FillNtupleIColumn(ntupleid, col_id++, neutronMaterial);
      ana_man->FillNtupleIColumn(ntupleid, col_id++, fNeutronGe77);
      // Startet neue Reihe in Output
      ana_man->AddNtupleRow(ntupleid);
    }
  }
}

void LowFidelityOutputScheme::DefineCommands() {
  
}

// vim: tabstop=2 shiftwidth=2 expandtab