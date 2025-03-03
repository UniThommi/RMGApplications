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

    // NeutronCapture Information
    std::vector<G4String> GetnCPhysVolume() const;
    std::vector<G4String> GetnCMaterial() const;
    std::vector<G4double> GetnCTime() const;
    std::vector<G4int> GetfGe77() const;

    std::vector<G4ThreeVector> GetGammaPosition() const;
    std::vector<G4ThreeVector> GetGammaMomentumDirection() const;
    std::vector<G4double> GetGammaKinEnergy() const;

    G4int GetEventID() const;
    G4int GetTrackID() const;


private:
    std::vector<G4String> nCPhysVolume;
    std::vector<G4String> nCMaterial;
    std::vector<G4double> nCTime;
    std::vector<G4int> fGe77;
    std::vector<G4int> fNeutronSeenByPMTs; // True wenn das Neutron, welches im nC involviert ist, von optischen Detektoren gesehen wurde (involviert auch alle Prozesse vor dem nC)

    std::vector<G4ThreeVector> gammaPosition;
    std::vector<G4ThreeVector> gammaMomentumDirection;
    std::vector<G4double> gammaKinEnergy;

    G4int eventID;
    G4int trackID;


};


class MySteppingAction : public G4UserSteppingAction {
public:
    void UserSteppingAction(const G4Step* step) override;
};


#endif // MY_TRACK_INFO_HH
