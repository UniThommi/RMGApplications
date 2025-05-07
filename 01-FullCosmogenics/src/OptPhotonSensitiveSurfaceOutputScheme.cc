#include "OptPhotonSensitiveSurfaceOutputScheme.hh"
#include "MyTrackInfo.hh"
// #include "MyPhotonHitsCollection.hh"

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
#include "G4Gamma.hh"

#include "RMGHardware.hh"
#include "RMGLog.hh"
#include "RMGManager.hh"

namespace u = CLHEP;

OptHitsSensitiveSurfaceOutputScheme::OptHitsSensitiveSurfaceOutputScheme() { 
  this->DefineCommands(); 
}

OptHitsSensitiveSurfaceOutputScheme::~OptHitsSensitiveSurfaceOutputScheme() {};

void OptHitsSensitiveSurfaceOutputScheme::ClearBeforeEvent() {
  // Clear Photons Allocator

};


void OptHitsSensitiveSurfaceOutputScheme::TrackingActionPre(const G4Track* aTrack) { 

}


// invoked in RMGRunAction::SetupAnalysisManager()
void OptHitsSensitiveSurfaceOutputScheme::AssignOutputNames(G4AnalysisManager* ana_man) {  
    G4cout << "Debug: AssignOutputNames" << G4endl;

    auto rmg_man = RMGManager::Instance();
    auto optPhotonsNTuple = rmg_man->RegisterNtuple(optPhotonsRegister,
        ana_man->CreateNtuple("SensitiveSurfaceOutput", "Event data"));

    ana_man->CreateNtupleIColumn(optPhotonsNTuple, "evtid");
    // Create column structure to safe data
    ana_man->CreateNtupleIColumn(optPhotonsNTuple, "nC_neutron_ID");
    ana_man->CreateNtupleDColumn(optPhotonsNTuple, "nC_global_time_in_s");
    ana_man->CreateNtupleDColumn(optPhotonsNTuple, "nC_x_position_in_m");
    ana_man->CreateNtupleDColumn(optPhotonsNTuple, "nC_y_position_in_m");
    ana_man->CreateNtupleDColumn(optPhotonsNTuple, "nC_z_position_in_m");
    ana_man->CreateNtupleIColumn(optPhotonsNTuple, "nC_physical_volume_id_of_N_creation");
    ana_man->CreateNtupleIColumn(optPhotonsNTuple, "nC_material_id_of_N_creation");
    ana_man->CreateNtupleIColumn(optPhotonsNTuple, "nC_gamma_amount");
    ana_man->CreateNtupleIColumn(optPhotonsNTuple, "nC_gamma_total_Energy_in_keV");
    ana_man->CreateNtupleIColumn(optPhotonsNTuple, "nC_Ge77_produced");
    
    ana_man->CreateNtupleDColumn(optPhotonsNTuple, "gamma_x_momentum_direction");
    ana_man->CreateNtupleDColumn(optPhotonsNTuple, "gamma_y_momentum_direction");
    ana_man->CreateNtupleDColumn(optPhotonsNTuple, "gamma_z_momentum_direction");
    ana_man->CreateNtupleDColumn(optPhotonsNTuple, "gamma_kinetic_energy_in_keV");

    ana_man->CreateNtupleIColumn(optPhotonsNTuple, "opt_hit_detector_uid");
    ana_man->CreateNtupleDColumn(optPhotonsNTuple, "opt_hit_energy_in_keV");
    ana_man->CreateNtupleDColumn(optPhotonsNTuple, "opt_hit_global_time_in_s");
    ana_man->CreateNtupleDColumn(optPhotonsNTuple, "opt_hit_x_position_in_m");
    ana_man->CreateNtupleDColumn(optPhotonsNTuple, "opt_hit_y_position_in_m");
    ana_man->CreateNtupleDColumn(optPhotonsNTuple, "opt_hit_z_position_in_m");
    ana_man->CreateNtupleDColumn(optPhotonsNTuple, "opt_hit_x_momentum_direction");
    ana_man->CreateNtupleDColumn(optPhotonsNTuple, "opt_hit_y_momentum_direction");
    ana_man->CreateNtupleDColumn(optPhotonsNTuple, "opt_hit_z_momentum_direction");

    // Speichern der Mappings
    auto physVol = rmg_man->RegisterNtuple(physVolRegister,
        ana_man->CreateNtuple("physVolumes", "physVolumes name mapping"));
    ana_man->CreateNtupleIColumn(physVol, "physVolumesID");
    ana_man->CreateNtupleSColumn(physVol, "physVolumeNames");
    ana_man->FinishNtuple(physVol);

    auto materials = rmg_man->RegisterNtuple(materialRegister,
        ana_man->CreateNtuple("materials", "materials name mapping"));
    ana_man->CreateNtupleIColumn(materials, "materialsID");
    ana_man->CreateNtupleSColumn(materials, "materialNames");
    ana_man->FinishNtuple(materials);
}

