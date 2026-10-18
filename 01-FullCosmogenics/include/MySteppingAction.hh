#ifndef MY_STEPPING_ACTION_HH
#define MY_STEPPING_ACTION_HH

#include "G4UserSteppingAction.hh"
#include "G4Track.hh"
#include "G4Step.hh"
#include <vector>


// Labels every track with the (muon ID, NC ID, gamma ID) of the neutron capture it
// descends from (MyTrackInfo, provided by the remage fork). The optical detector of
// the remage fork writes these IDs to each optical hit, so photons can be assigned to
// the capture that produced them.
class MySteppingAction : public G4UserSteppingAction {
public:
    MySteppingAction();
    ~MySteppingAction() override;

    void UserSteppingAction(const G4Step* step) override;

private:
    void DefineCommands();
};

#endif // MY_STEPPING_ACTION_HH
