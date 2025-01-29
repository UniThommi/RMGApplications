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

#include "CosmogenicOutputScheme.hh"

#include <set>

#include "G4AnalysisManager.hh"
#include "G4Event.hh"
#include "G4EventManager.hh"
#include "G4HCtable.hh"
#include "G4OpticalPhoton.hh"
#include "G4SDManager.hh"

#include "RMGGermaniumDetector.hh"
#include "RMGHardware.hh"
#include "RMGLog.hh"
#include "RMGManager.hh"

namespace u = CLHEP;

CosmogenicOutputScheme::CosmogenicOutputScheme() { this->DefineCommands(); }

void CosmogenicOutputScheme::ClearBeforeEvent() {
  Capture_Positions.clear();
}

// Need information of isotop creation here as well. Could also get from other IsotopeFilterOutputscheme.
void CosmogenicOutputScheme::TrackingActionPre(const G4Track* aTrack) {
  const auto particle = aTrack->GetParticleDefinition();
  if (!particle->IsGeneralIon()) return;
  const int z = particle->GetAtomicNumber();
  const int a = particle->GetAtomicMass();
  if (z != 32 || a != 77) return; // Hard coded for Ge77.

  // Save the locations of Ge77 creation
  Capture_Positions.push_back(aTrack->GetVertexPosition()); 
}

// invoked in RMGRunAction::SetupAnalysisManager()
void CosmogenicOutputScheme::AssignOutputNames(G4AnalysisManager* ana_man) {  

  auto rmg_man = RMGManager::Instance();

  auto id = rmg_man->RegisterNtuple(OutputRegisterID,
      ana_man->CreateNtuple("CosmogenicOutput", "Event data"));

  ana_man->CreateNtupleIColumn(id, "evtid");
  // Save the 3 flags
  ana_man->CreateNtupleIColumn(id, "GeFlag");
  ana_man->CreateNtupleIColumn(id, "ArgonFlag");
  ana_man->CreateNtupleIColumn(id, "WaterFlag");

  ana_man->FinishNtuple(id);
}

RMGGermaniumDetectorHitsCollection* CosmogenicOutputScheme::GetGeHitColl(const G4Event* event) {
  auto sd_man = G4SDManager::GetSDMpointer();

  auto hit_coll_id = sd_man->GetCollectionID("Germanium/Hits");
  if (hit_coll_id < 0) {
    RMGLog::OutDev(RMGLog::error, "Could not find hit collection Germanium/Hits");
    return nullptr;
  }

  auto hit_coll = dynamic_cast<RMGGermaniumDetectorHitsCollection*>(
      event->GetHCofThisEvent()->GetHC(hit_coll_id));

  if (!hit_coll) {
    RMGLog::Out(RMGLog::error, "Could not find hit collection associated with event");
    return nullptr;
  }

  return hit_coll;
}

