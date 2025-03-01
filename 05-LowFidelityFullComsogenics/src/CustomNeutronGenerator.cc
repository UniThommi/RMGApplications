#include "CustomNeutronGenerator.hh"

#include "G4Event.hh"
#include "G4ParticleTable.hh"
#include "MyPrimaryNeutronUserInfo.hh"

#include "H5Cpp.h" // Für hdf5 file
#include <iostream>
#include <vector>

#ifndef H5_NO_NAMESPACE
using namespace H5;
#endif

CustomNeutronGenerator::CustomNeutronGenerator()
    : RMGVGenerator("NeutronsDistribution") {
  this->DefineCommands();
  fGun = std::make_unique<G4ParticleGun>();
}

CustomNeutronGenerator::~CustomNeutronGenerator() {
  if (fInputFile.is_open())
    fInputFile.close();
}

void CustomNeutronGenerator::BeginOfRunAction(const G4Run*) {
    
    try {
        H5File file(fInputFile, H5F_ACC_RDONLY);

        // Datasets aus HDF5-Datei lesen
        evtid = ReadDataset(file, "hit/NeutronsOutput/evtid/pages");
        x = ReadDataset(file, "hit/NeutronsOutput/x_position_in_m/pages");
        y = ReadDataset(file, "hit/NeutronsOutput/y_position_in_m/pages");
        z = ReadDataset(file, "hit/NeutronsOutput/z_position_in_m/pages");
        px = ReadDataset(file, "hit/NeutronsOutput/x_momentum_in_m_s/pages");
        py = ReadDataset(file, "hit/NeutronsOutput/y_momentum_in_m_s/pages");
        pz = ReadDataset(file, "hit/NeutronsOutput/z_momentum_in_m_s/pages");
        eKin = ReadDataset(file, "hit/NeutronsOutput/kinetic_energy_in_keV/pages");
        physVolID = ReadDataset(file, "hit/NeutronsOutput/physical_volume_id_of_N_creation/pages");
        matID = ReadDataset(file, "hit/NeutronsOutput/material_id_of_N_creation/pages");
        fGe77 = ReadDataset(file, "hit/NeutronsOutput/fGe77/pages");

        // Konsistenz prüfen
        size_t N = x.size();
        if (y.size() != N || z.size() != N || px.size() != N || py.size() != N || pz.size() != N || eKin.size() != N || fGe77.size() != N) {
            std::cerr << "Fehler: Inkonsistente Datengrößen in HDF5!" << std::endl;
            return;
        }

        G4cout << "Geladene Events aus HDF5: " << data.size() << G4endl;

    } catch (H5::Exception& error) {
        error.printErrorStack();
    }
}

void CustomNeutronGenerator::GeneratePrimaries(G4Event *event) {
    G4int currentEventID = event->GetEventID();      

    int index = currentEventID;

    G4ParticleTable *theParticleTable = G4ParticleTable::GetParticleTable();

    fGun->SetParticleDefinition(theParticleTable->FindParticle("neutron"));

    G4ThreeVector Position(x[index], y[index], z[index]);
    G4ThreeVector momentumDir(px[index], py[index], pz[index]);

    fGun->SetParticlePosition(Position);
    fGun->SetParticleMomentumDirection(momentumDir);
    fGun->SetParticleEnergy(eKin[index]);
    fGun->SetParticleTime(time[index]) // FIX in vectoren

    // Erstelle das Vertex und speichere die Flag als UserInfo
    G4PrimaryVertex* vertex = new G4PrimaryVertex(Position, 0);
    vertex->SetUserInformation(new MyPrimaryNeutronUserInfo(physVolID[index], matID[index], fGe77));

    fGun->GeneratePrimaryVertex(event);
    event->AddPrimaryVertex(vertex);
}

void CustomNeutronGenerator::SetNeutronsFile(G4String pathToFile) {
    fInputFile.open(pathToFile, std::ifstream::in);
    if (!(fInputFile.is_open())) {
        G4cerr << "Neutrons file not valid! Name: " << pathToFile << G4endl;
    }
}

void CustomNeutronGenerator::DefineCommands() {

    // NOTE: SetUnit(Category) is not thread-safe

    fMessenger = std::make_unique<G4GenericMessenger>(
        this, "/Cosmogenics/Generator/",
        "Commands for controlling the Neutron µ generator");

    fMessenger->DeclareMethod("SetNeutronsFile", &CustomNeutronGenerator::SetNeutronsFile)
        .SetGuidance("Set the Neutron input file")
        .SetParameterName("pathToFile", false)
        .SetToBeBroadcasted(true)
        .SetStates(G4State_PreInit, G4State_Idle);
}

// vim: tabstop=2 shiftwidth=2 expandtab
