#ifndef MY_EVENT_ACTION_HH
#define MY_EVENT_ACTION_HH

#include "G4UserEventAction.hh"
#include "G4THitsCollection.hh"

#include "MyPhotonHitsCollection.hh"

#include "globals.hh"

class G4Event;
class PhotonHit;

class MyEventAction : public G4UserEventAction {
  public:
    MyEventAction();
    virtual ~MyEventAction();

    // Aufgerufen am Anfang jedes Events
    void BeginOfEventAction(const G4Event* event) override;

    // Aufgerufen am Ende jedes Events
    void EndOfEventAction(const G4Event* event) override;

    PhotonHitsCollection* GetPhotonHitsCollection() const { return fPhotonHitsCollection; }

private:
    PhotonHitsCollection* fPhotonHitsCollection = nullptr; // HIER   G4int fPhotonHitsCollectionID = -1; // ID der PhotonHitCollection (einmalig abgefragt)
};

#endif // MY_EVENT_ACTION_HH