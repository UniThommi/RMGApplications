#include "MyGammaCaptureOutputScheme.hh"

#include "G4AnalysisManager.hh"
#include "G4Event.hh"
#include "RMGLog.hh"
#include "RMGManager.hh"

namespace u = CLHEP;

// Initialize thread_local static member
thread_local std::vector<std::tuple<G4int, G4int, MyGammaCaptureOutputScheme::GammaInfo>>
    MyGammaCaptureOutputScheme::fPendingGammas;

MyGammaCaptureOutputScheme::MyGammaCaptureOutputScheme() {
    this->DefineCommands();
}

MyGammaCaptureOutputScheme::~MyGammaCaptureOutputScheme() {}

void MyGammaCaptureOutputScheme::AddPendingGamma(G4int ncID, G4int gammaID, const GammaInfo& gamma) {
    fPendingGammas.push_back(std::make_tuple(ncID, gammaID, gamma));
}

void MyGammaCaptureOutputScheme::ProcessPendingGammas() {
    for (const auto& entry : fPendingGammas) {
        G4int ncID = std::get<0>(entry);
        G4int gammaID = std::get<1>(entry);
        const auto& gamma = std::get<2>(entry);
        
        ncIDs.push_back(ncID);
        gammaIDs.push_back(gammaID);
        gammaMomenta.push_back(gamma.dir);
        gammaEnergies.push_back(gamma.energy);
        gammaPolarizations.push_back(gamma.polarization);
    }
    fPendingGammas.clear();
}

void MyGammaCaptureOutputScheme::ClearBeforeEvent() {
    ncIDs.clear();
    gammaIDs.clear();
    gammaMomenta.clear();
    gammaEnergies.clear();
    gammaPolarizations.clear();
}

void MyGammaCaptureOutputScheme::AssignOutputNames(G4AnalysisManager* ana_man) {
    G4cout << "Debug: AssignOutputNames for Gamma Output" << G4endl;

    auto rmg_man = RMGManager::Instance();
    auto gammasNTuple = rmg_man->RegisterNtuple(gammasRegisterID,
        ana_man->CreateNtuple("CaptureGammas", "Gamma data from neutron captures"));

    ana_man->CreateNtupleIColumn(gammasNTuple, "evtid");
    ana_man->CreateNtupleIColumn(gammasNTuple, "nc_id");
    ana_man->CreateNtupleIColumn(gammasNTuple, "gamma_id");
    ana_man->CreateNtupleDColumn(gammasNTuple, "gamma_px");
    ana_man->CreateNtupleDColumn(gammasNTuple, "gamma_py");
    ana_man->CreateNtupleDColumn(gammasNTuple, "gamma_pz");
    ana_man->CreateNtupleDColumn(gammasNTuple, "gamma_E_in_keV");
    ana_man->CreateNtupleDColumn(gammasNTuple, "gamma_pol_x");
    ana_man->CreateNtupleDColumn(gammasNTuple, "gamma_pol_y");
    ana_man->CreateNtupleDColumn(gammasNTuple, "gamma_pol_z");
    
    ana_man->FinishNtuple(gammasNTuple);
}

void MyGammaCaptureOutputScheme::StoreEvent(const G4Event* event) {
    // Erst pending Gammas verarbeiten
    ProcessPendingGammas();
    
    auto rmg_man = RMGManager::Instance();
    if (rmg_man->IsPersistencyEnabled()) {
        RMGLog::OutDev(RMGLog::debug, "Filling gamma persistent data vectors");
        const auto ana_man = G4AnalysisManager::Instance();
        auto ntupleid = rmg_man->GetNtupleID(gammasRegisterID);
        
        if (ntupleid < 0) {
            G4cerr << "❌ ERROR: Invalid Gamma Ntuple ID! Data will not be saved." << G4endl;
            return;
        }

        for (size_t i = 0; i < gammaEnergies.size(); ++i) {
            int col_id = 0;
            ana_man->FillNtupleIColumn(ntupleid, col_id++, event->GetEventID());
            ana_man->FillNtupleIColumn(ntupleid, col_id++, ncIDs[i]);
            ana_man->FillNtupleIColumn(ntupleid, col_id++, gammaIDs[i]);
            ana_man->FillNtupleDColumn(ntupleid, col_id++, gammaMomenta[i].x());
            ana_man->FillNtupleDColumn(ntupleid, col_id++, gammaMomenta[i].y());
            ana_man->FillNtupleDColumn(ntupleid, col_id++, gammaMomenta[i].z());
            ana_man->FillNtupleDColumn(ntupleid, col_id++, gammaEnergies[i] / u::keV);
            ana_man->FillNtupleDColumn(ntupleid, col_id++, gammaPolarizations[i].x());
            ana_man->FillNtupleDColumn(ntupleid, col_id++, gammaPolarizations[i].y());
            ana_man->FillNtupleDColumn(ntupleid, col_id++, gammaPolarizations[i].z());
            
            ana_man->AddNtupleRow(ntupleid);
        }
    }
}

void MyGammaCaptureOutputScheme::DefineCommands() {
    // Placeholder für zukünftige Messenger-Commands
}

// vim: tabstop=2 shiftwidth=2 expandtab