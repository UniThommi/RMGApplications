#ifndef MY_PRIMARY_GAMMA_USER_INFO_H
#define MY_PRIMARY_GAMMA_USER_INFO_H 1

#include "G4VUserPrimaryVertexInformation.hh"

class MyPrimaryGammaUserInfo : public G4VUserPrimaryVertexInformation {
  public:
    MyPrimaryGammaUserInfo(G4int muonID, G4int physVolumeID, G4int materialID, G4int fGe77, G4int nCGammaAmount, G4double nCGammaTotalEnergy);
    ~MyPrimaryGammaUserInfo();

    G4int GetMuonID() const;
    G4int GetnCPhysVolumeID() const;
    G4int GetnCMaterialID() const;
    G4int GetnCfGe77() const;
    G4int GetnCGammaAmount() const;
    G4int GetnCGammaTotalEnergy() const;

  private:
    G4int muonID;
    G4int physVolumeID;
    G4int materialID;
    G4int fGe77;
    G4int nCGammaAmount;
    G4double nCGammaTotalEnergy;

};

#endif