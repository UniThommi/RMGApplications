#include "MyPrimaryGammaUserInfo.hh"

// Constructor initializes track IDs
MyPrimaryGammaUserInfo::MyPrimaryGammaUserInfo(G4int muonID, G4int physVolumeID, G4int materialID, G4int fGe77, G4int nCGammaAmount, G4double nCGammaTotalEnergy)
    : muonID(muonID), physVolumeID(physVolumeID), materialID(materialID), fGe77(fGe77), nCGammaAmount(nCGammaAmount), nCGammaTotalEnergy(nCGammaTotalEnergy) {}

// Destruktor
MyPrimaryGammaUserInfo::~MyPrimaryGammaUserInfo() {}


// Setter-Methode für 1. Gen Neutron-ID
G4int MyPrimaryGammaUserInfo::GetMuonID() const {
    return muonID;
}

// Getter-Methode für 1. Gen Neutron-ID
G4int MyPrimaryGammaUserInfo::GetnCPhysVolumeID() const {
    return physVolumeID;
}

G4int MyPrimaryGammaUserInfo::GetnCMaterialID() const {
    return materialID;
}

G4int MyPrimaryGammaUserInfo::GetnCfGe77() const {
    return fGe77;
}

G4int MyPrimaryGammaUserInfo::GetnCGammaAmount() const {
    return nCGammaAmount;
}

G4double MyPrimaryGammaUserInfo::GetnCGammaTotalEnergy() const {
    return nCGammaTotalEnergy;
}