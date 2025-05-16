#ifndef MY_STEPPING_ACTION_HH
#define MY_STEPPING_ACTION_HH

#include "G4UserSteppingAction.hh"
#include "G4Track.hh"
#include "G4Step.hh"
#include <vector>


class MySteppingAction : public G4UserSteppingAction {
public:
    MySteppingAction(); // Konstruktor
    ~MySteppingAction() override;

    void UserSteppingAction(const G4Step* step) override;

private:
    void DefineCommands();
};

#endif // MY_STEPPING_ACTION_HH
