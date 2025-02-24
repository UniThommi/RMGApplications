#include "MyPrimaryNeutronUserInfo.hh"

// Constructor initializes track IDs
MyPrimaryNeutronUserInfo::MyPrimaryNeutronUserInfo(G4int volID, G4int matID, G4bool flag)
    : physVolumeID(physVolumeID), materialID(materialID), fGe77(fGe77) {}

// Destruktor
MyPrimaryNeutronUserInfo::~MyPrimaryNeutronUserInfo() {}


// Setter-Methode für 1. Gen Neutron-ID
// void MyPrimaryNeutronUserInfo::SetPrimaryNeutronfGe77(G4int id) {
//     fGe77 = id;
// }

// Getter-Methode für 1. Gen Neutron-ID
G4int MyPrimaryNeutronUserInfo::GetPhysVolumeID() const {
    return physVolumeID;
}

G4int MyPrimaryNeutronUserInfo::GetMaterialID() const {
    return materialID;
}

G4bool MyPrimaryNeutronUserInfo::GetPrimaryNeutronfGe77() const {
    return fGe77;
}