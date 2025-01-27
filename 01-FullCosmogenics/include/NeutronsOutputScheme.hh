#ifndef _NEUTRONS_OUTPUT_SCHEME_HH_
#define _NEUTRONS_OUTPUT_SCHEME_HH_

#include <optional>
#include <set>

#include "G4AnalysisManager.hh"
#include "G4GenericMessenger.hh"
#include "G4ThreeVector.hh"

#include "RMGVOutputScheme.hh"

class G4Event;
class NeutronsOutputScheme : public RMGVOutputScheme {

  public:

    NeutronsOutputScheme();

    void ClearBeforeEvent() override;
    void AssignOutputNames(G4AnalysisManager* ana_man) override;
    void StoreEvent(const G4Event*) override;
    bool ShouldDiscardEvent(const G4Event*) override;
    void TrackingActionPre(const G4Track* aTrack) override;

  protected:

    [[nodiscard]] inline std::string GetNtuplenameFlat() const override { return "neutrons"; }

  private:
    std::unique_ptr<G4GenericMessenger> fMessenger;
    void DefineCommands();

    G4int OutputRegisterID = 12120;

    std::vector<G4ThreeVector> Capture_Positions;
    std::vector<G4int> zOfEvent;
    std::vector<G4int> aOfEvent;
};

#endif
