// // PhotonHit.cc
// #include "MyPhotonHit.hh"

// #include "G4VVisManager.hh"
// #include "G4VisManager.hh"
// #include "G4Circle.hh"
// #include "G4Colour.hh"
// #include "G4VisAttributes.hh"


// PhotonHit::PhotonHit() 
//     : detectorUID(-1),
//     optPhotonEnergy(-1.),
//     optPhotonGlobalTime(-1.),
//     optPhotonPosition(-1., -1., -1.),
//     optPhotonMomentumDirection(-1., -1., -1.),
//     nCTrackID(-1),
//     nCPos(-1., -1., -1.),
//     nCTime(-1.),
//     nCPhysVol(""),
//     nCMaterial(""),
//     nCGammaAmount(-1),
//     nCGammaTotalEnergy(-1.),
//     nCfGe77(false),
//     gammaMomentumDirection(-1., -1., -1.),
//     gammaKineticEnergy(-1.)
//     {}

// PhotonHit::~PhotonHit() {}


// PhotonHit::PhotonHit(const PhotonHit& right) : G4VHit(right) {
//     *this = right;
// }

// PhotonHit& PhotonHit::operator=(const PhotonHit& right) {
//     if (this != &right) {
//         detectorUID = right.detectorUID;
//         optPhotonEnergy = right.optPhotonEnergy;
//         optPhotonGlobalTime = right.optPhotonGlobalTime;
//         optPhotonPosition = right.optPhotonPosition;
//         optPhotonMomentumDirection = right.optPhotonMomentumDirection;
//         nCTrackID = right.nCTrackID;
//         nCPos = right.nCPos;
//         nCTime = right.nCTime;
//         nCPhysVol = right.nCPhysVol;
//         nCMaterial = right.nCMaterial;
//         nCGammaAmount = right.nCGammaAmount;
//         nCGammaTotalEnergy = right.nCGammaTotalEnergy;
//         nCfGe77 = right.nCfGe77;
//         gammaMomentumDirection = right.gammaMomentumDirection;
//         gammaKineticEnergy = right.gammaKineticEnergy;
//     }
//     return *this;
// }

// // Setters for the hit data
// void PhotonHit::SetDetectorUID(G4int uid) { this->detectorUID = uid; }
// void PhotonHit::SetOptPhotonEnergy(G4double energy) { this->optPhotonEnergy = energy; }
// void PhotonHit::SetOptPhotonGlobalTime(G4double time) { this->optPhotonGlobalTime = time; }
// void PhotonHit::SetOptPhotonPosition(const G4ThreeVector& pos) { this->optPhotonPosition = pos; }
// void PhotonHit::SetOptPhotonMomentumDirection(const G4ThreeVector& momentumDirection) { this->optPhotonMomentumDirection = momentumDirection; }
// void PhotonHit::SetnCTrackID(G4int trackID) { this->nCTrackID = trackID; }
// void PhotonHit::SetnCPos(const G4ThreeVector& pos) { this->nCPos = pos; }
// void PhotonHit::SetnCTime(G4double time) { this->nCTime = time; }
// void PhotonHit::SetnCPhysVol(const G4String& physVol) { this->nCPhysVol = physVol; }
// void PhotonHit::SetnCMaterial(const G4String& material) { this->nCMaterial = material; }
// void PhotonHit::SetnCGammaAmount(G4int amount) { this->nCGammaAmount = amount; }
// void PhotonHit::SetnCGammaTotalEnergy(G4double totalEnergy) { this->nCGammaTotalEnergy = totalEnergy; }
// void PhotonHit::SetnCfGe77(G4bool fGe77) { this->nCfGe77 = fGe77; }
// void PhotonHit::SetGammaMomentumDirection(const G4ThreeVector& momentumDirection) { this->gammaMomentumDirection = momentumDirection; }
// void PhotonHit::SetGammaKineticEnergy(G4double kineticEnergy) { this->gammaKineticEnergy = kineticEnergy; }

// // Getter implementations
// G4int PhotonHit::GetDetectorUID() const { return this->detectorUID; }
// G4double PhotonHit::GetOptPhotonEnergy() const { return this->optPhotonEnergy; }
// G4double PhotonHit::GetOptPhotonGlobalTime() const { return this->optPhotonGlobalTime; }
// const G4ThreeVector& PhotonHit::GetOptPhotonPosition() const { return this->optPhotonPosition; }
// const G4ThreeVector& PhotonHit::GetOptPhotonMomentumDirection() const { return this->optPhotonMomentumDirection; }
// G4int PhotonHit::GetnCTrackID() const { return this->nCTrackID; }
// const G4ThreeVector& PhotonHit::GetnCPos() const { return this->nCPos; }
// G4double PhotonHit::GetnCTime() const { return this->nCTime; }
// const G4String& PhotonHit::GetnCPhysVol() const { return this->nCPhysVol; }
// const G4String& PhotonHit::GetnCMaterial() const { return this->nCMaterial; }
// G4int PhotonHit::GetnCGammaAmount() const { return this->nCGammaAmount; }
// G4double PhotonHit::GetnCGammaTotalEnergy() const { return this->nCGammaTotalEnergy; }
// G4bool PhotonHit::GetnCfGe77() const { return this->nCfGe77; }
// const G4ThreeVector& PhotonHit::GetGammaMomentumDirection() const { return this->gammaMomentumDirection; }
// G4double PhotonHit::GetGammaKineticEnergy() const { return this->gammaKineticEnergy; }


// G4bool PhotonHit::operator==(const PhotonHit& right) const {
//     return (this == &right); // You may refine this if needed
// }

// // Required by G4VHit
// void PhotonHit::Draw() {
//     G4VVisManager* pVVisManager = G4VVisManager::GetConcreteInstance();
//     if (pVVisManager) {
//         G4Circle circle(optPhotonPosition);
//         circle.SetScreenSize(4.);
//         circle.SetFillStyle(G4Circle::filled);
//         G4Colour colour(1.0, 0.0, 1.0);  // magenta
//         G4VisAttributes attribs(colour);
//         circle.SetVisAttributes(attribs);
//         pVVisManager->Draw(circle);
//     }
// }

// void PhotonHit::Print() {
//     G4cout << "PhotonHit in DetectorUID: " << detectorUID << G4endl;
//     G4cout << "Energy: " << optPhotonEnergy << "Energy" << G4endl;
//     G4cout << "Global Time: " << optPhotonGlobalTime << "Time" << G4endl;
//     G4cout << "Position: " << optPhotonPosition << "Position" << G4endl;
//     G4cout << "Momentum Dir: " << optPhotonMomentumDirection << G4endl;
//     G4cout << "TrackID: " << nCTrackID << ", Origin: " << nCPhysVol << ", Material: " << nCMaterial << G4endl;
// }

// G4Allocator<PhotonHit>* PhotonHitAllocator = nullptr;

// // new-Operator überladen, damit G4Allocator benutzt wird
// void* PhotonHit::operator new(size_t)
// {
//     if (!PhotonHitAllocator) {
//         PhotonHitAllocator = new G4Allocator<PhotonHit>;
//     }
//     return (void*) PhotonHitAllocator->MallocSingle();
// }

// // delete-Operator überladen, damit Speicher korrekt freigegeben wird
// void PhotonHit::operator delete(void* hit)
// {
//     PhotonHitAllocator->FreeSingle((PhotonHit*)hit);
// }
