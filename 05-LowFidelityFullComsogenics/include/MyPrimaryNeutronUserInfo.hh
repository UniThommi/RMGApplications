#ifndef MY_PRIMARY_NEUTRON_USER_INFO_H
#define MY_PRIMARY_NEUTRON_USER_INFO_H 1

#include "G4VUserPrimaryVertexInformation.hh"

class MyPrimaryNeutronUserInfo : public G4VUserPrimaryVertexInformation {
  public:
    MyPrimaryNeutronUserInfo(G4int physVolumeID, G4int materialID, G4bool fGe77);
    ~MyPrimaryNeutronUserInfo();

    // void SetPrimaryNeutronfGe77(G4bool flag);
    G4int GetPhysVolumeID() const;
    G4int GetMaterialID() const;
    G4bool GetPrimaryNeutronfGe77() const;

  private:
    G4int physVolumeID;
    G4int materialID;
    G4int fGe77;
};

#endif