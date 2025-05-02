#ifndef MY_STEPPING_ACTION
#define MY_STEPPING_ACTION

#include "MyEventAction.hh"

#include "G4UserSteppingAction.hh"
#include "G4Track.hh"
#include "G4Step.hh"
#include <vector>

class MyEventAction;

class MySteppingAction : public G4UserSteppingAction {
public:
    MySteppingAction(MyEventAction* eventAction); // Konstruktor
    ~MySteppingAction() override;

    void UserSteppingAction(const G4Step* step) override;

private:
    MyEventAction* fEventAction = nullptr;
};


#endif // MY_STEPPING_ACTION
