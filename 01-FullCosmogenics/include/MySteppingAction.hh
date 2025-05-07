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

    G4int optPhotonsRegister = 12130;
    G4int physVolRegister = 12131;
    G4int materialRegister = 12132;

    // Mappings: Physisches Volumen und Material
    std::map<std::string, int> physVolumeMapping;
    std::map<std::string, int> materialMapping;
};


#endif // MY_STEPPING_ACTION_HH
