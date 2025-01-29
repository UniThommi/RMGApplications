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

#include "G4Neutron.hh"

#include "RMGHardware.hh"
#include "RMGLog.hh"
#include "RMGManager.hh"

namespace u = CLHEP;

NeutronsOutputScheme::NeutronsOutputScheme() { this->DefineCommands(); }

void NeutronsOutputScheme::ClearBeforeEvent() {
  vertexPositions.clear();

  zOfEvents.clear();
  aOfEvents.clear();
}

// Need information of isotop creation here as well. Could also get from other IsotopeFilterOutputscheme.
void NeutronsOutputScheme::TrackingActionPre(const G4Track* aTrack) {
  const auto particle = aTrack->GetParticleDefinition();
  const int z = particle->GetAtomicNumber();
  const int a = particle->GetAtomicMass();
  if (particle != G4Neutron::NeutronDefinition()) return; // Hard coded for Neutrons.

  vertexPositions.push_back(aTrack->GetVertexPosition()); // Save the locations of Neutrons creation
  vertexMomentums.push_back(aTrack->GetVertexMomentumDirection());
  globalTimes.push_back(aTrack->GetGlobalTime());
  vertexKineticEnergies.push_back(aTrack->GetVertexKineticEnergy());
  trackLengths.push_back(aTrack->GetTrackLength());
  zOfEvents.push_back(z);
  aOfEvents.push_back(a);
}

// invoked in RMGRunAction::SetupAnalysisManager()
void NeutronsOutputScheme::AssignOutputNames(G4AnalysisManager* ana_man) {  

  auto rmg_man = RMGManager::Instance();

  auto id = rmg_man->RegisterNtuple(OutputRegisterID,
      ana_man->CreateNtuple("NeutronsOutput", "Event data"));

  ana_man->CreateNtupleIColumn(id, "evtid");
  // Hier könnte man noch NTuple kreieren die Informationen speichern, z.B Entstehungsort, ob Neutronen Multiplizitäskriterien erfüllen usw.
  // Für Debug Zwecke: 
  ana_man->CreateNtupleDColumn(id, "x_position_in_m");
  ana_man->CreateNtupleDColumn(id, "y_position_in_m");
  ana_man->CreateNtupleDColumn(id, "z_position_in_m");
  ana_man->CreateNtupleDColumn(id, "x_momentum_in_m");
  ana_man->CreateNtupleDColumn(id, "y_momentum_in_m");
  ana_man->CreateNtupleDColumn(id, "z_momentum_in_m");
  ana_man->CreateNtupleDColumn(id, "global_time");
  ana_man->CreateNtupleDColumn(id, "kinetic_energy_in_?");
  ana_man->CreateNtupleDColumn(id, "track_length_in_m");
  ana_man->CreateNtupleIColumn(id, "Z");
  ana_man->CreateNtupleIColumn(id, "A");
  ana_man->FinishNtuple(id);
}

// invoked in RMGEventAction::EndOfEventAction()
void NeutronsOutputScheme::StoreEvent(const G4Event* event) {  // Speichert events in G4AnalysisManager //FIX!!!

  auto rmg_man = RMGManager::Instance();
  if (rmg_man->IsPersistencyEnabled()) { 
    RMGLog::OutDev(RMGLog::debug, "Filling persistent data vectors");
    const auto ana_man = G4AnalysisManager::Instance();

    auto ntupleid = rmg_man->GetNtupleID(OutputRegisterID);

    for (size_t i = 0; i < zOfEvents.size(); ++i) {
      int col_id = 0;
      ana_man->FillNtupleIColumn(ntupleid, col_id++, event->GetEventID());
      ana_man->FillNtupleDColumn(ntupleid, col_id++, vertexPositions[i].getX())/u::m;
      ana_man->FillNtupleDColumn(ntupleid, col_id++, vertexPositions[i].getY())/u::m;
      ana_man->FillNtupleDColumn(ntupleid, col_id++, vertexPositions[i].getZ())/u::m;  // Standard Einheit ist mm, bei Definition mit Einheit in m *u::m -> mal 1000, bei Abfrage des Wertes in m /u::m -> geteilt durch 1000 für Rückrechnung
      // Füge hinzu : Kinetische Energie, Zeitpunkt der Entstehung (global, also seitdem das Muon erzeugt wurde), Impulsvektor (x, y, z), kinetische Energie
      ana_man->FillNtupleDColumn(ntupleid, col_id++, vertexMomentums[i].getX());
      ana_man->FillNtupleDColumn(ntupleid, col_id++, vertexMomentums[i].getY());
      ana_man->FillNtupleDColumn(ntupleid, col_id++, vertexMomentums[i].getZ()); 
      ana_man->FillNtupleDColumn(ntupleid, col_id++, globalTimes[i]);
      ana_man->FillNtupleDColumn(ntupleid, col_id++, vertexKineticEnergies[i]);
      ana_man->FillNtupleDColumn(ntupleid, col_id++, trackLengths[i]);
      ana_man->FillNtupleIColumn(ntupleid, col_id++, zOfEvents[i]);
      ana_man->FillNtupleIColumn(ntupleid, col_id++, aOfEvents[i]);
      // Startet neue Reihe in Output
      ana_man->AddNtupleRow(ntupleid);
    }
  }
}

void NeutronsOutputScheme::DefineCommands() {

}

// vim: tabstop=2 shiftwidth=2 expandtab