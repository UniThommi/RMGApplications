#include "CustomGammaGenerator.hh"

#include "G4Event.hh"
#include "G4ParticleTable.hh"
#include "MyPrimaryNeutronUserInfo.hh"

#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>

CustomGammaGenerator::CustomGammaGenerator()
    : RMGVGenerator("NeutronsDistribution") {
  this->DefineCommands();
  fGun = std::make_unique<G4ParticleGun>();
}

CustomGammaGenerator::~CustomGammaGenerator() {
  if (fInputFile.is_open())
    fInputFile.close();
}

void CustomGammaGenerator::BeginOfRunAction(const G4Run*) {
    
    std::ifstream file(fInputFile);
    if (!file.is_open()) {
        std::cerr << "Fehler: Konnte die Datei nicht öffnen!" << std::endl;
        return;
    }

    std::string line;
    
    // Header-Zeile überspringen
    std::getline(file, line);

    while (std::getline(file, line)) {
        std::stringstream ss(line);
        G4double x_val, y_val, z_val, px_val, py_val, pz_val, nCTime_val, eKin_val;
        G4int muonID_val, neutronID_val, physVolID_val, matID_val, fGe77_val;

        // CSV-Werte direkt einlesen (Semikolon als Trennzeichen)
        char delimiter;
        ss >> muonID_val >> delimiter
           >> neutronID_val >> delimiter
           >> x_val >> delimiter
           >> y_val >> delimiter
           >> z_val >> delimiter
           >> px_val >> delimiter
           >> py_val >> delimiter
           >> pz_val >> delimiter
           >> nCTime_val >> delimiter
           >> eKin_val >> delimiter
           >> physVolID_val >> delimiter
           >> matID_val >> delimiter
           >> fGe77_val;

        // Falls ein Lese-Fehler auftritt, überspringen
        if (ss.fail()) continue;

        // Werte speichern
        muonIDs.push_back(muonID_val);
        neutronIDs.push_back(neutronID_val);
        xs.push_back(x_val);
        ys.push_back(y_val);
        zs.push_back(z_val);
        pxs.push_back(px_val);
        pys.push_back(py_val);
        pzs.push_back(pz_val);
        nCTimes.push_back(nCTime_val);
        eKins.push_back(eKin_val);
        physVolIDs.push_back(physVolID_val);
        matIDs.push_back(matID_val);
        fGe77s.push_back(fGe77_val);
    }

    file.close();
    G4cout << "Geladene Events aus CSV: " << x.size() << G4endl;
}

void CustomGammaGenerator::GeneratePrimaries(G4Event *event) {
    G4int currentEventID = event->GetEventID();      

    G4ParticleTable *theParticleTable = G4ParticleTable::GetParticleTable();

    fGun->SetParticleDefinition(theParticleTable->FindParticle("gamma"));

    std::vector<size_t> gammaIndices;

    for (size_t i = 0; i < neutronIDs.size(); i++) {
        if (neutronIDs[i] == currentEventID) {
            gammaIndices.push_back(i);  // Füge Gamma hinzu, wenn es das richtige Event ist
        }
    }

    if (!gammaIndices.empty()) {
            G4int nCGammaAmount = gammaIndices.size();
            G4int nCGammaTotalEnergy = 0;

        for (size_t idx : gammaIndices) {
            nCGammaTotalEnergy += eKins[idx];
        }

        // Für jedes Gamma im Event:
        for (size_t idx : gammaIndices) {
            G4ThreeVector Position(xs[idx], ys[idx], zs[idx]);  // Position des Gammas
            G4ThreeVector momentumDir(pxs[idx], pys[idx], pzs[idx]);  // Richtung des Gammas
            G4double energy = eKins[idx];  // Energie des Gammas
            G4double time = nCTimes[idx];  // Zeit des Gammas

            // Erstelle einen neuen Primary Vertex für jedes Gamma
            G4PrimaryVertex* vertex = new G4PrimaryVertex(Position, time);

            // Setze die Eigenschaften für das Gamma
            fGun->SetParticlePosition(Position);
            fGun->SetParticleMomentumDirection(momentumDir);
            fGun->SetParticleEnergy(energy);
            fGun->SetParticleTime(time);

            // Füge das Primary Vertex für dieses Gamma hinzu
            vertex->SetUserInformation(new MyPrimaryGammaUserInfo(muonIDs[idx], physVolIDs[idx], matIDs[idx], fGe77s[idx], nCGammaAmount, nCGammaTotalEnergy));

            // Generiere das Primary-Vertex für das Gamma und füge es zum Event hinzu
            fGun->GeneratePrimaryVertex(event);
            event->AddPrimaryVertex(vertex);
        }
    }
}

void CustomGammaGenerator::SetNeutronsFile(G4String pathToFile) {
    fInputFile.open(pathToFile, std::ifstream::in);
    if (!(fInputFile.is_open())) {
        G4cerr << "Neutrons file not valid! Name: " << pathToFile << G4endl;
    }
}

void CustomGammaGenerator::DefineCommands() {

    // NOTE: SetUnit(Category) is not thread-safe

    fMessenger = std::make_unique<G4GenericMessenger>(
        this, "/Cosmogenics/Generator/",
        "Commands for controlling the Neutron µ generator");

    fMessenger->DeclareMethod("SetNeutronsFile", &CustomGammaGenerator::SetNeutronsFile)
        .SetGuidance("Set the Neutron input file")
        .SetParameterName("pathToFile", false)
        .SetToBeBroadcasted(true)
        .SetStates(G4State_PreInit, G4State_Idle);
}

// vim: tabstop=2 shiftwidth=2 expandtab
