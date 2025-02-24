#include "CustomMUSUNGenerator.hh"

#include "G4Event.hh"
#include "G4ParticleTable.hh"

CustomMUSUNGenerator::CustomMUSUNGenerator()
    : RMGVGenerator("MUSUNCosmicMuons") {
  this->DefineCommands();
  fGun = std::make_unique<G4ParticleGun>();
}

CustomMUSUNGenerator::~CustomMUSUNGenerator() {
  if (fInputFile.is_open())
    fInputFile.close();
}

void CustomMUSUNGenerator::GeneratePrimaries(G4Event *event) {
  G4int currentEventID = event->GetEventID();

  G4int nEvent = 0;
  G4double energy = 0.0 * u::MeV;
  G4double px, py, pz;
  G4double x = 0, y = 0, z = 0;
  G4int particleID = 0;

  if (fInputFile.eof()) {
    fInputFile.close();
    G4cerr << "File over: not enough events! Debugoutput" << G4endl;
    return;
  }

  // Make sure the event gets assigned the correct line
  while (fInputFile >> nEvent >> particleID >> energy >> x >> y >> z >> px >>
         py >> pz) {
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

  if (particleID == 10)
    fGun->SetParticleDefinition(theParticleTable->FindParticle("mu-"));
  else
    fGun->SetParticleDefinition(theParticleTable->FindParticle("mu+"));

  energy = energy * u::GeV;
  x = x * u::cm;
  y = y * u::cm;
  z = z * u::cm;

  G4ThreeVector Position(x, y, z);
  G4ThreeVector momentumDir(px, py, pz);

  fGun->SetParticlePosition(Position);
  fGun->SetParticleMomentumDirection(momentumDir);
  fGun->SetParticleEnergy(energy);

  fGun->GeneratePrimaryVertex(event);
}

void CustomMUSUNGenerator::SetMUSUNFile(G4String pathToFile) {
  fInputFile.open(pathToFile, std::ifstream::in);
  if (!(fInputFile.is_open())) {
    G4cerr << "Musung file not valid! Name: " << pathToFile << G4endl;
  }
}

void CustomMUSUNGenerator::DefineCommands() {

  // NOTE: SetUnit(Category) is not thread-safe

  fMessenger = std::make_unique<G4GenericMessenger>(
      this, "/Cosmogenics/Generator/",
      "Commands for controlling the MUSUN µ generator");

  fMessenger->DeclareMethod("SetMUSUNFile", &CustomMUSUNGenerator::SetMUSUNFile)
      .SetGuidance("Set the MUSUN input file")
      .SetParameterName("pathToFile", false)
      .SetToBeBroadcasted(true)
      .SetStates(G4State_PreInit, G4State_Idle);
}

// vim: tabstop=2 shiftwidth=2 expandtab
