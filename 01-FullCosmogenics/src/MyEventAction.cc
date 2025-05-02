#include "MyEventAction.hh"
#include "MyPhotonHit.hh" // dein benutzerdefinierter Hit-Typ

#include "G4Event.hh"
#include "G4HCofThisEvent.hh"
#include "G4SDManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4RunManager.hh"


#include <iostream>

MyEventAction::MyEventAction() = default;

MyEventAction::~MyEventAction() = default;

void MyEventAction::BeginOfEventAction(const G4Event* event) {
    auto photonHits = new PhotonHitsCollection("PhotonSD", "PhotonHitsCollection");
    
    G4int hcID = G4SDManager::GetSDMpointer()->GetCollectionID("PhotonSD/PhotonHitsCollection");
    if (hcID < 0) {
        G4Exception("EventAction", "HCIDNotFound", FatalException, "Could not find HCID!");
        G4cout << "FATAL ERROR: Could not find HCID!" << G4endl;
    }

    G4HCofThisEvent* hce = event->GetHCofThisEvent();
    if (!hce) {
        hce = new G4HCofThisEvent();
        const_cast<G4Event*>(event)->SetHCofThisEvent(hce);
    }

    hce->AddHitsCollection(hcID, photonHits);
}


void MyEventAction::EndOfEventAction(const G4Event* event) {
    fPhotonHitsCollection = nullptr;
}
