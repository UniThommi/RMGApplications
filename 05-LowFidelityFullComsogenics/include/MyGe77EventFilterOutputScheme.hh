#ifndef _MY_GE_77_EVENT_FILTER_OUTPUT_SCHEME_HH_
#define _MY_GE_77_EVENT_FILTER_OUTPUT_SCHEME_HH_

#include <optional>
#include <set>

#include "G4AnalysisManager.hh"
#include "G4GenericMessenger.hh"
#include "G4VUserEventInformation.hh"

#include "RMGVOutputScheme.hh"

class MyGe77EventInformation : public G4VUserEventInformation {

  public:

    MyGe77EventInformation() = default;
    inline void Print() const override {};
};

class G4Event;
class MyGe77EventFilterOutputScheme : public RMGVOutputScheme {

  public:

    MyGe77EventFilterOutputScheme();

    bool ShouldDiscardEvent(const G4Event*) override;
    std::optional<bool> StackingActionNewStage(int) override;
    std::optional<G4ClassificationOfNewTrack> StackingActionClassify(const G4Track*, int) override;
    void TrackingActionPre(const G4Track* aTrack) override;

  private:
    std::set<std::pair<int, int>> fIsotopes;
    bool fDiscardPhotonsIfIsotopeNotProduced = false;
    void DefineCommands();
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab
