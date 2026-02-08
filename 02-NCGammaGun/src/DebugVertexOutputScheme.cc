#include "DebugVertexOutputScheme.hh"

#include "G4AnalysisManager.hh"
#include "G4Event.hh"

#include "MyPrimaryGammaUserInfo.hh"
#include "RMGLog.hh"
#include "RMGManager.hh"

DebugVertexOutputScheme::DebugVertexOutputScheme() { 
  this->DefineCommands(); 
}

void DebugVertexOutputScheme::ClearBeforeEvent() {
  vertexMuonIDs.clear();
  vertexNCIDs.clear();
  vertexGammaIDs.clear();
}

void DebugVertexOutputScheme::AssignOutputNames(G4AnalysisManager* ana_man) {
  G4cout << "Debug: AssignOutputNames for DebugVertexOutputScheme" << G4endl;

  auto rmg_man = RMGManager::Instance();
  auto id = rmg_man->RegisterNtuple(OutputRegisterID,
      ana_man->CreateNtuple("DebugVertices", "Vertex debug data"));

  ana_man->CreateNtupleIColumn(id, "event_id");
  ana_man->CreateNtupleIColumn(id, "muon_id");
  ana_man->CreateNtupleIColumn(id, "nc_id");
  ana_man->CreateNtupleIColumn(id, "gamma_id");
  
  ana_man->FinishNtuple(id);
}

void DebugVertexOutputScheme::StoreEvent(const G4Event* event) {
  // Iterate over all primary vertices
  G4int nVertices = event->GetNumberOfPrimaryVertex();
  
  for (G4int i = 0; i < nVertices; ++i) {
    auto vertex = event->GetPrimaryVertex(i);
    if (!vertex) continue;
    
    // Extract UserInfo from vertex
    auto userInfo = dynamic_cast<MyPrimaryGammaUserInfo*>(vertex->GetUserInformation());
    if (!userInfo) {
      RMGLog::Out(RMGLog::warning, "Vertex ", i, " in event ", event->GetEventID(), 
                  " has no MyPrimaryGammaUserInfo");
      continue;
    }
    
    // Store IDs
    vertexMuonIDs.push_back(userInfo->GetMuonID());
    vertexNCIDs.push_back(userInfo->GetNCID());
    vertexGammaIDs.push_back(userInfo->GetGammaID());
  }
  
  // Write to ntuple
  auto rmg_man = RMGManager::Instance();
  if (rmg_man->IsPersistencyEnabled()) {
    const auto ana_man = G4AnalysisManager::Instance();
    auto ntupleid = rmg_man->GetNtupleID(OutputRegisterID);
    
    if (ntupleid < 0) {
      G4cerr << "❌ ERROR: Invalid Ntuple ID for DebugVertexOutputScheme!" << G4endl;
      return;
    }
    
    // One row per vertex
    for (size_t i = 0; i < vertexMuonIDs.size(); ++i) {
      int col_id = 0;
      ana_man->FillNtupleIColumn(ntupleid, col_id++, event->GetEventID());
      ana_man->FillNtupleIColumn(ntupleid, col_id++, vertexMuonIDs[i]);
      ana_man->FillNtupleIColumn(ntupleid, col_id++, vertexNCIDs[i]);
      ana_man->FillNtupleIColumn(ntupleid, col_id++, vertexGammaIDs[i]);
      
      ana_man->AddNtupleRow(ntupleid);
    }
  }
}

void DebugVertexOutputScheme::DefineCommands() {
  // Keine Commands nötig für Debug-Scheme
}
