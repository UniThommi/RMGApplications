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

// Initialize thread_local static member
thread_local std::vector<std::tuple<G4int, MyNeutronCaptureOutputScheme::NCInfo>>
    MyNeutronCaptureOutputScheme::fPendingNCs;

MyNeutronCaptureOutputScheme::MyNeutronCaptureOutputScheme() { 
  this->DefineCommands(); 
}

MyNeutronCaptureOutputScheme::~MyNeutronCaptureOutputScheme() {};

void MyNeutronCaptureOutputScheme::AddPendingNC(G4int ncID, const NCInfo& ncInfo) {
    fPendingNCs.push_back(std::make_tuple(ncID, ncInfo));}

void MyNeutronCaptureOutputScheme::ProcessPendingNCs() {
    for (const auto& entry : fPendingNCs) {
        G4int ncID = std::get<0>(entry);
        const auto& nc = std::get<1>(entry);

        nCTrackIDs.push_back(ncID);
        nCPositions.push_back(nc.pos);
        nCGlobTimes.push_back(nc.time);
        
        // PhysVol mapping
        if (physVolumeMapping.find(nc.physVol) == physVolumeMapping.end()) {
            const G4int id = physVolumeMapping.size();
            physVolumeMapping.emplace(nc.physVol, id);
            
            auto rmg_man = RMGManager::Instance();
            auto ana_man = G4AnalysisManager::Instance();
            int vol_col_id = 0;
            int physVol = rmg_man->GetNtupleID(physVolRegister);
            ana_man->FillNtupleIColumn(physVol, vol_col_id++, id);
            ana_man->FillNtupleSColumn(physVol, vol_col_id++, nc.physVol);
            ana_man->AddNtupleRow(physVol);
        }
        nCPhysVolumeIDs.push_back(physVolumeMapping[nc.physVol]);

        // Material mapping
        if (materialMapping.find(nc.material) == materialMapping.end()) {
            const G4int id = materialMapping.size();
            materialMapping.emplace(nc.material, id);
            
            auto rmg_man = RMGManager::Instance();
            auto ana_man = G4AnalysisManager::Instance();
            int mat_col_id = 0;
            int material = rmg_man->GetNtupleID(materialRegister);
            ana_man->FillNtupleIColumn(material, mat_col_id++, id);
            ana_man->FillNtupleSColumn(material, mat_col_id++, nc.material);
            ana_man->AddNtupleRow(material);
        }
        nCMaterialIDs.push_back(materialMapping[nc.material]);

        nCGammaAmounts.push_back(nc.gammaAmount);
        nCGammaTotalEnergies.push_back(nc.gammaTotalEnergy);
        nCfGe77s.push_back(nc.fGe77);
    }
    fPendingNCs.clear();
}

void MyNeutronCaptureOutputScheme::ClearBeforeEvent() {
    // Neutron Capture Info
    nCTrackIDs.clear();
    nCPositions.clear();
    nCPhysVolumeIDs.clear();
    nCMaterialIDs.clear();
    nCGlobTimes.clear();
    nCGammaTotalEnergies.clear();
    nCGammaAmounts.clear();
    nCfGe77s.clear();
};

void MyNeutronCaptureOutputScheme::TrackingActionPre(const G4Track* aTrack) {
    // Not used anymore - data comes via AddPendingNC
}

// invoked in RMGRunAction::SetupAnalysisManager()
void MyNeutronCaptureOutputScheme::AssignOutputNames(G4AnalysisManager* ana_man) {  
    G4cout << "Debug: AssignOutputNames" << G4endl;

    auto rmg_man = RMGManager::Instance();
    auto neutronsNTuple = rmg_man->RegisterNtuple(NCsRegisterID,
        ana_man->CreateNtuple("MyNeutronCaptureOutput", "Event data"));

    ana_man->CreateNtupleIColumn(neutronsNTuple, "evtid");
    // Create column structure to safe data
    ana_man->CreateNtupleIColumn(neutronsNTuple, "nC_track_id");
    ana_man->CreateNtupleDColumn(neutronsNTuple, "nC_x_position_in_m");
    ana_man->CreateNtupleDColumn(neutronsNTuple, "nC_y_position_in_m");
    ana_man->CreateNtupleDColumn(neutronsNTuple, "nC_z_position_in_m");
    ana_man->CreateNtupleIColumn(neutronsNTuple, "nC_phys_vol_id");
    ana_man->CreateNtupleIColumn(neutronsNTuple, "nC_material_id");
    ana_man->CreateNtupleDColumn(neutronsNTuple, "nC_time_in_ns");
    ana_man->CreateNtupleIColumn(neutronsNTuple, "nC_gamma_amount");
    ana_man->CreateNtupleDColumn(neutronsNTuple, "nC_gamma_total_energy_in_keV");
    ana_man->CreateNtupleIColumn(neutronsNTuple, "nC_flag_Ge77");

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
    // Erst pending NCs verarbeiten
    ProcessPendingNCs();
    auto rmg_man = RMGManager::Instance();
    if (rmg_man->IsPersistencyEnabled()) { 
        RMGLog::OutDev(RMGLog::debug, "Filling persistent data vectors");
        const auto ana_man = G4AnalysisManager::Instance();
        auto ntupleid = rmg_man->GetNtupleID(NCsRegisterID);
        if (ntupleid < 0) {
            G4cerr << "❌ ERROR: Invalid Ntuple ID! Data will not be saved." << G4endl;
            return;
        }   

        for (size_t i = 0; i < nCTrackIDs.size(); ++i) {
            int col_id = 0;
            ana_man->FillNtupleIColumn(ntupleid, col_id++, event->GetEventID()); // Gleichzeitig Neutron ID
            // Neutron Capture Info:
            ana_man->FillNtupleIColumn(ntupleid, col_id++, nCTrackIDs[i]);
            ana_man->FillNtupleDColumn(ntupleid, col_id++, nCPositions[i].getX()/u::m);
            ana_man->FillNtupleDColumn(ntupleid, col_id++, nCPositions[i].getY()/u::m);
            ana_man->FillNtupleDColumn(ntupleid, col_id++, nCPositions[i].getZ()/u::m);
            ana_man->FillNtupleIColumn(ntupleid, col_id++, nCPhysVolumeIDs[i]);
            ana_man->FillNtupleIColumn(ntupleid, col_id++, nCMaterialIDs[i]);
            ana_man->FillNtupleDColumn(ntupleid, col_id++, nCGlobTimes[i]);
            ana_man->FillNtupleIColumn(ntupleid, col_id++, nCGammaAmounts[i]);
            ana_man->FillNtupleDColumn(ntupleid, col_id++, nCGammaTotalEnergies[i]/u::keV);
            ana_man->FillNtupleIColumn(ntupleid, col_id++, nCfGe77s[i]);
           
            // Startet neue Reihe in Output
            ana_man->AddNtupleRow(ntupleid);
        }
    }
}


    void MyNeutronCaptureOutputScheme::DefineCommands() {
    
    }

// vim: tabstop=2 shiftwidth=2 expandtab