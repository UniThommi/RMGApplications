#include "MyTrackInfo.hh"
#include "RMGLog.hh"
#include "RMGHardware.hh"
#include "G4ParticleDefinition.hh"
#include "G4ParticleTypes.hh"
#include "G4VProcess.hh"
#include "G4RunManager.hh"


// Constructor initializes track IDs
MyTrackInfo::MyTrackInfo(G4int evtID, G4int trkID) 
    : eventID(evtID), trackID(trkID) {}

// Destruktor
MyTrackInfo::~MyTrackInfo() {}


G4int MyTrackInfo::GetEventID() const { return eventID; }
G4int MyTrackInfo::GetTrackID() const { return trackID; }

// Getter-Methoden
std::vector<G4String> MyTrackInfo::GetnCPhysVolume() const {
    return nCPhysVolume;
}

std::vector<G4String> MyTrackInfo::GetnCMaterial() const {
    return nCMaterial;
}

std::vector<G4double> MyTrackInfo::GetnCTime() const {
    return nCTime;
}

std::vector<G4int> MyTrackInfo::GetfGe77() const {
    return fGe77;
}


std::vector<G4ThreeVector> MyTrackInfo::GetGammaPosition() const {
    return gammaPosition;
}

std::vector<G4ThreeVector> MyTrackInfo::GetGammaMomentumDirection() const {
    return gammaMomentumDirection;
}

std::vector<G4double> MyTrackInfo::GetGammaKinEnergy() const {
    return gammaKinEnergy;
}


// Vererbung der Primary-ID und 1. Gen Neutron-ID. Für Muon Track wird 1. Gen Neutron-ID auf -1 gesetzt.
// Speichere zudem Gamma Daten von Neutron Captures.
void MySteppingAction::UserSteppingAction(const G4Step* step) {
     // Prüfe, ob das Prozessende ein Neutroneneinfang (nCapture) ist
    G4StepPoint* postStepPoint = step->GetPostStepPoint();
    const G4Track* track = step->GetTrack();

    if (postStepPoint->GetProcessDefinedStep()->GetProcessName() == "nCapture") {
        // Sicherstellen, dass das eingefangene Teilchen ein Neutron ist
        if (track->GetParticleDefinition() == G4Neutron::Definition()) {
            G4cout << "Neutron capture detected" << G4endl;

            G4TouchableHandle touchable = postStepPoint->GetTouchableHandle();
            G4VPhysicalVolume* physVol = touchable->GetVolume();
            G4Material* material = physVol->GetLogicalVolume()->GetMaterial();

            // Holen der sekundären Teilchen (entstandene Gammas)
            const std::vector<const G4Track*>* secondaries = step->GetSecondaryInCurrentStep();
            int prod_Ge77 = 0;
            for (const auto& secTrack : *secondaries) {
                const auto particle = secTrack->GetParticleDefinition();
                if (particle->IsGeneralIon()) {
                    int z = particle->GetAtomicNumber();
                    int a = particle->GetAtomicMass();
                    if (z == 32 && a == 77) { //Ge77?
                        prod_Ge77 = 1;
                        G4cout << "Ge-77 erzeugt! 🎉" << G4endl;
                    }
                }
            }

            // Hole oder erstelle MyTrackInfo für den aktuellen Track
            MyTrackInfo* trackInfo = dynamic_cast<MyTrackInfo*>(track->GetUserInformation());

            if (!trackInfo) {
                // Falls keine Instanz existiert, erstelle eine neue
                G4int eventID = G4RunManager::GetRunManager()->GetCurrentEvent()->GetEventID();
                G4int trackID = track->GetTrackID();
                trackInfo = new MyTrackInfo(eventID, trackID);
                const_cast<G4Track*>(track)->SetUserInformation(trackInfo);
            }

            for (const auto& secTrack : *secondaries) {
                if (secTrack->GetParticleDefinition() == G4Gamma::Definition()) {
                    trackInfo->GetnCPhysVolume().push_back(physVol->GetName());
                    trackInfo->GetnCMaterial().push_back(material->GetName());
                    trackInfo->GetnCTime().push_back(postStepPoint->GetGlobalTime());
                    trackInfo->GetfGe77().push_back(prod_Ge77);

                    trackInfo->GetGammaPosition().push_back(secTrack->GetPosition());
                    trackInfo->GetGammaMomentumDirection().push_back(secTrack->GetMomentumDirection());
                    trackInfo->GetGammaKinEnergy().push_back(secTrack->GetKineticEnergy());
                }
            }
        }
    }   
}

// vim: tabstop=2 shiftwIDth=2 expandtab