
#include "RNGCerenkov.hh"

G4ThreadLocal CLHEP::HepRandomEngine *RNGCerenkov::fAlternativeEngine = nullptr;

RNGCerenkov::RNGCerenkov(const G4String &name) : G4Cerenkov(name) {
  if (!fAlternativeEngine) {
    fAlternativeEngine = new CLHEP::RanecuEngine;
  }
}

G4VParticleChange *RNGCerenkov::PostStepDoIt(const G4Track &track,
                                             const G4Step &step) {
  CLHEP::HepRandomEngine *defaultEngine = G4Random::getTheEngine();

  G4Random::setTheEngine(fAlternativeEngine);

  // Call the original Cerenkov PostStepDoIt
  G4VParticleChange *particleChange = G4Cerenkov::PostStepDoIt(track, step);

  G4Random::setTheEngine(defaultEngine);

  return particleChange;
}

G4double RNGCerenkov::PostStepGetPhysicalInteractionLength(
    const G4Track &aTrack, G4double, G4ForceCondition *condition) {
  G4double StepLimit = DBL_MAX;
  *condition = StronglyForced;
  return StepLimit;
}
