#include <set>

#include "MyGe77EventFilterOutputScheme.hh"

#include "G4Event.hh"
#include "G4EventManager.hh"
#include "G4OpticalPhoton.hh"
#include "RMGLog.hh"


MyGe77EventFilterOutputScheme::MyGe77EventFilterOutputScheme() {
    fIsotopes.insert({77, 32}); // Germanium-77
}

void MyGe77EventFilterOutputScheme::TrackingActionPre(const G4Track* aTrack) {
    const auto particle = aTrack->GetParticleDefinition();
    if (!particle->IsGeneralIon()) return;

    int z = particle->GetAtomicNumber();
    int a = particle->GetAtomicMass();
    if (z == 32 && a == 77) {
        auto event = G4EventManager::GetEventManager()->GetNonconstCurrentEvent();
        if (event->GetUserInformation() == nullptr) {
            event->SetUserInformation(new MyGe77EventInformation());
            G4cout << "Germanium 77 ist entstanden" << G4endl;
        }
    }
}

// invoked in RMGEventAction::EndOfEventAction()
bool MyGe77EventFilterOutputScheme::ShouldDiscardEvent(const G4Event* event) {       
    // exit fast if no threshold is configured.
    return false;
}

std::optional<G4ClassificationOfNewTrack> MyGe77EventFilterOutputScheme::
    StackingActionClassify(const G4Track* aTrack, int stage) {
    // we are only interested in stacking optical photons into stage 1 after stage 0 finished.
    if (stage != 0) return std::nullopt;

    // defer tracking of optical photons.
    if (fDiscardPhotonsIfIsotopeNotProduced &&
        aTrack->GetDefinition() == G4OpticalPhoton::OpticalPhotonDefinition())
        return fWaiting;
    return std::nullopt;
}

std::optional<bool> MyGe77EventFilterOutputScheme::StackingActionNewStage(const int stage) {
    // we are only interested in stacking optical photons into stage 1 after stage 0 finished.
    if (stage != 0) return std::nullopt;
    // if we do not want to discard any photons ourselves, let other output schemes decide (i.e. not
    // force `true` on them).
    if (!fDiscardPhotonsIfIsotopeNotProduced) return std::nullopt;

    const auto event = G4EventManager::GetEventManager()->GetConstCurrentEvent();
    // discard all waiting events, if there were none of the requested isotopes produced.
    return ShouldDiscardEvent(event) ? std::make_optional(false) : std::nullopt;
}

void MyGe77EventFilterOutputScheme::DefineCommands() {

}

// vim: tabstop=2 shiftwidth=2 expandtab
