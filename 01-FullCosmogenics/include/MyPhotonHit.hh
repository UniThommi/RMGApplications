#ifndef MY_PHOTON_HIT_HH
#define MY_PHOTON_HIT_HH

#include "G4ThreeVector.hh"
#include "G4Allocator.hh"
#include "G4VHit.hh"
#include "globals.hh"

// PhotonHit speichert alle relevanten Infos eines optischen Photons, das einen Sensitive Detector trifft
class PhotonHit : public G4VHit {
public:
    PhotonHit();
    virtual ~PhotonHit();

    PhotonHit(const PhotonHit&);
    PhotonHit& operator=(const PhotonHit&);
    G4bool operator==(const PhotonHit&) const;

    // Setters for the hit data
    void SetDetectorUID(G4int uid);
    void SetOptPhotonEnergy(G4double energy);
    void SetOptPhotonGlobalTime(G4double time);
    void SetOptPhotonPosition(const G4ThreeVector& pos);
    void SetOptPhotonMomentumDirection(const G4ThreeVector& momentumDirection);
    void SetnCTrackID(G4int trackID);
    void SetnCPos(const G4ThreeVector& pos);
    void SetnCTime(G4double time);
    void SetnCPhysVol(const G4String& physVol);
    void SetnCMaterial(const G4String& material);    
    void SetnCGammaAmount(G4int amount);
    void SetnCGammaTotalEnergy(G4double totalEnergy);
    void SetnCfGe77(G4bool fGe77);
    void SetGammaMomentumDirection(const G4ThreeVector& momentumDirection);
    void SetGammaKineticEnergy(G4double kineticEnergy);

    // Getter for the hit data
    G4int GetDetectorUID() const;
    G4double GetOptPhotonEnergy() const;
    G4double GetOptPhotonGlobalTime() const;
    const G4ThreeVector& GetOptPhotonPosition() const;
    const G4ThreeVector& GetOptPhotonMomentumDirection() const;
    G4int GetnCTrackID() const;
    const G4ThreeVector& GetnCPos() const;
    G4double GetnCTime() const;
    const G4String& GetnCPhysVol() const;
    const G4String& GetnCMaterial() const;
    G4int GetnCGammaAmount() const;
    G4double GetnCGammaTotalEnergy() const;
    G4bool GetnCfGe77() const;
    const G4ThreeVector& GetGammaMomentumDirection() const;
    G4double GetGammaKineticEnergy() const;

    // Geant4 required methods
    virtual void Draw() override;
    virtual void Print() override;

    // Allocator Overloads
    void* operator new(size_t);
    void operator delete(void*);

private:
    G4int detectorUID;                         // ID des getroffenen Detektors
    G4double optPhotonEnergy;                 // Energie des Photons (z.B. in eV)
    G4double optPhotonGlobalTime;             // Globalzeit des Treffers
    G4ThreeVector optPhotonPosition;           // Trefferposition
    G4ThreeVector optPhotonMomentumDirection;  // Flugrichtung beim Treffer

    G4int nCTrackID;
    G4ThreeVector nCPos;
    G4double nCTime;
    G4String nCPhysVol;
    G4String nCMaterial;
    G4int nCGammaAmount;
    G4double nCGammaTotalEnergy;
    G4bool nCfGe77;
    G4ThreeVector gammaMomentumDirection;
    G4double gammaKineticEnergy;               

};

// Globale Deklaration des Allocators
extern G4Allocator<PhotonHit>* PhotonHitAllocator;

#endif // MY_PHOTON_HIT_HH