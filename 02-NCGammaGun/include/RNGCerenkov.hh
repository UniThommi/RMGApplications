#ifndef _RNG_CERENKOV_HH_
#define _RNG_CERENKOV_HH_

#include "G4Cerenkov.hh"
#include "Randomize.hh"
#include "globals.hh"

class RNGCerenkov : public G4Cerenkov {
public:
  static G4ThreadLocal CLHEP::HepRandomEngine *fAlternativeEngine;

  RNGCerenkov(const G4String &name = "RNGCerenkov");

  virtual G4VParticleChange *PostStepDoIt(const G4Track &track,
                                          const G4Step &step) override;

  G4double
  PostStepGetPhysicalInteractionLength(const G4Track &aTrack, G4double,
                                       G4ForceCondition *condition) override;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab
