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
#include "MyGe77EventFilterOutputScheme.hh"
#include "RMGHardware.hh"
#include "RMGLog.hh"
#include "RMGManager.hh"

namespace u = CLHEP;

NeutronsOutputScheme::NeutronsOutputScheme() { 
  this->DefineCommands(); 
}

NeutronsOutputScheme::~NeutronsOutputScheme() {};

void NeutronsOutputScheme::ClearBeforeEvent() {
  vertexPositions.clear();
  vertexMomentums.clear();
  globalTimes.clear();
  vertexKineticEnergies.clear();
  physicalVolumes.clear();
  materials.clear();
  fGe77Produced = false;

  physicalVolumeMappingIDs.clear();
  materialMappingIDs.clear();
  physicalVolumeMappingNames.clear();
  materialMappingNames.clear();

  fPhysVolumeMapping = false;
  fMaterialMapping = false;
};

// Need information of isotop creation here as well. Could also get from other IsotopeFilterOutputscheme.
void NeutronsOutputScheme::TrackingActionPre(const G4Track* aTrack) { 
  // Check if the track is a neutron
  auto* info = aTrack->GetDefinition();
  auto* trackInfo = dynamic_cast<MyTrackInfo*>(aTrack->GetUserInformation());

  // Schaue ob gen1NeutronID des Tracks in killableIDs ist. Wenn ja kille Track.
  if (info == G4Neutron::NeutronDefinition()) {
    // Check if MyTrackInfo exists and if the gen1NeutronID is -1 (indicating first-generation neutron)
    if (trackInfo && trackInfo->MyTrackInfo::GetGen1NeutronID() == -1) {
        // Set the gen1NeutronID to the current neutron's TrackID (this is the first-generation neutron)
        trackInfo->MyTrackInfo::SetGen1NeutronID(aTrack->GetTrackID());

        // Push Data
        vertexPositions.push_back(aTrack->GetVertexPosition()); // Save the locations of Neutrons creation
        vertexMomentums.push_back(aTrack->GetMomentumDirection());
        globalTimes.push_back(aTrack->GetGlobalTime());
        vertexKineticEnergies.push_back(aTrack->GetVertexKineticEnergy());
        // In welchem Volumen erzeugt? Nicht als string ausgeben sondern als int 
        const G4VPhysicalVolume* physicalVolume = aTrack->GetVolume();
        int physVolumeID = -1;
        int materialID = -1;
        if (physicalVolume) {
        // Volumenname und Materialname ermitteln
          std::string physVolumeName = physicalVolume->GetName();
          
          if (physVolumeMapping.find(physVolumeName) == physVolumeMapping.end()) {
            physVolumeMapping.emplace(physVolumeName, physVolumeMapping.size());
            physicalVolumeMappingNames.push_back(physVolumeName);
            physicalVolumeMappingIDs.push_back(physVolumeMapping[physVolumeName]);
            fPhysVolumeMapping = true;
          }
          physVolumeID = physVolumeMapping[physVolumeName];
          
          G4Material* material = physicalVolume->GetLogicalVolume()->GetMaterial();
          if (material) {
            std::string materialName = material->GetName();
          
            if (materialMapping.find(materialName) == materialMapping.end()) {
              materialMapping.emplace(materialName, materialMapping.size());
              materialMappingNames.push_back(materialName);
              materialMappingIDs.push_back(materialMapping[materialName]);
              fMaterialMapping = true;
            }
            materialID = materialMapping[materialName];
          }
        }
        physicalVolumes.push_back(physVolumeID);
        materials.push_back(materialID);
    }
  }
}

