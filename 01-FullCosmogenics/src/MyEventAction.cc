#include "MyEventAction.hh"
#include "MyPhotonHit.hh" // dein benutzerdefinierter Hit-Typ

#include "G4Event.hh"
#include "MyPhotonHitsCollection.hh"
#include "G4SystemOfUnits.hh"
#include "G4RunManager.hh"


#include <iostream>

MyEventAction::MyEventAction() {
    this->DefineCommands();
};

MyEventAction::~MyEventAction() = default;

void MyEventAction::BeginOfEventAction(const G4Event* event) {
    fPhotonHitsCollection = new PhotonHitsCollection("OpticalPhotonHitsCollection", 0);
}


void MyEventAction::EndOfEventAction(const G4Event* event) {
    RMGManager::Instance()->SetPhotonHitsCollection(fPhotonHitsCollection);  // Beispiel

    
}

void MyEventAction::DefineCommands() {
  
}
