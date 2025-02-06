#ifndef MYTRACKINFO_HH
#define MYTRACKINFO_HH

#include "G4VUserTrackInformation.hh"
#include "G4UserTrackingAction.hh"
#include "G4UserSteppingAction.hh"
#include "G4Track.hh"
#include "G4Step.hh"
#include "G4ios.hh"
#include <vector>

class MyTrackInfo : public G4VUserTrackInformation {
public:
    explicit MyTrackInfo(G4int primaryID);
    ~MyTrackInfo() override;

    void SetPrimaryID(G4int id);
    G4int GetPrimaryID() const;

    void SetGen1NeutronID(G4int id);
    G4int GetGen1NeutronID() const;

private:
    G4int primaryTrackID;
    G4int gen1NeutronID;
};

class MyTrackingAction : public G4UserTrackingAction {
public:
    void PreUserTrackingAction(const G4Track* track) override;
};


#endif // MYTRACKINFO_HH