// invoked in RMGRunAction::SetupAnalysisManager()
void NeutronsOutputScheme::AssignOutputNames(G4AnalysisManager* ana_man) {  
  G4cout << "Debug: AssignOutputNames" << G4endl;

  auto rmg_man = RMGManager::Instance();
  auto neutronsNTuple = rmg_man->RegisterNtuple(neutronsRegister,
      ana_man->CreateNtuple("NeutronsOutput", "Event data"));

  ana_man->CreateNtupleIColumn(neutronsNTuple, "evtid");
  // Create column structure to safe data
  ana_man->CreateNtupleDColumn(neutronsNTuple, "x_position_in_m");
  ana_man->CreateNtupleDColumn(neutronsNTuple, "y_position_in_m");
  ana_man->CreateNtupleDColumn(neutronsNTuple, "z_position_in_m");
  ana_man->CreateNtupleDColumn(neutronsNTuple, "x_momentum_direction");
  ana_man->CreateNtupleDColumn(neutronsNTuple, "y_momentum_direction");
  ana_man->CreateNtupleDColumn(neutronsNTuple, "z_momentum_direction");
  ana_man->CreateNtupleDColumn(neutronsNTuple, "global_time");
  ana_man->CreateNtupleDColumn(neutronsNTuple, "kinetic_energy_in_keV");
  ana_man->CreateNtupleIColumn(neutronsNTuple, "physical_volume_id_of_N_creation");
  ana_man->CreateNtupleIColumn(neutronsNTuple, "material_id_of_N_creation");
  ana_man->CreateNtupleIColumn(neutronsNTuple, "Ge77_produced_in_muon_event");

  auto physVol = rmg_man->RegisterNtuple(physVolRegister,
    ana_man->CreateNtuple("physVolumes", "physVolumes name mapping"));
ana_man->CreateNtupleIColumn(physVol, "physVolumesID");
ana_man->CreateNtupleSColumn(physVol, "physVolumeNames");
ana_man->FinishNtuple(physVol);

auto materials = rmg_man->RegisterNtuple(materialRegister,
    ana_man->CreateNtuple("materials", "materials name mapping"));
ana_man->CreateNtupleIColumn(materials, "materialsID");
ana_man->CreateNtupleSColumn(materials, "materialNames");
ana_man->FinishNtuple(materials);
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
    auto neutronsNTuple = rmg_man->GetNtupleID(neutronsRegister); 

    for (size_t i = 0; i < globalTimes.size(); ++i) {
      int col_id = 0;
      // Output: Was Ge77 produced in this event?
      ana_man->FillNtupleIColumn(neutronsNTuple, col_id++, event->GetEventID());
      ana_man->FillNtupleDColumn(neutronsNTuple, col_id++, vertexPositions[i].getX()/u::m);
      ana_man->FillNtupleDColumn(neutronsNTuple, col_id++, vertexPositions[i].getY()/u::m);
      ana_man->FillNtupleDColumn(neutronsNTuple, col_id++, vertexPositions[i].getZ()/u::m);  // Standard Einheit ist mm, bei Definition mit Einheit in m *u::m -> mal 1000, bei Abfrage des Wertes in m /u::m -> geteilt durch 1000 für Rückrechnung
      // Füge hinzu : Kinetische Energie, Zeitpunkt der Entstehung (global, also seitdem das Muon erzeugt wurde), Impulsvektor (x, y, z), kinetische Energie
      ana_man->FillNtupleDColumn(neutronsNTuple, col_id++, vertexMomentums[i].getX());
      ana_man->FillNtupleDColumn(neutronsNTuple, col_id++, vertexMomentums[i].getY());
      ana_man->FillNtupleDColumn(neutronsNTuple, col_id++, vertexMomentums[i].getZ()); 
      ana_man->FillNtupleDColumn(neutronsNTuple, col_id++, globalTimes[i]);
      ana_man->FillNtupleDColumn(neutronsNTuple, col_id++, vertexKineticEnergies[i]/u::keV);
      //Volumen und Material:
      ana_man->FillNtupleIColumn(neutronsNTuple, col_id++, physicalVolumes[i]);
      ana_man->FillNtupleIColumn(neutronsNTuple, col_id++, materials[i]);
      // Ge77Flag
      ana_man->FillNtupleIColumn(neutronsNTuple, col_id++, fGe77Produced);
      // Startet neue Reihe in Output
      ana_man->AddNtupleRow(neutronsNTuple);
    }

    if (fPhysVolumeMapping) {
      auto physVolumesNTuple = rmg_man->GetNtupleID(physVolRegister);
      for (size_t i = 0; i < physicalVolumeMappingIDs.size(); ++i) {
        int col_id = 0;
        ana_man->FillNtupleIColumn(physVolumesNTuple, col_id++, physicalVolumeMappingIDs[i]);
        ana_man->FillNtupleSColumn(physVolumesNTuple, col_id++, physicalVolumeMappingNames[i]);
        ana_man->AddNtupleRow(physVolumesNTuple);
      }
    }

    if (fMaterialMapping) {
      auto materialsNTuple = rmg_man->GetNtupleID(materialRegister);
      for (size_t i = 0; i < physicalVolumeMappingIDs.size(); ++i) {
        int col_id = 0;
        ana_man->FillNtupleIColumn(materialsNTuple, col_id++, materialMappingIDs[i]);
        ana_man->FillNtupleSColumn(materialsNTuple, col_id++, materialMappingNames[i]);
        ana_man->AddNtupleRow(materialsNTuple);
      }
    }
  }
}

void NeutronsOutputScheme::DefineCommands() {
  
}

// vim: tabstop=2 shiftwidth=2 expandtab