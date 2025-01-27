// Copyright (C) 2022 Luigi Pertoldi <gipert@pm.me>
//
// This program is free software: you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License as published by the Free
// Software Foundation, either version 3 of the License, or (at your option) any
// later version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
// details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#include "NeutronsOutputScheme.hh"

#include <set>

#include "G4AnalysisManager.hh"
#include "G4Event.hh"
#include "G4EventManager.hh"
#include "G4HCtable.hh"
#include "G4OpticalPhoton.hh"
#include "G4SDManager.hh"

#include "RMGHardware.hh"
#include "RMGLog.hh"
#include "RMGManager.hh"

namespace u = CLHEP;

NeutronsOutputScheme::NeutronsOutputScheme() { this->DefineCommands(); }

void NeutronsOutputScheme::ClearBeforeEvent() {
  Capture_Positions.clear();
  zOfEvent.clear();
  aOfEvent.clear();
}

// Need information of isotop creation here as well. Could also get from other IsotopeFilterOutputscheme.
void NeutronsOutputScheme::TrackingActionPre(const G4Track* aTrack) {
  const auto particle = aTrack->GetParticleDefinition();
  // if (!particle->IsGeneralIon()) return;
  const int z = particle->GetAtomicNumber();
  const int a = particle->GetAtomicMass();
  if (z != 0 || a != 1) return; // Hard coded for Neutrons.

  // Save the locations of Neutrons creation
  Capture_Positions.push_back(aTrack->GetVertexPosition()); 
  zOfEvent.push_back(z);
  aOfEvent.push_back(a);
}

// invoked in RMGRunAction::SetupAnalysisManager()
void NeutronsOutputScheme::AssignOutputNames(G4AnalysisManager* ana_man) {  

  auto rmg_man = RMGManager::Instance();

  auto id = rmg_man->RegisterNtuple(OutputRegisterID,
      ana_man->CreateNtuple("NeutronsOutput", "Event data"));

  ana_man->CreateNtupleIColumn(id, "evtid");
  // Hier könnte man noch NTuple kreieren die Informationen speichern, z.B Entstehungsort, ob Neutronen Multiplizitäskriterien erfüllen usw.
  // Für Debug Zwecke: 
  ana_man->CreateNtupleIColumn(id, "Z");
  ana_man->CreateNtupleIColumn(id, "A");
  ana_man->FinishNtuple(id);
}

// Braucht man das für Neutronen?
RMGOpticalDetectorHitsCollection* NeutronsOutputScheme::GetOptHitColl(const G4Event* event) {  // Gets hit collection from sensitive Ge detectors with optical information.
  auto sd_man = G4SDManager::GetSDMpointer();

  auto hit_coll_id = sd_man->GetCollectionID("Optical/Hits");
  if (hit_coll_id < 0) {
    RMGLog::OutDev(RMGLog::error, "Could not find hit collection Optical/Hits");
    return nullptr;
  }

  auto hit_coll =
      dynamic_cast<RMGOpticalDetectorHitsCollection*>(event->GetHCofThisEvent()->GetHC(hit_coll_id));

  if (!hit_coll) {
    RMGLog::Out(RMGLog::error, "Could not find hit collection associated with event");
    return nullptr;
  }

  return hit_coll;
}

// invoked in RMGEventAction::EndOfEventAction()
bool NeutronsOutputScheme::ShouldDiscardEvent(const G4Event* event) {  // Keine Neutron events werden discarded
  return false;
}

// invoked in RMGEventAction::EndOfEventAction()
void NeutronsOutputScheme::StoreEvent(const G4Event* event) {  // Speichert events in G4AnalysisManager //FIX!!!

  auto rmg_man = RMGManager::Instance();
  if (rmg_man->IsPersistencyEnabled()) { 
    RMGLog::OutDev(RMGLog::debug, "Filling persistent data vectors");
    const auto ana_man = G4AnalysisManager::Instance();

    auto ntupleid = rmg_man->GetNtupleID(OutputRegisterID);
    int col_id = 0;
    ana_man->FillNtupleIColumn(ntupleid, col_id++, event->GetEventID());
    ana_man->FillNtupleIColumn(ntupleid, col_id++, zOfEvent[i]);
    ana_man->FillNtupleIColumn(ntupleid, col_id++, aOfEvent[i]);

    // NOTE: must be called here for hit-oriented output
    ana_man->AddNtupleRow(ntupleid); // Muss das sein für Neutronen???
  }
}

void NeutronsOutputScheme::DefineCommands() {

}

// vim: tabstop=2 shiftwidth=2 expandtab