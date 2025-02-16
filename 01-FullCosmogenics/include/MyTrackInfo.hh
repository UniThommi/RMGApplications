#ifndef MYTRACKINFO_HH
#define MYTRACKINFO_HH

#include "G4VUserTrackInformation.hh"
#include "G4UserSteppingAction.hh"
#include "G4Track.hh"
#include "G4Step.hh"
#include <vector>

class MyTrackInfo : public G4VUserTrackInformation {
public:
    explicit MyTrackInfo(G4int neutronID);
    ~MyTrackInfo() override;

    void SetGen1NeutronID(G4int id);
    G4int GetGen1NeutronID() const;

private:
    G4int gen1NeutronID;
};


class MySteppingAction : public G4UserSteppingAction {
public:
    void UserSteppingAction(const G4Step* step) override;
};


#endif // MYTRACKINFO_HH
