// Output scheme for the neutron capture (NC) truth, see MyNeutronCaptureOutputScheme.hh.
// MySteppingAction buffers the NCs of an event via AddPendingNC(); StoreEvent() moves
// them into the per-event vectors and writes one ntuple row per NC.

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

// Initialise thread_local static member
thread_local std::vector<std::tuple<G4int, MyNeutronCaptureOutputScheme::NCInfo>>
    MyNeutronCaptureOutputScheme::fPendingNCs;

MyNeutronCaptureOutputScheme::MyNeutronCaptureOutputScheme() {
  this->DefineCommands();
}

MyNeutronCaptureOutputScheme::~MyNeutronCaptureOutputScheme() {};

void MyNeutronCaptureOutputScheme::AddPendingNC(G4int ncID, const NCInfo& ncInfo) {
    fPendingNCs.push_back(std::make_tuple(ncID, ncInfo));}

// Moves the buffered NCs into the per-event vectors. A volume or material name that
// has not been seen before gets the next free ID, and the new (ID, name) pair is
// written to the mapping ntuple at once.
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
        gammaDirs1.push_back(nc.gamma1_dir);
        gammaEs1.push_back(nc.gamma1_E);
        gammaDirs2.push_back(nc.gamma2_dir);
        gammaEs2.push_back(nc.gamma2_E);
        gammaDirs3.push_back(nc.gamma3_dir);
        gammaEs3.push_back(nc.gamma3_E);
        gammaDirs4.push_back(nc.gamma4_dir);
        gammaEs4.push_back(nc.gamma4_E);
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
    gammaDirs1.clear(); gammaEs1.clear();
    gammaDirs2.clear(); gammaEs2.clear();
    gammaDirs3.clear(); gammaEs3.clear();
    gammaDirs4.clear(); gammaEs4.clear();
};

// BEGIN UNUSED CODE — no-op override of an empty base-class hook
void MyNeutronCaptureOutputScheme::TrackingActionPre(const G4Track* aTrack) {
    // Not used anymore - data comes via AddPendingNC
}
// END UNUSED CODE

// invoked in RMGRunAction::SetupAnalysisManager()
void MyNeutronCaptureOutputScheme::AssignOutputNames(G4AnalysisManager* ana_man) {
    G4cout << "Debug: AssignOutputNames" << G4endl;

    auto rmg_man = RMGManager::Instance();
    auto neutronsNTuple = rmg_man->RegisterNtuple(NCsRegisterID,
        ana_man->CreateNtuple("MyNeutronCaptureOutput", "Event data"));

    ana_man->CreateNtupleIColumn(neutronsNTuple, "evtid");
    // Create column structure to save data
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
    for (int i = 1; i <= 4; ++i) {
        ana_man->CreateNtupleDColumn(neutronsNTuple, "gamma" + std::to_string(i) + "_px");
        ana_man->CreateNtupleDColumn(neutronsNTuple, "gamma" + std::to_string(i) + "_py");
        ana_man->CreateNtupleDColumn(neutronsNTuple, "gamma" + std::to_string(i) + "_pz");
        ana_man->CreateNtupleDColumn(neutronsNTuple, "gamma" + std::to_string(i) + "_E_in_keV");
    }

    ana_man->FinishNtuple(neutronsNTuple);

    // Save the mappings
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
    // Process the pending NCs first
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

        // Positions in m, energies in keV; the time is in Geant4 internal units (ns).
        for (size_t i = 0; i < nCTrackIDs.size(); ++i) {
            int col_id = 0;
            // Event ID: identifies the primary (one MUSUN muon per event, or one neutron
            // per event in the homogeneous-NC simulation)
            ana_man->FillNtupleIColumn(ntupleid, col_id++, event->GetEventID());
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

            // Top-4 Gammas
            ana_man->FillNtupleDColumn(ntupleid, col_id++, gammaDirs1[i].x());
            ana_man->FillNtupleDColumn(ntupleid, col_id++, gammaDirs1[i].y());
            ana_man->FillNtupleDColumn(ntupleid, col_id++, gammaDirs1[i].z());
            ana_man->FillNtupleDColumn(ntupleid, col_id++, gammaEs1[i] / u::keV);

            ana_man->FillNtupleDColumn(ntupleid, col_id++, gammaDirs2[i].x());
            ana_man->FillNtupleDColumn(ntupleid, col_id++, gammaDirs2[i].y());
            ana_man->FillNtupleDColumn(ntupleid, col_id++, gammaDirs2[i].z());
            ana_man->FillNtupleDColumn(ntupleid, col_id++, gammaEs2[i] / u::keV);

            ana_man->FillNtupleDColumn(ntupleid, col_id++, gammaDirs3[i].x());
            ana_man->FillNtupleDColumn(ntupleid, col_id++, gammaDirs3[i].y());
            ana_man->FillNtupleDColumn(ntupleid, col_id++, gammaDirs3[i].z());
            ana_man->FillNtupleDColumn(ntupleid, col_id++, gammaEs3[i] / u::keV);

            ana_man->FillNtupleDColumn(ntupleid, col_id++, gammaDirs4[i].x());
            ana_man->FillNtupleDColumn(ntupleid, col_id++, gammaDirs4[i].y());
            ana_man->FillNtupleDColumn(ntupleid, col_id++, gammaDirs4[i].z());
            ana_man->FillNtupleDColumn(ntupleid, col_id++, gammaEs4[i] / u::keV);

            // Starts a new row in the output
            ana_man->AddNtupleRow(ntupleid);
        }
    }
}


    // No messenger commands are defined; kept as a placeholder.
    void MyNeutronCaptureOutputScheme::DefineCommands() {

    }

// vim: tabstop=2 shiftwidth=2 expandtab