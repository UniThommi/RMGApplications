#include "MyTrackInfo.hh"
#include "RMGLog.hh"

// Constructor initializes track IDs
MyTrackInfo::MyTrackInfo(G4int neutronID)
    : gen1NeutronID(neutronID) {}

// Destruktor
MyTrackInfo::~MyTrackInfo() {}


// Setter-Methode für 1. Gen Neutron-ID
void MyTrackInfo::SetGen1NeutronID(G4int id) {
    gen1NeutronID = id;
}

// Getter-Methode für 1. Gen Neutron-ID
G4int MyTrackInfo::GetGen1NeutronID() const {
    return gen1NeutronID;
}

// Vererbung der Primary-ID und 1. Gen Neutron-ID. Für Muon Track wird 1. Gen Neutron-ID auf -1 gesetzt.
void MySteppingAction::UserSteppingAction(const G4Step* step) {
    const G4Track* track = step->GetTrack();

    if (track->GetParentID() == 0) { // Primary Particle
        auto* info = new MyTrackInfo(-1);
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

    G4int neutronID = trackInfo->GetGen1NeutronID();

    // Inherit User Information
    const std::vector<const G4Track*>* secondaries = step->GetSecondaryInCurrentStep();
    for (const auto* secondary : *secondaries) {
        auto* inheritedInfo = new MyTrackInfo(neutronID);
        secondary->SetUserInformation(inheritedInfo);
    }
}

// vim: tabstop=2 shiftwIDth=2 expandtab