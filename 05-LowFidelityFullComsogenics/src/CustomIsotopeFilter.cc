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

#include "CustomIsotopeFilter.hh"

#include <set>

#include "G4Electron.hh"
#include "G4Event.hh"
#include "G4EventManager.hh"
#include "G4OpticalPhoton.hh"
#include "G4Positron.hh"
#include "G4RunManager.hh"

#include "RMGLog.hh"
#include "RMGManager.hh"

namespace u = CLHEP;

void CustomIsotopeFilter::AssignOutputNames(G4AnalysisManager *ana_man) {

  auto vid = RMGManager::Instance()->RegisterNtuple(
      -10, ana_man->CreateNtuple("musun", "Primary vertex data"));

  ana_man->CreateNtupleIColumn(vid, "evtid");
  ana_man->CreateNtupleIColumn(vid, "type");
  ana_man->CreateNtupleDColumn(vid, "Ekin");
  ana_man->CreateNtupleDColumn(vid, "x");
  ana_man->CreateNtupleDColumn(vid, "y");
  ana_man->CreateNtupleDColumn(vid, "z");
  ana_man->CreateNtupleDColumn(vid, "px");
  ana_man->CreateNtupleDColumn(vid, "py");
  ana_man->CreateNtupleDColumn(vid, "pz");

  ana_man->FinishNtuple(vid);
}

void CustomIsotopeFilter::StoreEvent(const G4Event *event) {

  if (ShouldDiscardEvent(event))
    return;
  // stores the random seed for this event to be reproducible
  G4RunManager::GetRunManager()->rndmSaveThisEvent();

  auto rmg_man = RMGManager::Instance();
  const auto ana_man = G4AnalysisManager::Instance();
  auto vntupleid = rmg_man->GetNtupleID(-10);

  auto primary_vertex = event->GetPrimaryVertex(0);
  int n_primaries = primary_vertex->GetNumberOfParticle();
  if (n_primaries != 1)
    RMGLog::Out(RMGLog::fatal, "More than one primary was found! This is not "
                               "expected in this Cosmogenic simulation.");

  auto primary = primary_vertex->GetPrimary(0);

  int vcol_id = 0;
  ana_man->FillNtupleIColumn(vntupleid, vcol_id++, event->GetEventID());

  G4ParticleTable *theParticleTable = G4ParticleTable::GetParticleTable();
  if (primary->GetParticleDefinition() ==
      theParticleTable->FindParticle("mu-")) {
    ana_man->FillNtupleIColumn(vntupleid, vcol_id++, 10);
  } else {
    if (primary->GetParticleDefinition() !=
        theParticleTable->FindParticle("mu+"))
      RMGLog::Out(RMGLog::fatal,
                  "Primary is not a muon. (This is a cosmogenic simulation)");
    ana_man->FillNtupleIColumn(vntupleid, vcol_id++, 11);
  }

  ana_man->FillNtupleDColumn(vntupleid, vcol_id++,
                             primary->GetKineticEnergy() / u::GeV);
  ana_man->FillNtupleDColumn(vntupleid, vcol_id++,
                             primary_vertex->GetX0() / u::cm);
  ana_man->FillNtupleDColumn(vntupleid, vcol_id++,
                             primary_vertex->GetY0() / u::cm);
  ana_man->FillNtupleDColumn(vntupleid, vcol_id++,
                             primary_vertex->GetZ0() / u::cm);
  ana_man->FillNtupleDColumn(vntupleid, vcol_id++,
                             primary->GetMomentumDirection().getX());
  ana_man->FillNtupleDColumn(vntupleid, vcol_id++,
                             primary->GetMomentumDirection().getY());
  ana_man->FillNtupleDColumn(vntupleid, vcol_id++,
                             primary->GetMomentumDirection().getZ());

  // NOTE: must be called here for hit-oriented output
  ana_man->AddNtupleRow(vntupleid);
}

// vim: tabstop=2 shiftwidth=2 expandtab
