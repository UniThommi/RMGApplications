#include "NeutronsOutputScheme.hh"

#include <set>
#include <algorithm>
#include <iostream>
#include <string>


#include "G4AnalysisManager.hh"
#include "G4Event.hh"
#include "G4EventManager.hh"
#include "G4HCtable.hh"
#include "G4OpticalPhoton.hh"
#include "G4SDManager.hh"
#include "G4Neutron.hh"

#include "MyTrackInfo.hh"
#include "MyRunMappingAction.hh"
#include "MyGe77EventFilterOutputScheme.hh"
#include "RMGHardware.hh"
#include "RMGLog.hh"
#include "RMGManager.hh"

namespace u = CLHEP;

NeutronsOutputScheme::NeutronsOutputScheme() { 
  this->DefineCommands(); 
}


void NeutronsOutputScheme::ClearBeforeEvent() {
  vertexPositions.clear();
  vertexMomentums.clear();
  globalTimes.clear();
  vertexKineticEnergies.clear();
  physicalVolumes.clear();
  materials.clear();
  fGe77Produced = false;
};

// Need information of isotop creation here as well. Could also get from other IsotopeFilterOutputscheme.
void NeutronsOutputScheme::TrackingActionPre(const G4Track* aTrack) { 
  // Check if the track is a neutron
  auto* info = aTrack->GetDefinition();
  auto* trackInfo = dynamic_cast<MyTrackInfo*>(aTrack->GetUserInformation());

  // Schaue ob gen1NeutronID des Tracks in killableIDs ist. Wenn ja kille Track.
  if (info == G4Neutron::NeutronDefinition()) {
    G4cout << "Debug: Teilchen ist Neutron mit Gen1NeutronID " << trackInfo->MyTrackInfo::GetGen1NeutronID() << G4endl;
    // Check if MyTrackInfo exists and if the gen1NeutronID is -1 (indicating first-generation neutron)
    if (trackInfo && trackInfo->MyTrackInfo::GetGen1NeutronID() == -1) {
        // Set the gen1NeutronID to the current neutron's TrackID (this is the first-generation neutron)
        trackInfo->MyTrackInfo::SetGen1NeutronID(aTrack->GetTrackID());

        // Push Data
        vertexPositions.push_back(aTrack->GetVertexPosition()); // Save the locations of Neutrons creation
        vertexMomentums.push_back(aTrack->GetVertexMomentumDirection());
        globalTimes.push_back(aTrack->GetGlobalTime());
        vertexKineticEnergies.push_back(aTrack->GetVertexKineticEnergy());
        // In welchem Volumen erzeugt? Nicht als string ausgeben sondern als int 
        const G4VPhysicalVolume* physicalVolume = aTrack->GetVolume();
        if (physicalVolume) {
        // Volumenname und Materialname ermitteln
          G4int G4PhysVolumeID = MyRunMappingAction::GetPhysVolumeMappingID(physicalVolume->GetName());
          G4cout << "G4PhysVolumeID: " << G4PhysVolumeID << G4endl; 
          physicalVolumes.push_back(G4PhysVolumeID);
          G4Material* material = physicalVolume->GetLogicalVolume()->GetMaterial();
          if (material) {
            G4int G4MaterialID = MyRunMappingAction::GetMaterialMappingID(material->GetName());
            G4cout << "G4MaterialID : " << G4MaterialID << G4endl;
            physicalVolumes.push_back(G4PhysVolumeID);
            materials.push_back(G4MaterialID);
          }
          else {
            materials.push_back(-1);  
          }
        }
        else {   
          physicalVolumes.push_back(-1);     
          materials.push_back(-1);     
      }
    }
  }
}

