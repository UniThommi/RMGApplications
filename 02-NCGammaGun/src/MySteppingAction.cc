// Transfers the NC label of the primary vertex (MyPrimaryGammaUserInfo, set by the
// gamma generators) to the primary gamma tracks as MyTrackInfo and passes it on to all
// secondaries, so that the optical photons carry the (muon ID, NC ID, gamma ID) of a
// neutron capture of the first simulation (see the note on vertex 0 below).

#include "MySteppingAction.hh"
#include "MyTrackInfo.hh"
#include "MyPrimaryGammaUserInfo.hh"

#include "RMGOpticalDetector.hh"
#include "RMGLog.hh"
#include "RMGManager.hh"
#include "RMGHardware.hh"

#include "G4OpticalPhoton.hh"
#include "G4SDManager.hh"
#include "G4ParticleDefinition.hh"
#include "G4ParticleTypes.hh"
#include "G4VProcess.hh"
#include "G4RunManager.hh"
#include "G4Neutron.hh"
#include "G4OpticalPhoton.hh"
#include "G4Step.hh"
#include "G4EventManager.hh"

namespace u = CLHEP;

MySteppingAction::MySteppingAction() {
        this->DefineCommands();
    }

// Destructor
MySteppingAction::~MySteppingAction() {}


// Labels the primary gammas and passes the label of every labelled track on to its
// secondaries.
void MySteppingAction::UserSteppingAction(const G4Step* step) {
    const G4Track* track = step->GetTrack();
    G4StepPoint* postStepPoint = step->GetPostStepPoint();

    const std::vector<const G4Track*>* secondaries = step->GetSecondaryInCurrentStep();

    // Check if this is a primary particle without UserInfo yet
    if (track->GetParentID() == 0 && !track->GetUserInformation()) {
        // This is a primary particle from the gun
        // Try to get UserInfo from vertex. Every primary track takes the label of
        // vertex 0 of the event, not of the vertex it was created from: with
        // --generator single all vertices belong to one NC, but their gamma IDs
        // differ; with --generator muon they can belong to different NCs.
        const G4Event* event = G4EventManager::GetEventManager()->GetConstCurrentEvent();
        const G4PrimaryVertex* vertex = event->GetPrimaryVertex(0);
        const auto* vertexUserInfo = dynamic_cast<const MyPrimaryGammaUserInfo*>(vertex->GetUserInformation());

        if (vertexUserInfo) {
            // Transfer vertex UserInfo to track UserInfo
            auto* trackInfo = new MyTrackInfo(
                vertexUserInfo->GetMuonID(),
                vertexUserInfo->GetNCID(),
                vertexUserInfo->GetGammaID()
            );
            const_cast<G4Track*>(track)->SetUserInformation(trackInfo);
        }
    }

    // Get track info (now it should exist)
    const auto* userInfo = track->GetUserInformation();
    const auto* trackInfo = dynamic_cast<const MyTrackInfo*>(userInfo);
    if (!trackInfo) {
        return;
    }

    // BEGIN UNUSED CODE — nonConstTrackInfo is never read
    MyTrackInfo* nonConstTrackInfo = const_cast<MyTrackInfo*>(trackInfo);
    // END UNUSED CODE

    // Pass the UserInfo on to all secondaries
    for (const auto& secTrack : *secondaries) {
        auto* inheritedInfo = new MyTrackInfo(trackInfo->GetMuonID(), trackInfo->GetNCID(), trackInfo->GetGammaID());
        const_cast<G4Track*>(secTrack)->SetUserInformation(inheritedInfo);
    }
}

// No messenger commands are defined; kept as a placeholder.
void MySteppingAction::DefineCommands() {
  //
}
