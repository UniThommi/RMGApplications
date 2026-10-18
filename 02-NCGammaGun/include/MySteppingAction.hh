#ifndef MY_STEPPING_ACTION_HH
#define MY_STEPPING_ACTION_HH

#include "G4UserSteppingAction.hh"
#include "G4Track.hh"
#include "G4Step.hh"
#include <vector>


// Labels the primary gammas with the (muon ID, NC ID, gamma ID) of a neutron capture
// of the first simulation (MyTrackInfo, provided by the remage fork) and passes the
// label on to all secondaries. The optical detector of the remage fork writes these IDs
// to each optical hit.
class MySteppingAction : public G4UserSteppingAction {
public:
    MySteppingAction();
    ~MySteppingAction() override;

    void UserSteppingAction(const G4Step* step) override;

private:
    void DefineCommands();
};

#endif // MY_STEPPING_ACTION_HH