// invoked in RMGRunAction::SetupAnalysisManager()
void NeutronsOutputScheme::AssignOutputNames(G4AnalysisManager* ana_man) {  
  G4cout << "Debug: AssignOutputNames" << G4endl;

  auto rmg_man = RMGManager::Instance();
  auto id = rmg_man->RegisterNtuple(OutputRegisterID,
      ana_man->CreateNtuple("NeutronsOutput", "Event data"));

  ana_man->CreateNtupleIColumn(id, "evtid");
  // Create column structure to safe data
  ana_man->CreateNtupleDColumn(id, "x_position_in_m");
  ana_man->CreateNtupleDColumn(id, "y_position_in_m");
  ana_man->CreateNtupleDColumn(id, "z_position_in_m");
  ana_man->CreateNtupleDColumn(id, "x_momentum_in_m_s");
  ana_man->CreateNtupleDColumn(id, "y_momentum_in_m_s");
  ana_man->CreateNtupleDColumn(id, "z_momentum_in_m_s");
  ana_man->CreateNtupleDColumn(id, "global_time");
  ana_man->CreateNtupleDColumn(id, "kinetic_energy_in_keV");
  ana_man->CreateNtupleIColumn(id, "physical_volume_id_of_N_creation");
  ana_man->CreateNtupleIColumn(id, "material_of_N_creation");
  ana_man->CreateNtupleIColumn(id, "Ge77_produced_in_muon_event");
}

void NeutronsOutputScheme::StoreEvent(const G4Event* event) {
  // Wurde Ge77 Flagge gesetzt?
  auto info = event->GetUserInformation();
  if (info != nullptr && dynamic_cast<MyGe77EventInformation*>(info) != nullptr) {
      G4cout << "Ge77Flag wird auf true gesetzt" << G4endl;
      fGe77Produced = true;
  }

  auto rmg_man = RMGManager::Instance();
  if (rmg_man->IsPersistencyEnabled()) { 
    RMGLog::OutDev(RMGLog::debug, "Filling persistent data vectors");
    const auto ana_man = G4AnalysisManager::Instance();
    auto ntupleid = rmg_man->GetNtupleID(OutputRegisterID);
    if (ntupleid < 0) {
        G4cerr << "❌ ERROR: Invalid Ntuple ID! Data will not be saved." << G4endl;
        // return;
    }   

    for (size_t i = 0; i < globalTimes.size(); ++i) {
      int col_id = 0;
      // Output: Was Ge77 produced in this event?
      ana_man->FillNtupleIColumn(ntupleid, col_id++, event->GetEventID());
      ana_man->FillNtupleDColumn(ntupleid, col_id++, vertexPositions[i].getX()/u::m);
      ana_man->FillNtupleDColumn(ntupleid, col_id++, vertexPositions[i].getY()/u::m);
      ana_man->FillNtupleDColumn(ntupleid, col_id++, vertexPositions[i].getZ()/u::m);  // Standard Einheit ist mm, bei Definition mit Einheit in m *u::m -> mal 1000, bei Abfrage des Wertes in m /u::m -> geteilt durch 1000 für Rückrechnung
      // Füge hinzu : Kinetische Energie, Zeitpunkt der Entstehung (global, also seitdem das Muon erzeugt wurde), Impulsvektor (x, y, z), kinetische Energie
      ana_man->FillNtupleDColumn(ntupleid, col_id++, vertexMomentums[i].getX());
      ana_man->FillNtupleDColumn(ntupleid, col_id++, vertexMomentums[i].getY());
      ana_man->FillNtupleDColumn(ntupleid, col_id++, vertexMomentums[i].getZ()); 
      ana_man->FillNtupleDColumn(ntupleid, col_id++, globalTimes[i]);
      ana_man->FillNtupleDColumn(ntupleid, col_id++, vertexKineticEnergies[i]/u::keV);
      //Volumen und Material:
      ana_man->FillNtupleIColumn(ntupleid, col_id++, physicalVolumes[i]);
      ana_man->FillNtupleIColumn(ntupleid, col_id++, materials[i]);
      // Ge77Flag
      ana_man->FillNtupleIColumn(ntupleid, col_id++, fGe77Produced);
      // Startet neue Reihe in Output
      ana_man->AddNtupleRow(ntupleid);
    }
  }
}

void NeutronsOutputScheme::DefineCommands() {
  
}

// vim: tabstop=2 shiftwidth=2 expandtab