// Could summarize these functions into one, but this is more readable i think
RMGOpticalDetectorHitsCollection* CosmogenicOutputScheme::GetOptHitColl(const G4Event* event) {  // Gets hit collection from sensitive Ge detectors with optical information.
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
bool CosmogenicOutputScheme::ShouldDiscardEvent(const G4Event* event) {  // Keine Ge events werden discarded
  return false;
}

// invoked in RMGEventAction::EndOfEventAction()
void CosmogenicOutputScheme::StoreEvent(const G4Event* event) {  // Speichert events in G4AnalysisManager
  auto Ge_hit_coll = GetGeHitColl(event);
  if (!Ge_hit_coll) {
    RMGLog::Out(RMGLog::error, "No germanium hit collection!");
  }

  if (Ge_hit_coll->entries() <= 0) {
    RMGLog::OutDev(RMGLog::debug, "Germanium hit collection is empty");
  } else {
    RMGLog::OutDev(RMGLog::debug, "Germanium hit collection contains ", Ge_hit_coll->entries(), " hits");
  }

  auto Opt_hit_coll = GetOptHitColl(event);
  // Optical hit collection can be empty!
  if (!Opt_hit_coll) {
    RMGLog::Out(RMGLog::error, "No optical hit collection!");
  }
  if (Opt_hit_coll->entries() <= 0) {
    RMGLog::OutDev(RMGLog::debug, "Optical hit collection is empty");
  } else {
    RMGLog::OutDev(RMGLog::debug, "Optical hit collection contains ", Opt_hit_coll->entries(), " hits");
  }

  auto rmg_man = RMGManager::Instance();
  if (rmg_man->IsPersistencyEnabled()) { 
    RMGLog::OutDev(RMGLog::debug, "Filling persistent data vectors");
    const auto ana_man = G4AnalysisManager::Instance();

    int Ge_flag = GetGeFlag(Ge_hit_coll);                       // Checkt ob events innerhalb eines Zeitintervalls (1ms) und mindest deponierte Energie >100keV und Abstand zum nächsten string gering.
    int Water_flag = GetWaterFlag(Opt_hit_coll);                // Checkt events nach ob Muon Veto und Neutron Tagger Veto aktiviert wird.

    auto ntupleid = rmg_man->GetNtupleID(OutputRegisterID);
    int col_id = 0;
    ana_man->FillNtupleIColumn(ntupleid, col_id++, event->GetEventID());
    ana_man->FillNtupleIColumn(ntupleid, col_id++, Ge_flag);
    ana_man->FillNtupleIColumn(ntupleid, col_id++, 0); // No argon flag so far
    ana_man->FillNtupleIColumn(ntupleid, col_id++, Water_flag);

    // NOTE: must be called here for hit-oriented output
    ana_man->AddNtupleRow(ntupleid);
  }
}

int CosmogenicOutputScheme::GetGeFlag(const RMGGermaniumDetectorHitsCollection * hit_coll)
{
  // Because i want to differentiate between having vetoed no captures at all, or having vetoed some but not all
  bool detected_one = false;
  bool missed_one = false;

  if(Capture_Positions.size() == 0)
    throw std::runtime_error("No capture positions found!");

  // Check all capture positions and check if all of them would be vetoed
  for (auto c_position : Capture_Positions) {
    double edep = 0.;
    // Sum energy deposition in range of this capture position
    for (auto hit : *hit_coll->GetVector()) {
      if (!hit) continue;

      if(hit->global_time / u::ns > 1e6) // Only check hits within 1 millisecond
        continue;
      
      //if(hit->global_time / u::ns < 1e4) // Only check hits after 10 microseconds
      //  continue;

      G4ThreeVector hit_position(hit->global_position.getX(), hit->global_position.getY(), hit->global_position.getZ());
      double distance = (c_position - hit_position).mag(); // TODO check if units match?

      // Is within adjacent string?
      if((distance / u::cm) < (dist_to_next_string + gerad)) {            
        edep += hit->energy_deposition;                                // Greift auf energy_deposition vom Zeiger hit zu und addiert das zur deponierten Energie.
      }
    }

    if(edep > 100 * u::keV)                         
      detected_one = true;
    else 
      missed_one = true;
  }
  
  // return 0 if nothing was vetoed. Return 1 if everything was vetoed. Return 2 if some were vetoed but not all
  if(missed_one) {
    if(detected_one)
      return 2;
    else
      return 0;
  }
  else
   return 1;
}


int CosmogenicOutputScheme::GetWaterFlag(const RMGOpticalDetectorHitsCollection * hit_coll)
{
  // First get the muon veto flag (60ns coincidence)
  // Check in the first microsecond for a multiplicity of 6 within 60ns
  const double coincidence_window = 60 * u::ns;
  const double muon_max_time = 1 * u::microsecond;
  const double neutron_max_time = 200 * u::microsecond;
  const int required_multiplicity = 6;        // Für muon Veto

  std::vector<std::pair<double, int>> hit_times_and_ids;
  for (auto hit : *hit_coll->GetVector()) {
    if (!hit) continue;
    double time = hit->global_time / u::ns;
    int detector_id = hit->detector_uid;
    if (time < muon_max_time) {
      hit_times_and_ids.emplace_back(time, detector_id);
    }
  }

  std::sort(hit_times_and_ids.begin(), hit_times_and_ids.end());

  bool muon_veto_flag = false;
  for (size_t i = 0; i < hit_times_and_ids.size(); ++i) {
    std::set<int> unique_detectors;
    unique_detectors.insert(hit_times_and_ids[i].second);
    for (size_t j = i + 1; j < hit_times_and_ids.size(); ++j) {
      if (hit_times_and_ids[j].first - hit_times_and_ids[i].first <= coincidence_window) {
        unique_detectors.insert(hit_times_and_ids[j].second);
        if (unique_detectors.size() >= required_multiplicity) {
          muon_veto_flag = true; // Muon veto flag triggered
          break;
        }
      } else {
        break;
      }
    }
    if (muon_veto_flag) break;
  }

  // Secondly, check for the neutron tagger flag. The neutron tagger only checks events after 1 microsecond.
  // The neutron tagger requires the muon veto flag to be triggered. The neutron tagger flag creates 200ns time bins
  // and checks for a multiplicity of at least 6. If 15 bins have a multiplicity of 6, the flag is triggered.

  if (muon_veto_flag) {
    const double bin_width = 200 * u::ns;
    const int required_bins = 15;

    std::vector<double> neutron_hit_times;
    for (auto hit : *hit_coll->GetVector()) {
      if (!hit) continue;
      double time = hit->global_time / u::ns;
      if (time >= muon_max_time && time < neutron_max_time) {
        neutron_hit_times.push_back(time);
      }
    }

    std::sort(neutron_hit_times.begin(), neutron_hit_times.end());

    std::map<int, int> time_bins;
    for (double time : neutron_hit_times) {
      int bin = static_cast<int>(time / bin_width);
      time_bins[bin]++;
    }

    int bins_with_multiplicity = 0;
    for (const auto& bin : time_bins) {
      if (bin.second >= required_multiplicity) {
        bins_with_multiplicity++;
        if (bins_with_multiplicity >= required_bins) {
          return 1; // Both muon veto and neutron tagger flags triggered
        }
      }
    }

    return 2; // Only muon veto flag triggered
  }

  return 0; // No flag triggered
}

void CosmogenicOutputScheme::DefineCommands() {

}

// vim: tabstop=2 shiftwidth=2 expandtab