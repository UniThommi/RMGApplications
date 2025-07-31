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



class G4Event;
class MyNeutronCaptureOutputScheme : public RMGVOutputScheme {

  public:

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

    // Saved NCs
    std::set<std::pair<G4int, G4int>> eventTrackPairs;

    // Output Register:
    G4int NCsRegisterID = 12120;
    G4int physVolRegister = 12121;
    G4int materialRegister = 12122;

    // Mappings: Physisches Volumen und Material
    std::map<std::string, int> physVolumeMapping;
    std::map<std::string, int> materialMapping;

    // Neutron Parameter
    std::vector<G4int> nCTrackIDs;
    std::vector<G4ThreeVector> nCPositions;
    std::vector<G4String> nCPhysVolumes;
    std::vector<G4String> nCMaterials;
    std::vector<G4double> nCGlobTimes;
    std::vector<G4double> nCGammaTotalEnergies;
    std::vector<G4int> nCGammaAmounts;
    std::vector<G4bool> nCfGe77s; // True -> Ge77 wurde produziert.
};

#endif
