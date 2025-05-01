#ifndef MY_TRACK_INFO_HH
#define MY_TRACK_INFO_HH

#include "G4VUserTrackInformation.hh"
#include "G4Step.hh"
#include <vector>

class MyTrackInfo : public G4VUserTrackInformation {
public:
    explicit MyTrackInfo(
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
    );
    ~MyTrackInfo() override;

    G4int GetnCTrackID() const;
    void SetnCTrackID(G4int nCTrackID);

    G4ThreeVector GetnCPos() const;
    void SetnCPos(G4ThreeVector nCPos);

    G4bool GetnCfGe77() const;
    void SetnCfGe77(G4bool flag);

    G4double GetnCTime() const;
    void SetnCTime(G4double nCTime);

    G4string GetnCPhysVol() const;
    void SetnCPhysVol(G4string& nCPhysVolID);

    G4string GetnCMaterial() const;
    void SetnCMaterial(G4string& nCMaterialID);

    G4int GetnCGammaAmount() const;
    void SetnCGammaAmount(G4int nCGammaAmount);

    G4double GetnCGammaTotalEnergy() const;
    void SetnCGammaTotalEnergy(G4double nCGammaTotalEnergy);

    G4ThreeVector GetGammaMomentumDirection() const;
    void SetGammaMomentumDirection(G4ThreeVector nCGammaMomentumDirection);

    G4double GetGammaKineticEnergy() const;
    void SetGammaKineticEnergy(G4double gammaKineticEnergy);

private:
    G4int nCTrackID;
    G4ThreeVector nCPos;
    G4double nCTime;
    G4string nCPhysVol;
    G4string nCMaterial;
    G4int nCGammaAmount;
    G4double nCGammaTotalEnergy;
    G4bool nCfGe77;
    G4ThreeVector gammaMomentumDirection;
    G4double gammaKineticEnergy;
    
};

#endif // MY_TRACK_INFO_HH
