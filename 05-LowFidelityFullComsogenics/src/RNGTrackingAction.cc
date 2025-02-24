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

#include "RNGTrackingAction.hh"

#include "G4OpticalPhoton.hh"
#include "G4Track.hh"
#include "Randomize.hh"

G4ThreadLocal CLHEP::HepRandomEngine *RNGTrackingAction::fAlternativeEngine =
    nullptr;

RNGTrackingAction::RNGTrackingAction() {
  // Initialize alternative RNG engine per thread
  if (!fAlternativeEngine) {
    fAlternativeEngine = new CLHEP::RanecuEngine;
  }
}

void RNGTrackingAction::PreUserTrackingAction(const G4Track *aTrack) {

  auto particle = aTrack->GetDefinition();
  if (particle == G4OpticalPhoton::OpticalPhotonDefinition()) {
    defaultEngine = G4Random::getTheEngine();

    G4Random::setTheEngine(fAlternativeEngine);
    ResetRNG = true;
  }
}

void RNGTrackingAction::PostUserTrackingAction(const G4Track * /*aTrack*/) {
  if (ResetRNG) {
    G4Random::setTheEngine(defaultEngine);
    ResetRNG = false;
  }
}

// vim: tabstop=2 shiftwidth=2 expandtab
