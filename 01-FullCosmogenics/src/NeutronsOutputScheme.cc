#include "NeutronsOutputScheme.hh"

#include <set>
#include <map>
#include <fstream>
#include <algorithm>

#include "G4AnalysisManager.hh"
#include "G4Event.hh"
#include "G4EventManager.hh"
#include "G4HCtable.hh"
#include "G4OpticalPhoton.hh"
#include "G4SDManager.hh"
#include "G4Neutron.hh"

#include "MyTrackInfo.hh"
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
  volumes.clear();
  materials.clear();
  primaryTrackId.clear();
  gen1NeutronID.clear();
  fGe77Produced.clear();
};

void NeutronsOutputScheme::InsertGe77Info(const G4int NeutronID) {
  // Hole die gen1NeutronID des aktuellen Tracks
  auto* trackInfo = dynamic_cast<MyTrackInfo*>(aTrack->GetUserInformation());
  if (!trackInfo) return;

  int currentGen1ID = trackInfo->GetGen1NeutronID();
  if (currentGen1ID == -1) return;  // Kein gültiger gen1NeutronID vorhanden

  // Suche nach gleichen gen1NeutronIDs in gespeicherten Daten und ändere für die Einträge die fGe77Produced auf true.
  for (size_t i = 0; i < gen1NeutronID.size(); ++i) {
    if (gen1NeutronID[i] == currentGen1ID) {
      fGe77Produced[i] = true;
    }
  }
}

// Need information of isotop creation here as well. Could also get from other IsotopeFilterOutputscheme.
void NeutronsOutputScheme::TrackingActionPre(const G4Track* aTrack) {  
 // Check if the track is a neutron
  auto* info = aTrack->GetDefinition();
  auto* trackInfo = dynamic_cast<MyTrackInfo*>(aTrack->GetUserInformation());

  // Schaue ob gen1NeutronID des Tracks in killableIDs ist. Wenn ja kille Track.
  if (trackInfo && std::find(killableIDs.begin(), killableIDs.end(), trackInfo->GetGen1NeutronID()) != killableIDs.end()) {
    const_cast<G4Track*>(aTrack)->SetTrackStatus(fStopAndKill);
  }
  if (info == G4Neutron::NeutronDefinition()) {
    // Check if MyTrackInfo exists and if the gen1NeutronID is -1 (indicating first-generation neutron)
    if (trackInfo && trackInfo->GetGen1NeutronID() == -1) {
        // Set the gen1NeutronID to the current neutron's TrackID (this is the first-generation neutron)
        trackInfo->SetGen1NeutronID(aTrack->GetTrackID());

        // Push Data
        vertexPositions.push_back(aTrack->GetVertexPosition()); // Save the locations of Neutrons creation
        vertexMomentums.push_back(aTrack->GetVertexMomentumDirection());
        globalTimes.push_back(aTrack->GetGlobalTime());
        vertexKineticEnergies.push_back(aTrack->GetVertexKineticEnergy());
        // In welchem Volumen erzeugt? Nicht als string ausgeben sondern als int
        const G4VPhysicalVolume* volume = aTrack->GetVolume();
        if (volume) {
        // Volumenname und Materialname ermitteln
          volumes.push_back(volume->GetVolumeID());
          G4Material* material = volume->GetLogicalVolume()->GetMaterial();
          if (material) {
            materials.push_back(material->GetName());
          }
          else {
            materials.push_back('Unknown');  
          }
        }
        else {
          volumes.push_back(-1);     
        }

        // Get primaryTrackID and gen1NeutronID from MyTrackInfo
        primaryTrackId.push_back(trackInfo->GetPrimaryID());
        gen1NeutronID.push_back(trackInfo->GetGen1NeutronID());

        // Setze Flag für Ge77 Production erstmal auf False. Später bearbeiten.
        fGe77Produced.push_back(false);
    }
  }
  if (info->GetAtomicMass() == 77 && info-> GetAtomicNumber() == 32) {
    if (trackInfo && trackInfo->GetGen1NeutronID() != -1) {
      InsertGe77Info(aTrack);
      // Kille den Track, da nicht mehr Info benötigt wird nach Ge77 Produktion.
      killableIDs.push_back(trackInfo->GetGen1NeutronID());
      const_cast<G4Track*>(aTrack)->SetTrackStatus(fStopAndKill);
    }
  } 
}

