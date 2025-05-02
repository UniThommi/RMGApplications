#ifndef MY_TRACK_INFO_HH
#define MY_TRACK_INFO_HH

#include "G4VUserTrackInformation.hh"
#include "G4Step.hh"
#include <vector>

class MyTrackInfo : public G4VUserTrackInformation {
public:
    explicit MyTrackInfo(
        G4int trackID,
        G4ThreeVector pos,
        G4double time,
        G4String physVol,
        G4String material,
        G4int amount,
        G4double totalEnergy,
        G4bool fGe77,
        G4ThreeVector momentumDirection,
        G4double kineticEnergy
    );
    ~MyTrackInfo() override;

    G4int GetnCTrackID() const;
    void SetnCTrackID(G4int trackID);

    const G4ThreeVector& GetnCPos() const;
    void SetnCPos(const G4ThreeVector& pos);

    G4bool GetnCfGe77() const;
    void SetnCfGe77(G4bool fGe77);

    G4double GetnCTime() const;
    void SetnCTime(G4double time);

    const G4String& GetnCPhysVol() const;
    void SetnCPhysVol(const G4String& physVol);

    const G4String& GetnCMaterial() const;
    void SetnCMaterial(const G4String& material);

    G4int GetnCGammaAmount() const;
    void SetnCGammaAmount(G4int amount);

    G4double GetnCGammaTotalEnergy() const;
    void SetnCGammaTotalEnergy(G4double totalEnergy);

    const G4ThreeVector& GetGammaMomentumDirection() const;
    void SetGammaMomentumDirection(const G4ThreeVector& momentumDirection);

    G4double GetGammaKineticEnergy() const;
    void SetGammaKineticEnergy(G4double kineticEnergy);

private:
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

#endif // MY_TRACK_INFO_HH
