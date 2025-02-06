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
    void TrackingActionPre(const G4Track* aTrack) override;

  protected:

    [[nodiscard]] inline std::string GetNtuplenameFlat() const override { return "neutrons"; }

  private:

    std::unique_ptr<G4GenericMessenger> fMessenger;
    void DefineCommands();
    void InsertGe77Info();

    // Vektor an killbaren Tracks, da Ge77 bereits produziert wurde.
    std::vector<G4int> killableIDs;
    
    G4int OutputRegisterID = 12120;

    std::vector<G4ThreeVector> vertexPositions;
    std::vector<G4ThreeVector> vertexMomentums;
    std::vector<G4double> globalTimes;
    std::vector<G4double> vertexKineticEnergies;
    std::vector<G4int> volumes;
    std::vector<G4string> materials;
    std::vector<G4int> primaryTrackId; // Damit man sieht welche Neutronen 1. Generation zum gleichen Muon gehören.
    std::vector<G4int> gen1NeutronID; // Damit man sieht welches das Neutron 1. Generation ist und welche weiteren Reaktionen zu diesem Neutron gehören -> Man kann Ge77 PRoduktion zuweisen. Entsteht kein Neutron ist der Wert auf -1 (wird dann aber auch nicht gespeichert).   
    std::vector<G4bool> fGe77Produced; // True -> Ge77 wurde produziert.
};

#endif