void OptHitsSensitiveSurfaceOutputScheme::StoreEvent(const G4Event* event) {
    // auto rmg_man = RMGManager::Instance();
    // if (rmg_man->IsPersistencyEnabled()) { 
    //     RMGLog::OutDev(RMGLog::debug, "Filling persistent data vectors");
    //     const auto ana_man = G4AnalysisManager::Instance();
    //     auto optPhotonsNTuple = rmg_man->GetNtupleID(optPhotonsRegister); 
    //     auto physVolumesNTuple = rmg_man->GetNtupleID(physVolRegister);
    //     auto materialsNTuple = rmg_man->GetNtupleID(materialRegister);

    //     auto hitsCollection = fEventAction->GetPhotonHitsCollection();
    //     if (!hitsCollection) {
    //         G4cout << "ERROR: Keine Hits Collection für das ganze Event!" << G4endl;
    //         return;
    //     }

    //     for (size_t i = 0; i < hitsCollection->GetSize(); ++i) {
    //         PhotonHit* hit = (*hitsCollection)[i];
    //         // Update Mappings
    //         int physVolumeID = -1;
    //         int materialID = -1;
    //         if (hit) {
    //         // Volumenname und Materialname ermitteln
    //             G4String physVolumeName = hit->GetnCPhysVol();
                
    //             if (physVolumeMapping.find(physVolumeName) == physVolumeMapping.end()) {
    //                 const G4int physicalVolumeMappingID = physVolumeMapping.size();
    //                 physVolumeMapping.emplace(physVolumeName, physicalVolumeMappingID);
    //                 //Speichern         
    //                 int vol_col_id = 0;
    //                 ana_man->FillNtupleIColumn(physVolumesNTuple, vol_col_id++, physicalVolumeMappingID);
    //                 ana_man->FillNtupleSColumn(physVolumesNTuple, vol_col_id++, physVolumeName);
    //                 ana_man->AddNtupleRow(physVolumesNTuple);
                    
    //             }
    //             physVolumeID = physVolumeMapping[physVolumeName];
                
            
    //             G4String materialName = hit->GetnCMaterial();
            
    //             if (materialMapping.find(materialName) == materialMapping.end()) {
    //                 const G4int materialMappingID = materialMapping.size();
    //                 materialMapping.emplace(materialName, materialMappingID);
    //                 // Speichern
    //                 int mat_col_id = 0;
    //                 ana_man->FillNtupleIColumn(materialsNTuple, mat_col_id++, materialMappingID);
    //                 ana_man->FillNtupleSColumn(materialsNTuple, mat_col_id++, materialName);
    //                 ana_man->AddNtupleRow(materialsNTuple);

    //             }
    //             materialID = materialMapping[materialName];
            
    //         }
            
    //         // -> Speicher die Infos raus (Position, Zeit, Energie, ...)
    //         int col_id = 0;
    //         // Output: Was Ge77 produced in this event?
    //         ana_man->FillNtupleIColumn(optPhotonsNTuple, col_id++, event->GetEventID());
    //         ana_man->FillNtupleIColumn(optPhotonsNTuple, col_id++, hit->GetnCTrackID());
    //         ana_man->FillNtupleDColumn(optPhotonsNTuple, col_id++, hit->GetnCTime()/u::s);
    //         ana_man->FillNtupleDColumn(optPhotonsNTuple, col_id++, hit->GetnCPos().getX()/u::m);
    //         ana_man->FillNtupleDColumn(optPhotonsNTuple, col_id++, hit->GetnCPos().getY()/u::m);
    //         ana_man->FillNtupleDColumn(optPhotonsNTuple, col_id++, hit->GetnCPos().getZ()/u::m); 
    //         ana_man->FillNtupleIColumn(optPhotonsNTuple, col_id++, physVolumeID);
    //         ana_man->FillNtupleIColumn(optPhotonsNTuple, col_id++, materialID);

    //         ana_man->FillNtupleIColumn(optPhotonsNTuple, col_id++, hit->GetnCGammaAmount());
    //         ana_man->FillNtupleDColumn(optPhotonsNTuple, col_id++, hit->GetnCGammaTotalEnergy()/u::keV);
    //         ana_man->FillNtupleIColumn(optPhotonsNTuple, col_id++, hit->GetnCfGe77());
            
    //         ana_man->FillNtupleDColumn(optPhotonsNTuple, col_id++, hit->GetGammaMomentumDirection().getX());
    //         ana_man->FillNtupleDColumn(optPhotonsNTuple, col_id++, hit->GetGammaMomentumDirection().getY());
    //         ana_man->FillNtupleDColumn(optPhotonsNTuple, col_id++, hit->GetGammaMomentumDirection().getZ()); 
    //         ana_man->FillNtupleDColumn(optPhotonsNTuple, col_id++, hit->GetGammaKineticEnergy()/u::keV);

    //         ana_man->FillNtupleIColumn(optPhotonsNTuple, col_id++, hit->GetDetectorUID());
    //         ana_man->FillNtupleDColumn(optPhotonsNTuple, col_id++, hit->GetOptPhotonEnergy()/u::keV);
    //         ana_man->FillNtupleDColumn(optPhotonsNTuple, col_id++, hit->GetOptPhotonGlobalTime()/u::s);
    //         ana_man->FillNtupleDColumn(optPhotonsNTuple, col_id++, hit->GetOptPhotonPosition().getX()/u::m);
    //         ana_man->FillNtupleDColumn(optPhotonsNTuple, col_id++, hit->GetOptPhotonPosition().getY()/u::m);
    //         ana_man->FillNtupleDColumn(optPhotonsNTuple, col_id++, hit->GetOptPhotonPosition().getZ()/u::m);
    //         ana_man->FillNtupleDColumn(optPhotonsNTuple, col_id++, hit->GetOptPhotonMomentumDirection().getX());
    //         ana_man->FillNtupleDColumn(optPhotonsNTuple, col_id++, hit->GetOptPhotonMomentumDirection().getY());
    //         ana_man->FillNtupleDColumn(optPhotonsNTuple, col_id++, hit->GetOptPhotonMomentumDirection().getZ());

    //         ana_man->AddNtupleRow(optPhotonsNTuple);
    //     }
    // }  
}

void OptHitsSensitiveSurfaceOutputScheme::DefineCommands() {
  
}

// vim: tabstop=2 shiftwidth=2 expandtab