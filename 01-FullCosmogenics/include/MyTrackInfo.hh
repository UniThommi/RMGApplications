#ifndef MY_TRACK_INFO_HH
#define MY_TRACK_INFO_HH

#include "G4VUserTrackInformation.hh"
#include "G4UserSteppingAction.hh"
#include "G4Track.hh"
#include "G4Step.hh"
#include <vector>

class MyTrackInfo : public G4VUserTrackInformation {
public:
    explicit MyTrackInfo(G4int eventID, G4int trackID);
    ~MyTrackInfo() override;

    G4int GetnCNeutronID() const;
    void SetnCNeutronID(G4int neutronID);

    G4bool GetnCfGe77() const;
    void SetnCfGe77(G4bool flag);


private:

    G4int nCNeutronID;
    G4bool nCfGe77;


};


class MySteppingAction : public G4UserSteppingAction {
public:
    void UserSteppingAction(const G4Step* step) override;
};


#endif // MY_TRACK_INFO_HH
