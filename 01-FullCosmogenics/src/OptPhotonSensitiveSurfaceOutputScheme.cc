#include "OptHitsSensitiveSurfaceOutputScheme.hh"
#include "MyTrackInfo.hh"
#include "MyPhotonHit.hh"

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


void OptHitsSensitiveSurfaceOutputScheme::TrackingActionPre(const G4Track* aTrack) { //FIX: Wird nicht gebraucht!

    // Wenn es sich um ein optisches Photon handelt was den Sensitive Surface Detector getroffen hat (registriert in SDManager) speichere es
    // optical photon?
    auto particle = aTrack->GetDefinition();
    if (particle != G4OpticalPhoton::OpticalPhotonDefinition()) return;
    
    auto* trackInfo = dynamic_cast<MyTrackInfo*>(aTrack->GetUserInformation());


    if (aTrack->GetParticleDefinition() == G4Gamma::Definition()) {
    // Initialisieren der Track Info
    auto* trackInfo = dynamic_cast<MyTrackInfo*>(aTrack->GetUserInformation());
    if (trackInfo && trackInfo->GetnCNeutronID() != -1) {
        // Push Data
        // G4cout << "Pushe Gamma Daten" << G4endl;
        nCNeutronID.push_back(trackInfo->GetnCNeutronID());
        gammaPositions.push_back(aTrack->GetVertexPosition()); // Save the locations of Neutrons creation
        gammaMomentumDirections.push_back(aTrack->GetMomentumDirection());
        globalTimes.push_back(aTrack->GetGlobalTime());
        gammaKinEnergies.push_back(aTrack->GetVertexKineticEnergy());
        // In welchem Volumen erzeugt? Nicht als string ausgeben sondern als int 
        
        fGe77.push_back(trackInfo->GetnCfGe77());
        const_cast<G4Track*>(aTrack)->SetTrackStatus(fStopAndKill);
        }
    }
}


// invoked in RMGRunAction::SetupAnalysisManager()
void OptHitsSensitiveSurfaceOutputScheme::AssignOutputNames(G4AnalysisManager* ana_man) {  
    G4cout << "Debug: AssignOutputNames" << G4endl;

    auto rmg_man = RMGManager::Instance();
    auto optPhotonsNTuple = rmg_man->RegisterNtuple(neutronsRegister,
        ana_man->CreateNtuple("NeutronCaptureOutput", "Event data"));

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
    auto rmg_man = RMGManager::Instance();
    if (rmg_man->IsPersistencyEnabled()) { 
        RMGLog::OutDev(RMGLog::debug, "Filling persistent data vectors");
        const auto ana_man = G4AnalysisManager::Instance();
        auto optPhotonsNTuple = rmg_man->GetNtupleID(optPhotonsRegister); 
        auto physVolumesNTuple = rmg_man->GetNtupleID(physVolRegister);
        auto materialsNTuple = rmg_man->GetNtupleID(materialRegister);

        // FIX: Hole hier die Daten aus Photon Allocator
        G4HCofThisEvent* hce = event->GetHCofThisEvent();
        if (!hce) return;

        G4int hcID = G4SDManager::GetSDMpointer()->GetCollectionID("PhotonSD/PhotonHitsCollection");
        auto hitsCollection = static_cast<PhotonHitsCollection*>(hce->GetHC(hcID));

        if (!hitsCollection) {
            G4cout << "ERROR: Keine Hits Collection für das ganze Event!" << G4endl;
            return;
        }

        for (size_t i = 0; i < hitsCollection->GetSize(); ++i) {
            PhotonHit* hit = (*hitsCollection)[i];
            // Update Mappings
            int physVolumeID = -1;
            int materialID = -1;
            if (hit) {
            // Volumenname und Materialname ermitteln
                G4string physVolumeName = hit->nCPhysVol;
                
                if (physVolumeMapping.find(physVolumeName) == physVolumeMapping.end()) {
                    const G4int physicalVolumeMappingID = physVolumeMapping.size();
                    physVolumeMapping.emplace(physVolumeName, physicalVolumeMappingID);
                    //Speichern         
                    int vol_col_id = 0;
                    ana_man->FillNtupleIColumn(physVolumesNTuple, vol_col_id++, physicalVolumeMappingID);
                    ana_man->FillNtupleSColumn(physVolumesNTuple, vol_col_id++, physVolumeName);
                    ana_man->AddNtupleRow(physVolumesNTuple);
                    
                }
                physVolumeID = physVolumeMapping[physVolumeName];
                
            
                G4string materialName = hit->nCMaterial;
            
                if (materialMapping.find(materialName) == materialMapping.end()) {
                    const G4int materialMappingID = materialMapping.size();
                    materialMapping.emplace(materialName, materialMappingID);
                    // Speichern
                    int mat_col_id = 0;
                    ana_man->FillNtupleIColumn(materialsNTuple, mat_col_id++, materialMappingID);
                    ana_man->FillNtupleSColumn(materialsNTuple, mat_col_id++, materialName);
                    ana_man->AddNtupleRow(materialsNTuple);

                }
                materialID = materialMapping[materialName];
            
            }
            
            // -> Speicher die Infos raus (Position, Zeit, Energie, ...)
            int col_id = 0;
            // Output: Was Ge77 produced in this event?
            ana_man->FillNtupleIColumn(optPhotonsNTuple, col_id++, event->GetEventID());
            ana_man->FillNtupleIColumn(optPhotonsNTuple, col_id++, hit->nCTrackID);
            ana_man->FillNtupleDColumn(optPhotonsNTuple, col_id++, hit->nCTime/u::s);
            ana_man->FillNtupleDColumn(optPhotonsNTuple, col_id++, hit->nCPos.getX()/u::m);
            ana_man->FillNtupleDColumn(optPhotonsNTuple, col_id++, hit->nCPos.getY()/u::m);
            ana_man->FillNtupleDColumn(optPhotonsNTuple, col_id++, hit->nCPos.getZ()/u::m); 
            ana_man->FillNtupleIColumn(optPhotonsNTuple, col_id++, physVolumeID);
            ana_man->FillNtupleIColumn(optPhotonsNTuple, col_id++, materialID);

            ana_man->FillNtupleIColumn(optPhotonsNTuple, col_id++, hit->nCGammaAmount);
            ana_man->FillNtupleIColumn(optPhotonsNTuple, col_id++, hit->nCGammaTotalEnergy/u::keV);
            ana_man->FillNtupleIColumn(optPhotonsNTuple, col_id++, hit->nCfGe77);
            
            ana_man->FillNtupleDColumn(optPhotonsNTuple, col_id++, hit->gammaMomentumDirections.getX());
            ana_man->FillNtupleDColumn(optPhotonsNTuple, col_id++, hit->gammaMomentumDirections.getY());
            ana_man->FillNtupleDColumn(optPhotonsNTuple, col_id++, hit->gammaMomentumDirections.getZ()); 
            ana_man->FillNtupleDColumn(optPhotonsNTuple, col_id++, hit->gammaKineticEnergy/u::keV);

            ana_man->FillNtupleDColumn(optPhotonsNTuple, col_id++, hit->detectorUID);
            ana_man->FillNtupleDColumn(optPhotonsNTuple, col_id++, hit->optPhotonEnergy/u::keV);
            ana_man->FillNtupleDColumn(optPhotonsNTuple, col_id++, hit->optPhotonGlobalTime/u::s);
            ana_man->FillNtupleDColumn(optPhotonsNTuple, col_id++, hit->optPhotonPosition.getX()/u::m);
            ana_man->FillNtupleDColumn(optPhotonsNTuple, col_id++, hit->optPhotonPosition.getY()/u::m);
            ana_man->FillNtupleDColumn(optPhotonsNTuple, col_id++, hit->optPhotonPosition.getZ()/u::m);
            ana_man->FillNtupleDColumn(optPhotonsNTuple, col_id++, hit->optPhotonMomentumDirection.getX());
            ana_man->FillNtupleDColumn(optPhotonsNTuple, col_id++, hit->optPhotonMomentumDirection.getY());
            ana_man->FillNtupleDColumn(optPhotonsNTuple, col_id++, hit->optPhotonMomentumDirection.getZ());

            ana_man->AddNtupleRow(optPhotonsNTuple);
        }
    }  
}

void OptHitsSensitiveSurfaceOutputScheme::DefineCommands() {
  
}

// vim: tabstop=2 shiftwidth=2 expandtab