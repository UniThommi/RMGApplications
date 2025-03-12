#include "MyTrackInfo.hh"
#include "RMGLog.hh"
#include "RMGHardware.hh"
#include "G4ParticleDefinition.hh"
#include "G4ParticleTypes.hh"
#include "G4VProcess.hh"
#include "G4RunManager.hh"
#include "G4Neutron.hh"

// Constructor initializes track IDs
MyTrackInfo::MyTrackInfo(G4int neutronID, G4int fGe77) 
    : nCNeutronID(neutronID), nCfGe77(fGe77) {}

// Destructor
MyTrackInfo::~MyTrackInfo() {}

G4int MyTrackInfo::GetnCNeutronID() const { return nCNeutronID; }
void MyTrackInfo::SetnCNeutronID(G4int neutronID) { nCNeutronID = neutronID; }

G4bool MyTrackInfo::GetnCfGe77() const { return nCfGe77; }
void MyTrackInfo::SetnCfGe77(G4bool fGe77) { nCfGe77 = fGe77; }

// Inheriting Primary ID and 1st Gen Neutron ID. For Muon track, the 1st Gen Neutron ID is set to -1.
// Also save Gamma data from Neutron captures.
void MySteppingAction::UserSteppingAction(const G4Step* step) {
    const G4Track* track = step->GetTrack();

    if (track->GetParentID() == 0) { // Primary Particle
        auto* info = new MyTrackInfo(-1, false);
        track->SetUserInformation(info);
    }

    // Muon is the primary particle
    const auto* userInfo = track->GetUserInformation();
    if (!userInfo)  {
        RMGLog::OutDev(RMGLog::error, "No user information for the track.");
        return;
    }

    const auto* trackInfo = dynamic_cast<const MyTrackInfo*>(userInfo);
    if (!trackInfo) {
        RMGLog::OutDev(RMGLog::error, "No valid track information in user info.");
        return;
    }

    // Check if the process is neutron capture (nCapture)
    G4StepPoint* postStepPoint = step->GetPostStepPoint();
    const std::vector<const G4Track*>* secondaries = step->GetSecondaryInCurrentStep();
    MyTrackInfo* nonConstTrackInfo = const_cast<MyTrackInfo*>(trackInfo);
    if (postStepPoint->GetProcessDefinedStep()->GetProcessName() == "nCapture") {
        // Ensure the captured particle is a neutron
        if (track->GetParticleDefinition() == G4Neutron::Definition()) {
            // G4cout << "Neutron capture detected" << G4endl;
            // Use const_cast to remove the const qualifier and modify the object
            nonConstTrackInfo->SetnCNeutronID(track->GetTrackID());

            // Flag set if Ge77 was produced.
            for (const auto& secTrack : *secondaries) {
                const auto particle = secTrack->GetParticleDefinition();
                if (particle->IsGeneralIon()) {
                    int z = particle->GetAtomicNumber();
                    int a = particle->GetAtomicMass();
                    if (z == 32 && a == 77) { // Ge77?
                        // Remove const qualifier to modify nCfGe77
                        nonConstTrackInfo->SetnCfGe77(true);
                        G4cout << "Ge-77 erzeugt! 🎉" << G4endl;
                    }
                }
            }
        }
    }

    // Inherit Track Info to secondary particles
    for (const auto& secTrack : *secondaries) {
        auto* inheritedInfo = new MyTrackInfo(nonConstTrackInfo->GetnCNeutronID(), nonConstTrackInfo->GetnCfGe77());
        // Set user information for the secondary track
        const_cast<G4Track*>(secTrack)->SetUserInformation(inheritedInfo);

    }   
}
