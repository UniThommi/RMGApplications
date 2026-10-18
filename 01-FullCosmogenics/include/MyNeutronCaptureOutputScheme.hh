#ifndef _MY_NEUTRON_CAPTURE_OUTPUT_SCHEME_HH_
#define _MY_NEUTRON_CAPTURE_OUTPUT_SCHEME_HH_

#include <optional>
#include <set>
#include <string>
#include <utility>

#include "G4AnalysisManager.hh"
#include "G4GenericMessenger.hh"
#include "G4ThreeVector.hh"
#include "G4Run.hh"
#include "RMGVOutputScheme.hh"
#include <tuple>


class G4Event;
// remage output scheme that writes one row per primary neutron capture (NC) to the
// ntuple "MyNeutronCaptureOutput": capture position, time, volume, material, gamma
// multiplicity and energy, the 77Ge flag and the four most energetic gammas. Two
// further ntuples ("physVolumes", "materials") map the integer IDs to names; the IDs
// are assigned in order of first appearance by each thread and are therefore only
// valid within one output file. This output is the NC truth used by
// merge_nc_data_for_gun.py and simPostProcessing.
class MyNeutronCaptureOutputScheme : public RMGVOutputScheme {
public:
    struct NCInfo {
        G4ThreeVector pos;
        G4double time;
        G4String physVol;
        G4String material;
        G4int gammaAmount;
        G4double gammaTotalEnergy;
        G4bool fGe77;
        G4ThreeVector gamma1_dir, gamma2_dir, gamma3_dir, gamma4_dir;
        G4double gamma1_E, gamma2_E, gamma3_E, gamma4_E;
    };

    // Static method for MySteppingAction, which has no access to the output scheme
    // instance. The NC is buffered until StoreEvent() of the same thread.
    static void AddPendingNC(G4int ncID, const NCInfo& ncInfo);

    MyNeutronCaptureOutputScheme();
    ~MyNeutronCaptureOutputScheme();


    void ClearBeforeEvent() override;
    void AssignOutputNames(G4AnalysisManager* ana_man) override;
    void StoreEvent(const G4Event*) override;
    // BEGIN UNUSED CODE — no-op override of an empty base-class hook
    void TrackingActionPre(const G4Track* aTrack) override;
    // END UNUSED CODE

    // Store the NCs even if another output scheme discards the event.
    [[nodiscard]] inline bool StoreAlways() const override { return true; }

  protected:

    [[nodiscard]] inline std::string GetNtuplenameFlat() const override { return "neutrons"; }

  private:

    // RMGVOutputScheme variables.
    // BEGIN UNUSED CODE — fMessenger is never created (DefineCommands() is empty)
    std::unique_ptr<G4GenericMessenger> fMessenger;
    // END UNUSED CODE
    void DefineCommands();
    void ProcessPendingNCs();

    // Thread-local storage for pending NCs: each worker thread processes its own
    // events, so the buffer must not be shared between threads.
    static thread_local std::vector<std::tuple<G4int, NCInfo>> fPendingNCs;

    // BEGIN UNUSED CODE — eventTrackPairs is never filled or read
    // Saved NCs
    std::set<std::pair<G4int, G4int>> eventTrackPairs;
    // END UNUSED CODE

    // Output register:
    G4int NCsRegisterID = 12120;
    G4int physVolRegister = 12121;
    G4int materialRegister = 12122;

    // Mappings: physical volume and material
    std::map<std::string, int> physVolumeMapping;
    std::map<std::string, int> materialMapping;

    // Neutron capture data
    std::vector<G4int> nCTrackIDs;
    std::vector<G4ThreeVector> nCPositions;
    std::vector<G4int> nCPhysVolumeIDs;
    std::vector<G4int> nCMaterialIDs;
    std::vector<G4double> nCGlobTimes;
    std::vector<G4double> nCGammaTotalEnergies;
    std::vector<G4int> nCGammaAmounts;
    std::vector<G4bool> nCfGe77s; // True -> Ge77 was produced.
    std::vector<G4ThreeVector> gammaDirs1, gammaDirs2, gammaDirs3, gammaDirs4;
    std::vector<G4double> gammaEs1, gammaEs2, gammaEs3, gammaEs4;

};

#endif
