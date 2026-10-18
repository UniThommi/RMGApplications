#ifndef _DEBUG_VERTEX_OUTPUT_SCHEME_HH_
#define _DEBUG_VERTEX_OUTPUT_SCHEME_HH_

#include <vector>
#include "G4AnalysisManager.hh"
#include "G4GenericMessenger.hh"
#include "RMGVOutputScheme.hh"

class G4Event;

// Writes the label (muon ID, NC ID, gamma ID) of every primary vertex to the ntuple
// "DebugVertices", one row per vertex. The consistency notebooks use it to check the
// event structure of the generators (one NC or one muon per event, complete gamma IDs,
// same vertex count as the remage vertex output).
class DebugVertexOutputScheme : public RMGVOutputScheme {
  public:
    DebugVertexOutputScheme();

    void ClearBeforeEvent() override;
    void AssignOutputNames(G4AnalysisManager* ana_man) override;
    void StoreEvent(const G4Event*) override;
    [[nodiscard]] inline bool StoreAlways() const override { return true; }

  protected:
    [[nodiscard]] inline std::string GetNtuplenameFlat() const override { return "DebugVertices"; }

  private:
    void DefineCommands();
    G4int OutputRegisterID = 13122;

    // Data per vertex
    std::vector<G4int> vertexMuonIDs;
    std::vector<G4int> vertexNCIDs;
    std::vector<G4int> vertexGammaIDs;
};

#endif