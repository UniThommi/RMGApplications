#ifndef _RNG_TRACKING_ACTION_HH_
#define _RNG_TRACKING_ACTION_HH_

#include "G4UserTrackingAction.hh"
#include "Randomize.hh"
#include "globals.hh"

class RNGTrackingAction : public G4UserTrackingAction {

public:
  RNGTrackingAction();
  ~RNGTrackingAction() = default;

  RNGTrackingAction(RNGTrackingAction const &) = delete;
  RNGTrackingAction &operator=(RNGTrackingAction const &) = delete;
  RNGTrackingAction(RNGTrackingAction &&) = delete;
  RNGTrackingAction &operator=(RNGTrackingAction &&) = delete;

  void PreUserTrackingAction(const G4Track *) override;
  void PostUserTrackingAction(const G4Track *) override;

private:
  bool ResetRNG = false;
  CLHEP::HepRandomEngine *defaultEngine;
  static G4ThreadLocal CLHEP::HepRandomEngine *fAlternativeEngine;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab
