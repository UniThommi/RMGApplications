#ifndef _NEUTRONS_OUTPUT_SCHEME_HH_
#define _NEUTRONS_OUTPUT_SCHEME_HH_

#include <optional>
#include <set>
#include <string>

#include "G4AnalysisManager.hh"
#include "G4GenericMessenger.hh"
#include "G4ThreeVector.hh"
#include "G4Run.hh"
#include "RMGVOutputScheme.hh"

#include "MyRunMappingAction.hh"


class G4Event;
class NeutronsOutputScheme : public RMGVOutputScheme {

  public:

    NeutronsOutputScheme();
    

    void ClearBeforeEvent() override;
    void AssignOutputNames(G4AnalysisManager* ana_man) override;
    void StoreEvent(const G4Event*) override;
    void TrackingActionPre(const G4Track* aTrack) override;

  protected:

    [[nodiscard]] inline std::string GetNtuplenameFlat() const override { return "neutrons"; }

  private:
    
    // RMGVOutputScheme Variablen.
    std::unique_ptr<G4GenericMessenger> fMessenger;
    void DefineCommands();
    G4int OutputRegisterID = 12120;

    // Ge77 Flag für Event (1 Muon):
    G4bool fGe77Produced = false; // True -> Ge77 wurde produziert.

    // Neutron Parameter
    std::vector<G4ThreeVector> vertexPositions;
    std::vector<G4ThreeVector> vertexMomentums;
    std::vector<G4double> globalTimes;
    std::vector<G4double> vertexKineticEnergies;
    std::vector<G4int> physicalVolumes;
    std::vector<G4int> materials;
};

#endif
