#ifndef MY_PRIMARY_GAMMA_USER_INFO_HH
#define MY_PRIMARY_GAMMA_USER_INFO_HH

#include "G4VUserPrimaryVertexInformation.hh"
#include "G4Types.hh"

class MyPrimaryGammaUserInfo : public G4VUserPrimaryVertexInformation {
public:
    explicit MyPrimaryGammaUserInfo(G4int muonID = -1, G4int ncID = -1, G4int gammaID = -1);
    ~MyPrimaryGammaUserInfo();

    virtual void Print() const override {};

    G4int GetMuonID() const;
    G4int GetNCID() const;
    G4int GetGammaID() const;

private:
    G4int fMuonID;
    G4int fNCID;
    G4int fGammaID;
};

#endif // MY_PRIMARY_GAMMA_USER_INFO_HH