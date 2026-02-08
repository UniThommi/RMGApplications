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


// Inheriting Primary ID and 1st Gen Neutron ID. For Muon track, the 1st Gen Neutron ID is set to -1.
// Also save Gamma data from Neutron captures.
void MySteppingAction::UserSteppingAction(const G4Step* step) {
    const G4Track* track = step->GetTrack();
    G4StepPoint* postStepPoint = step->GetPostStepPoint();

    const std::vector<const G4Track*>* secondaries = step->GetSecondaryInCurrentStep();
    
    // Check if this is a primary particle without UserInfo yet
    if (track->GetParentID() == 0 && !track->GetUserInformation()) {
        // This is a primary particle from the gun
        // Try to get UserInfo from vertex
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

    MyTrackInfo* nonConstTrackInfo = const_cast<MyTrackInfo*>(trackInfo);

    // Vererbe UserInfo zu allen Sekundärteilchen
    for (const auto& secTrack : *secondaries) {
        auto* inheritedInfo = new MyTrackInfo(trackInfo->GetMuonID(), trackInfo->GetNCID(), trackInfo->GetGammaID());
        const_cast<G4Track*>(secTrack)->SetUserInformation(inheritedInfo);
    }
}    

void MySteppingAction::DefineCommands() {
  // 
}
