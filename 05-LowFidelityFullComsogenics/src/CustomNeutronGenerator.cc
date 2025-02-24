#include "CustomNeutronGenerator.hh"

#include "G4Event.hh"
#include "G4ParticleTable.hh"
#include "MyPrimaryNeutronUserInfo.hh"

CustomNeutronGenerator::CustomNeutronGenerator()
    : RMGVGenerator("NeutronsDistribution") {
  this->DefineCommands();
  fGun = std::make_unique<G4ParticleGun>();
}

CustomNeutronGenerator::~CustomNeutronGenerator() {
  if (fInputFile.is_open())
    fInputFile.close();
}

void CustomNeutronGenerator::GeneratePrimaries(G4Event *event) {
    G4int currentEventID = event->GetEventID();

    G4int nEvent = 0;
    G4double energy = 0.0 * u::keV;
    G4double px, py, pz;
    G4double x = 0, y = 0, z = 0;
    G4int particleID = 0;
    G4int fGe77 = 0; 

    if (fInputFile.eof()) {
        fInputFile.close();
        G4cerr << "File over: not enough events! Debugoutput" << G4endl;
        return;
    }

    // Make sure the event gets assigned the correct line
    while (fInputFile >> nEvent >> particleID >> energy >> x >> y >> z >> px >>
            py >> pz >> fGe77) {
        if (nEvent == currentEventID) {
        // Found the correct event, now we can process it
        break;
        } else if (fInputFile.eof()) {
        // If we reach the end of the file before finding the event something went
        // wrong.
        throw std::runtime_error("Could not find the correct event in the file.");
        }
    }

    G4ParticleTable *theParticleTable = G4ParticleTable::GetParticleTable();

    fGun->SetParticleDefinition(theParticleTable->FindParticle("neutron"));

    energy = energy * u::keV;
    x = x * u::m;
    y = y * u::m;
    z = z * u::m;

    G4ThreeVector Position(x, y, z);
    G4ThreeVector momentumDir(px, py, pz);

    fGun->SetParticlePosition(Position);
    fGun->SetParticleMomentumDirection(momentumDir);
    fGun->SetParticleEnergy(energy);

    // Erstelle das Vertex und speichere die Flag als UserInfo
    G4PrimaryVertex* vertex = new G4PrimaryVertex(Position, 0);
    vertex->SetUserInformation(new MyPrimaryNeutronUserInfo(fGe77));

    fGun->GeneratePrimaryVertex(event);
    event->AddPrimaryVertex(vertex);
}

void CustomNeutronGenerator::SetNeutronFile(G4String pathToFile) {
    fInputFile.open(pathToFile, std::ifstream::in);
    if (!(fInputFile.is_open())) {
        G4cerr << "Musung file not valid! Name: " << pathToFile << G4endl;
    }
}

void CustomNeutronGenerator::DefineCommands() {

    // NOTE: SetUnit(Category) is not thread-safe

    fMessenger = std::make_unique<G4GenericMessenger>(
        this, "/Cosmogenics/Generator/",
        "Commands for controlling the Neutron µ generator");

    fMessenger->DeclareMethod("SetNeutronFile", &CustomNeutronGenerator::SetNeutronFile)
        .SetGuidance("Set the Neutron input file")
        .SetParameterName("pathToFile", false)
        .SetToBeBroadcasted(true)
        .SetStates(G4State_PreInit, G4State_Idle);
}

// vim: tabstop=2 shiftwidth=2 expandtab
