// PhotonHit.cc
#include "MyPhotonHit.hh"

PhotonHit::PhotonHit() 
    : detectorUID(-1),
    optPhotonEnergy(-1.),
    optPhotonGlobalTime(-1.),
    optPhotonPosition(-1., -1., -1.),
    optPhotonMomentumDirection(-1., -1., -1.),
    nCTrackID(-1),
    nCPos(-1., -1., -1.),
    nCTime(-1.),
    nCPhysVol(-1),
    nCMaterial(-1),
    nCGammaAmount(-1),
    nCGammaTotalEnergy(-1.),
    nCfGe77(false),
    gammaMomentumDirection(-1., -1., -1.),
    gammaKineticEnergy(-1.)
    {}

PhotonHit::~PhotonHit() {}

// Setters for the hit data
void PhotonHit::SetDetectorUID(G4int _detectorUID) { detectorUID = _detectorUID };
void PhotonHit::SetOptPhotonEnergy(G4double _optPhotonEnergy) { optPhotonEnergy = _optPhotonEnergy };
void PhotonHit::SetOptPhotonglobalTime(G4double _optPhotonglobalTime) { optPhotonGlobalTime = _optPhotonglobalTime };
void PhotonHit::SetOptPhotonPosition(G4ThreeVector _optPhotonPosition) { optPhotonPosition = _optPhotonPosition };
void PhotonHit::SetOptPhotonMomentumDirection(G4ThreeVector _optPhotonMomentumDirection) { optPhotonMomentumDirection = _optPhotonMomentumDirection };
void PhotonHit::SetnCTrackID(G4int _nCTrackID) { nCTrackID = _nCTrackID };
void PhotonHit::SetnCPos(G4ThreeVector _nCPos) { nCPos = _nCPos };
void PhotonHit::SetnCTime(G4double _nCTime) { nCTime = _nCTime };
void PhotonHit::SetnCPhysVol(G4string _nCPhysVolID) { nCPhysVolID = _nCPhysVolID };
void PhotonHit::SetnCMaterial(G4string _nCMaterialID) { nCMaterialID = _nCMaterialID };
void PhotonHit::SetnCGammaAmount(G4int _nCGammaAmount) { nCGammaAmount = _nCGammaAmount };
void PhotonHit::SetnCGammaTotalEnergy(G4double _nCGammaTotalEnergy) { nCGammaTotalEnergy = _nCGammaTotalEnergy };
void PhotonHit::SetnCfGe77(G4bool _nCfGe77) { nCfGe77 = _nCfGe77 };
void PhotonHit::SetGammaMomentumDirection(G4ThreeVector _gammaMomentumDirection) { gammaMomentumDirection = _gammaMomentumDirection };
void PhotonHit::SetGammaKineticEnergy(G4double _gammaKineticEnergy) { gammaKineticEnergy = _gammaKineticEnergy };

// Initialisiere den globalen Allocator-Pointer (anfangs nullptr)
G4Allocator<PhotonHit>* PhotonHitAllocator = nullptr;

// new-Operator überladen, damit G4Allocator benutzt wird
void* PhotonHit::operator new(size_t)
{
    if (!PhotonHitAllocator) {
        PhotonHitAllocator = new G4Allocator<PhotonHit>;
    }
    return (void*) PhotonHitAllocator->MallocSingle();
}

// delete-Operator überladen, damit Speicher korrekt freigegeben wird
void PhotonHit::operator delete(void* hit)
{
    PhotonHitAllocator->FreeSingle((PhotonHit*)hit);
}
