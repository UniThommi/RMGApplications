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
    };

    // Static method für MySteppingAction
    static void AddPendingNC(G4int ncID, const NCInfo& ncInfo);

    MyNeutronCaptureOutputScheme();
    ~MyNeutronCaptureOutputScheme();
    

    void ClearBeforeEvent() override;
    void AssignOutputNames(G4AnalysisManager* ana_man) override;
    void StoreEvent(const G4Event*) override;
    void TrackingActionPre(const G4Track* aTrack) override;

    [[nodiscard]] inline bool StoreAlways() const override { return true; }

  protected:

    [[nodiscard]] inline std::string GetNtuplenameFlat() const override { return "neutrons"; }

  private:
    
    // RMGVOutputScheme Variablen.
    std::unique_ptr<G4GenericMessenger> fMessenger;
    void DefineCommands();
    void ProcessPendingNCs();

    // Thread-local storage für pending NCs
    static thread_local std::vector<std::tuple<G4int, NCInfo>> fPendingNCs;

    // Saved NCs
    std::set<std::pair<G4int, G4int>> eventTrackPairs;

    // Output Register:
    G4int NCsRegisterID = 12120;
    G4int physVolRegister = 12121;
    G4int materialRegister = 12122;

    // Mappings: Physisches Volumen und Material
    std::map<std::string, int> physVolumeMapping;
    std::map<std::string, int> materialMapping;

    // Neutron Capture Daten
    std::vector<G4int> nCTrackIDs;
    std::vector<G4ThreeVector> nCPositions;
    std::vector<G4int> nCPhysVolumeIDs;
    std::vector<G4int> nCMaterialIDs;
    std::vector<G4double> nCGlobTimes;
    std::vector<G4double> nCGammaTotalEnergies;
    std::vector<G4int> nCGammaAmounts;
    std::vector<G4bool> nCfGe77s; // True -> Ge77 wurde produziert.

};

#endif