// invoked in RMGRunAction::SetupAnalysisManager()
void NeutronsOutputScheme::AssignOutputNames(G4AnalysisManager* ana_man) {  

  auto rmg_man = RMGManager::Instance();

  auto id = rmg_man->RegisterNtuple(OutputRegisterID,
      ana_man->CreateNtuple("NeutronsOutput", "Event data"));

  ana_man->CreateNtupleIColumn(id, "evtid");
  // Create column structure to safe data
  ana_man->CreateNtupleDColumn(id, "x_position_in_m");
  ana_man->CreateNtupleDColumn(id, "y_position_in_m");
  ana_man->CreateNtupleDColumn(id, "z_position_in_m");
  ana_man->CreateNtupleDColumn(id, "x_momentum_in_m/s");
  ana_man->CreateNtupleDColumn(id, "y_momentum_in_m/s");
  ana_man->CreateNtupleDColumn(id, "z_momentum_in_m/s");
  ana_man->CreateNtupleDColumn(id, "global_time");
  ana_man->CreateNtupleDColumn(id, "kinetic_energy_in_keV");
  ana_man->CreateNtupleIColumn(id, "volume_id_of_N_creation");
  ana_man->CreateNtupleSColumn(id, "material_of_N_creation");
  ana_man->CreateNtupleIColumn(id, "track_ID_of_Muon");
  ana_man->CreateNtupleIColumn(id, "track_ID_of_Gen1_N");
  ana_man->CreateNtupleBColumn(id, "Ge77_creation_from_N");
}

// invoked in RMGEventAction::EndOfEventAction()
void NeutronsOutputScheme::StoreEvent(const G4Event* event) {  // Speichert events in G4AnalysisManager //FIX!!!

  auto rmg_man = RMGManager::Instance();
  if (rmg_man->IsPersistencyEnabled()) { 
    RMGLog::OutDev(RMGLog::debug, "Filling persistent data vectors");
    const auto ana_man = G4AnalysisManager::Instance();
    auto ntupleid = rmg_man->GetNtupleID(OutputRegisterID);

    for (size_t i = 0; i < globalTimes.size(); ++i) {
      int col_id = 0;
      // Output: Was Ge77 produced in this event?
      ana_man->FillNtupleIColumn(ntupleid, col_id++, event->GetEventID());
      ana_man->FillNtupleDColumn(ntupleid, col_id++, vertexPositions[i].getX())/u::m;
      ana_man->FillNtupleDColumn(ntupleid, col_id++, vertexPositions[i].getY())/u::m;
      ana_man->FillNtupleDColumn(ntupleid, col_id++, vertexPositions[i].getZ())/u::m;  // Standard Einheit ist mm, bei Definition mit Einheit in m *u::m -> mal 1000, bei Abfrage des Wertes in m /u::m -> geteilt durch 1000 für Rückrechnung
      // Füge hinzu : Kinetische Energie, Zeitpunkt der Entstehung (global, also seitdem das Muon erzeugt wurde), Impulsvektor (x, y, z), kinetische Energie
      ana_man->FillNtupleDColumn(ntupleid, col_id++, vertexMomentums[i].getX())/(u::m / u::s);
      ana_man->FillNtupleDColumn(ntupleid, col_id++, vertexMomentums[i].getY())/(u::m / u::s);
      ana_man->FillNtupleDColumn(ntupleid, col_id++, vertexMomentums[i].getZ())/(u::m / u::s); 
      ana_man->FillNtupleDColumn(ntupleid, col_id++, globalTimes[i]);
      ana_man->FillNtupleDColumn(ntupleid, col_id++, vertexKineticEnergies[i])/u::keV;
      //Volumen und Material:
      ana_man->FillNtupleIColumn(ntupleid, col_id++, volumes[i]);
      ana_man->FillNtupleSColumn(ntupleid, col_id++, materials[i]);
      // IDs und Ge77 Flag
      ana_man->FillNtupleIColumn(ntupleid, col_id++, primaryTrackId[i]);
      ana_man->FillNtupleIColumn(ntupleid, col_id++, gen1NeutronID[i]);
      ana_man->FillNtupleBColumn(ntupleid, col_id++, fGe77Produced[i]);
      // Startet neue Reihe in Output
      ana_man->AddNtupleRow(ntupleid);
    }
  }
}

void NeutronsOutputScheme::DefineCommands() {

}

// vim: tabstop=2 shiftwidth=2 expandtab