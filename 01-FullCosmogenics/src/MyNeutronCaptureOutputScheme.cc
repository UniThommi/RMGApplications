#include "MyNeutronCaptureOutputScheme.hh"

#include <set>
#include <algorithm>
#include <iostream>
#include <string>
#include <utility>

#include "G4AnalysisManager.hh"
#include "G4Event.hh"
#include "G4EventManager.hh"
#include "G4HCtable.hh"
#include "G4OpticalPhoton.hh"
#include "G4SDManager.hh"
#include "G4Gamma.hh"

#include "MyTrackInfo.hh"
#include "RMGHardware.hh"
#include "RMGLog.hh"
#include "RMGManager.hh"

namespace u = CLHEP;

MyNeutronCaptureOutputScheme::MyNeutronCaptureOutputScheme() { 
  this->DefineCommands(); 
}

MyNeutronCaptureOutputScheme::~MyNeutronCaptureOutputScheme() {};

void MyNeutronCaptureOutputScheme::ClearBeforeEvent() {
  // Neutron Capture Info
  nCTrackIDs.clear();
  nCPositions.clear();
  nCPhysVolumes.clear();
  nCMaterials.clear();
  nCGlobTimes.clear();
  nCGammaTotalEnergies.clear();
  nCGammaAmounts.clear()
  nCfGe77s.clear();
};

void MyNeutronCaptureOutputScheme::TrackingActionPre(const G4Track* aTrack) {
    if (aTrack->GetParticleDefinition() == G4Gamma::Definition()) {
        G4int muonID = event->GetEventID();
        const auto* userInfo = track->GetUserInformation();
        const auto* trackInfo = dynamic_cast<const MyTrackInfo*>(userInfo);

        if (trackInfo && trackInfo->GetnCTrackID() != -1) {
            G4int nCTrackID = trackInfo->GetnCTrackID();
            std::pair<G4int, G4int> candidate = {muonID, nCTrackID};
            if (muonTrackPairs.find(candidate) == muonTrackPairs.end()) {
            // Paar ist noch nicht drin 👉 hinzufügen
            muonTrackPairs.insert(candidate);
            G4cout << "Added pair: (" << muonID << ", " << nCTrackID << ")" << G4endl;

            nCTrackID.push_back(nCTrackID);
            nCPositions.push_back(trackInfo->GetnCPos());
            nCGlobTime.push_back(trackInfo->GetnCTime());
            nCPhysVolId.push_back(trackInfo->GetnCPhysVol());
            nCMaterialId.push_back(trackInfo->GetnCMaterial());
            nCGammaAmount.push_back(trackInfo->GetnCGammaAmount());
            nCGammaTotalEnergy.push_back(trackInfo->GetnCGammaTotalEnergy());
            nCfGe77.push_back(trackInfo->GetnCfGe77());
            } else {
                // Schon vorhanden 👉 nichts tun oder debuggen
                G4cout << "Pair already exists: (" << muonID << ", " << nCTrackID << ")" << G4endl;
            }
        }
    }
}

// invoked in RMGRunAction::SetupAnalysisManager()
void MyNeutronCaptureOutputScheme::AssignOutputNames(G4AnalysisManager* ana_man) {  
    G4cout << "Debug: AssignOutputNames" << G4endl;

    auto rmg_man = RMGManager::Instance();
    auto neutronsNTuple = rmg_man->RegisterNtuple(neutronsRegister,
        ana_man->CreateNtuple("MyNeutronCaptureOutput", "Event data"));

    ana_man->CreateNtupleIColumn(neutronsNTuple, "evtid");
    // Create column structure to safe data
    ana_man->CreateNtupleIColumn(id, "nC_track_id");
    ana_man->CreateNtupleDColumn(id, "nC_x_position_in_m");
    ana_man->CreateNtupleDColumn(id, "nC_y_position_in_m");
    ana_man->CreateNtupleDColumn(id, "nC_z_position_in_m");
    ana_man->CreateNtupleIColumn(id, "nC_phys_vol_id");
    ana_man->CreateNtupleIColumn(id, "nC_material_id");
    ana_man->CreateNtupleDColumn(id, "nC_time_in_ns");
    ana_man->CreateNtupleDColumn(id, "nC_gamma_total_energy_in_keV");
    ana_man->CreateNtupleIColumn(id, "nC_flag_Ge77");
    ana_man->CreateNtupleIColumn(id, "nC_gamma_amount");

    // Speichern der Mappings
    auto physVol = rmg_man->RegisterNtuple(physVolRegister,
        ana_man->CreateNtuple("physVolumes", "physVolumes_name_mapping"));
    ana_man->CreateNtupleIColumn(physVol, "physVolumesID");
    ana_man->CreateNtupleSColumn(physVol, "physVolumeNames");
    ana_man->FinishNtuple(physVol);

    auto materials = rmg_man->RegisterNtuple(materialRegister,
        ana_man->CreateNtuple("materials", "materials_name_mapping"));
    ana_man->CreateNtupleIColumn(materials, "materialsID");
    ana_man->CreateNtupleSColumn(materials, "materialNames");
    ana_man->FinishNtuple(materials);
}

void MyNeutronCaptureOutputScheme::StoreEvent(const G4Event* event) {
    auto rmg_man = RMGManager::Instance();
    if (rmg_man->IsPersistencyEnabled()) { 
        RMGLog::OutDev(RMGLog::debug, "Filling persistent data vectors");
        const auto ana_man = G4AnalysisManager::Instance();
        auto ntupleid = rmg_man->GetNtupleID(OutputRegisterID);
        if (ntupleid < 0) {
            G4cerr << "❌ ERROR: Invalid Ntuple ID! Data will not be saved." << G4endl;
            // return;
        }   

        for (size_t i = 0; i < nCTrackIDs.size(); ++i) {
            G4String nCPhysVol = nCPhysVolIDs[i];
            G4String nCMaterial = nCMaterialIDs[i];

            if (physVolumeMapping.find(nCPhysVol) == physVolumeMapping.end()) {
                const G4int physicalVolumeMappingID = physVolumeMapping.size();
                physVolumeMapping.emplace(nCPhysVol, physicalVolumeMappingID);
                //Speichern   
                int vol_col_id = 0;
                int physVol = rmg_man->GetNtupleID(physVolRegister); 
                ana_man->FillNtupleIColumn(physVol, vol_col_id++, physicalVolumeMappingID);
                ana_man->FillNtupleSColumn(physVol, vol_col_id++, nCPhysVol);
                ana_man->AddNtupleRow(physVol);
            }

            G4int physVolumeID = physVolumeMapping[nCPhysVol];
            
            if (materialMapping.find(nCMaterial) == materialMapping.end()) {
                const G4int materialMappingID = materialMapping.size();
                materialMapping.emplace(nCMaterial, materialMappingID);
                // Speichern
                int mat_col_id = 0;
                int material = rmg_man->GetNtupleID(materialRegister);
                ana_man->FillNtupleIColumn(material, mat_col_id++, materialMappingID);
                ana_man->FillNtupleSColumn(material, mat_col_id++, nCMaterial);
                ana_man->AddNtupleRow(material);
            }

            G4int materialID = materialMapping[nCMaterial];


            int col_id = 0;
            ana_man->FillNtupleIColumn(ntupleid, col_id++, event->GetEventID()); // Gleichzeitig Neutron ID
            // Neutron Capture Info:
            ana_man->FillNtupleIColumn(ntupleid, col_id++, nCTrackIDs[i]);
            ana_man->FillNtupleDColumn(ntupleid, col_id++, nCPositions[i]->GetX()/u::m);
            ana_man->FillNtupleDColumn(ntupleid, col_id++, nCPositions[i]->GetY()/u::m);
            ana_man->FillNtupleDColumn(ntupleid, col_id++, nCPositions[i]->GetZ()/u::m);
            ana_man->FillNtupleDColumn(ntupleid, col_id++, nCPhysVol);
            ana_man->FillNtupleDColumn(ntupleid, col_id++, nCMaterial);
            ana_man->FillNtupleIColumn(ntupleid, col_id++, nCGlobTimes[i]);
            ana_man->FillNtupleIColumn(ntupleid, col_id++, nCGammaTotalEnergies[i]/u::keV);
            ana_man->FillNtupleIColumn(ntupleid, col_id++, nCfGe77s[i]);
            ana_man->FillNtupleIColumn(ntupleid, col_id++, nCGammaAmounts[i]);           
            // Startet neue Reihe in Output
            ana_man->AddNtupleRow(ntupleid);
        }
    }
    }


    void MyNeutronCaptureOutputScheme::DefineCommands() {
    
    }

// vim: tabstop=2 shiftwidth=2 expandtab