#include "MyPrimaryGammaUserInfo.hh"
#include "G4ios.hh"

MyPrimaryGammaUserInfo::MyPrimaryGammaUserInfo(G4int muonID, G4int ncID, G4int gammaID)
    : fMuonID(muonID), fNCID(ncID), fGammaID(gammaID) {}

MyPrimaryGammaUserInfo::~MyPrimaryGammaUserInfo() {}

void MyPrimaryGammaUserInfo::Print() const {
    G4cout << "MyPrimaryGammaUserInfo: MuonID=" << fMuonID 
           << " NCID=" << fNCID 
           << " GammaID=" << fGammaID << G4endl;
}

G4int MyPrimaryGammaUserInfo::GetMuonID() const { return fMuonID; }
G4int MyPrimaryGammaUserInfo::GetNCID() const { return fNCID; }
G4int MyPrimaryGammaUserInfo::GetGammaID() const { return fGammaID; }