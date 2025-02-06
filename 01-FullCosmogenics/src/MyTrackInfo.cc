#include "MyTrackInfo.hh"

// Konstruktor: Weist die Primary-Track-ID zu
MyTrackInfo::MyTrackInfo(G4int primaryID) : primaryTrackID(primaryID) {}

// Destruktor
MyTrackInfo::~MyTrackInfo() {}

// Setter-Methode für Primary-ID
void MyTrackInfo::SetPrimaryID(G4int id) {
    primaryTrackID = id;
}

// Getter-Methode für Primary-ID
G4int MyTrackInfo::GetPrimaryID() const {
    return primaryTrackID;
}

// Setter-Methode für 1. Gen Neutron-ID
void MyTrackInfo::SetGen1NeutronID(G4int id) {
    gen1NeutronID = id;
}

// Getter-Methode für 1. Gen Neutron-ID
G4int MyTrackInfo::GetGen1NeutronID() const {
    return gen1NeutronID;
}


// Vererbung der Primary-ID und 1. Gen Neutron-ID. Für Muon Track wird 1. Gen Neutron-ID auf -1 gesetzt.
void MyTrackingAction::PreUserTrackingAction(const G4Track* track) {
    if (track->GetParentID() == 0) { // Primary Particle
        auto* info = new MyTrackInfo(track->GetTrackID(), -1);
        track->SetUserInformation(info);
    } else {
        const G4Track* parentTrack = track->GetParentTrack();
        auto* parentInfo = dynamic_cast<MyTrackInfo*>(parentTrack->GetUserInformation());
        if (parentInfo) {
            auto* info = new MyTrackInfo(parentInfo->GetPrimaryID(), parentInfo->GetGen1NeutronID());
            track->SetUserInformation(info);
        }
    }
}

// vim: tabstop=2 shiftwIDth=2 expandtab