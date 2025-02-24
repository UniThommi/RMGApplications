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

#ifndef _CUSTOM_ISOTOPE_FILER_HH_
#define _CUSTOM_ISOTOPE_FILER_HH_

#include <optional>
#include <set>

#include "G4AnalysisManager.hh"
#include "G4GenericMessenger.hh"
#include "G4VUserEventInformation.hh"

#include "RMGIsotopeFilterOutputScheme.hh"

class G4Event;
class CustomIsotopeFilter : public RMGIsotopeFilterOutputScheme {

public:
  void AssignOutputNames(G4AnalysisManager *ana_man) override;
  void StoreEvent(const G4Event *) override;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab
