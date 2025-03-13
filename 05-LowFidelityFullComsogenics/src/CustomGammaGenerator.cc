#include "CustomGammaGenerator.hh"

#include "G4Event.hh"
#include "G4ParticleTable.hh"
#include "MyPrimaryGammaUserInfo.hh"

#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>

namespace u = CLHEP;

CustomGammaGenerator::CustomGammaGenerator()
    : RMGVGenerator("GammaDistribution") {
  this->DefineCommands();
  fGun = std::make_unique<G4ParticleGun>();
}

CustomGammaGenerator::~CustomGammaGenerator() {
  if (fInputFile.is_open())
    fInputFile.close();
}

void CustomGammaGenerator::BeginOfRunAction(const G4Run*) {
    if (!fInputFile.is_open()) {
        G4cerr << "Fehler: Gamma Daten konnten nicht gelesen werden." << G4endl;
        return;
    }

    std::string line;
    
    // Header-Zeile überspringen
    std::getline(fInputFile, line);

    while (std::getline(fInputFile, line)) {
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
        xs.push_back(x_val*u::m);
        ys.push_back(y_val*u::m);
        zs.push_back(z_val*u::m);
        pxs.push_back(px_val);
        pys.push_back(py_val);
        pzs.push_back(pz_val);
        nCTimes.push_back(nCTime_val*u::s);
        eKins.push_back(eKin_val*u::keV);
        physVolIDs.push_back(physVolID_val);
        matIDs.push_back(matID_val);
        fGe77s.push_back(fGe77_val);
    }

    fInputFile.close();
    G4cout << "Geladene Events aus CSV: " << xs.size() << G4endl;
}

void CustomGammaGenerator::GeneratePrimaries(G4Event *event) {
    gammaIndices.clear();
    nCGammaTotalEnergy = 0;
    nCGammaAmount = 0;

    // Get correct neutron captures
    G4int currentEventID = event->GetEventID();      

    G4ParticleTable *theParticleTable = G4ParticleTable::GetParticleTable();

    fGun->SetParticleDefinition(theParticleTable->FindParticle("gamma"));

    for (size_t i = 0; i < neutronIDs.size(); i++) {
        if (neutronIDs[i] == currentEventID) {
            gammaIndices.push_back(i);  // Füge Gamma hinzu, wenn es das richtige Event ist
        }
        else if (neutronIDs[i] > currentEventID) {
            break;
        }
    }

    if (!gammaIndices.empty()) {
        nCGammaAmount = gammaIndices.size();
        G4cout << "Gamma Indizes vorhanden: " << gammaIndices.size() << " Gammas" << G4endl;

        for (size_t idx : gammaIndices) {
            G4cout << "Addiere Gamma Energie" << G4endl;
            nCGammaTotalEnergy += eKins[idx];
        }
        G4cout << "Gamma totale Energie: " << nCGammaTotalEnergy << G4endl;
        // Für jedes Gamma im Event:
        for (size_t idx : gammaIndices) {
            G4cout << "Loope Gamma Daten in Particle Gun" << G4endl;
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
            event->AddPrimaryVertex(vertex);
            // Generiere das Primary-Vertex für das Gamma und füge es zum Event hinzu
            fGun->GeneratePrimaryVertex(event);
            
        }
    }
}

void CustomGammaGenerator::SetGammasFile(G4String pathToFile) {
    fInputFile.open(pathToFile, std::ifstream::in);
    if (!(fInputFile.is_open())) {
        G4cerr << "Gammas file not valid! Name: " << pathToFile << G4endl;
    }
}

void CustomGammaGenerator::DefineCommands() {

    // NOTE: SetUnit(Category) is not thread-safe

    fMessenger = std::make_unique<G4GenericMessenger>(
        this, "/Cosmogenics/Generator/",
        "Commands for controlling the Neutron µ generator");

    fMessenger->DeclareMethod("SetGammasFile", &CustomGammaGenerator::SetGammasFile)
        .SetGuidance("Set the Neutron input file")
        .SetParameterName("pathToFile", false)
        .SetToBeBroadcasted(true)
        .SetStates(G4State_PreInit, G4State_Idle);
}

// vim: tabstop=2 shiftwidth=2 expandtab
