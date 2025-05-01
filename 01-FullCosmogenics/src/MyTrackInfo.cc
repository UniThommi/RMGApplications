#include "MyTrackInfo.hh"

#include "RMGLog.hh"
#include "RMGHardware.hh"
#include "RMGManager.hh"


// Constructor initializes track IDs
MyTrackInfo::MyTrackInfo(
    G4int _nCTrackID,
    G4ThreeVector _nCPos,
    G4double _nCTime,
    G4string _nCPhysVol,
    G4string _nCMaterial,
    G4int _nCGammaAmount,
    G4double _nCGammaTotalEnergy,
    G4bool _nCfGe77,
    G4ThreeVector _gammaMomentumDirection,
    G4double _gammaKineticEnergy
) 
    : nCTrackID(_nCTrackID),
    nCPos(_nCPos), 
    nCTime(_nCTime),
    nCPhysVol(_nCPhysVol),
    nCMaterial(_nCMaterial),
    nCGammaAmount(_nCGammaAmount),
    nCGammaTotalEnergy(_nCGammaTotalEnergy),
    nCfGe77(_nCfGe77), 
    gammaMomentumDirection(_gammaMomentumDirection),
    gammaKineticEnergy(_gammaKineticEnergy)
    {}

// Destructor
MyTrackInfo::~MyTrackInfo() {}

// Get and Set Info
G4int MyTrackInfo::GetnCTrackID() const { return nCTrackID; }
void MyTrackInfo::SetnCTrackID(G4int _nCTrackID) { nCTrackID = _nCTrackID; }

G4ThreeVector MyTrackInfo::GetnCPos() const { return nCPos; }
void MyTrackInfo::SetnCPos(G4ThreeVector _nCPos) { nCPos = _nCPos; }

G4bool MyTrackInfo::GetnCfGe77() const { return nCfGe77; }
void MyTrackInfo::SetnCfGe77(G4bool _nCfGe77) { nCfGe77 = _nCfGe77; }

G4double MyTrackInfo::GetnCTime() const { return nCTime; }
void MyTrackInfo::SetnCTime(G4double _nCTime) { nCTime = _nCTime; }

G4string MyTrackInfo::GetnCPhysVol() const { return nCPhysVol; }
void MyTrackInfo::SetnCPhysVol(G4string& _nCPhysVol) { nCPhysVol = _nCPhysVol; }

G4string MyTrackInfo::GetnCMaterial() const { return nCMaterial; }
void MyTrackInfo::SetnCMaterial(G4string& _nCMaterial) { nCMaterial = _nCMaterial; }

G4int MyTrackInfo::GetnCGammaAmount() const { return nCGammaAmount; }
void MyTrackInfo::SetnCGammaAmount(G4int _nCGammaAmount) { nCGammaAmount = _nCGammaAmount; }

G4double MyTrackInfo::GetnCGammaTotalEnergy() const { return nCGammaTotalEnergy; }
void MyTrackInfo::SetnCGammaTotalEnergy(G4double _nCGammaTotalEnergy) { nCGammaTotalEnergy = _nCGammaTotalEnergy; } 

G4ThreeVector MyTrackInfo::GetGammaMomentumDirection() const { return gammaMomentumDirection; }
void MyTrackInfo::SetGammaMomentumDirection(G4ThreeVector _gammaMomentumDirection) { gammaMomentumDirection = _gammaMomentumDirection; }

G4double MyTrackInfo::GetGammaKineticEnergy() const { return gammaKineticEnergy; }
void MyTrackInfo::SetGammaKineticEnergy(G4double _gammaKineticEnergy) { gammaKineticEnergy = _gammaKineticEnergy; }

