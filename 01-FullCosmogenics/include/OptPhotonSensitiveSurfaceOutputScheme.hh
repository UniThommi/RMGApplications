#ifndef _OPT_PHOTON_SENSITIVE_SURFACE_OUTPUT_SCHEME_HH_
#define _OPT_PHOTON_SENSITIVE_SURFACE_OUTPUT_SCHEME_HH_

#include <optional>
#include <set>
#include <string>

#include "G4AnalysisManager.hh"
#include "G4GenericMessenger.hh"
#include "G4ThreeVector.hh"
#include "G4Run.hh"
#include "RMGVOutputScheme.hh"

#include "MyEventAction.hh"



class G4Event;
class OptHitsSensitiveSurfaceOutputScheme : public RMGVOutputScheme {

  public:

    OptHitsSensitiveSurfaceOutputScheme();
    ~OptHitsSensitiveSurfaceOutputScheme();
    

    void ClearBeforeEvent() override;
    void AssignOutputNames(G4AnalysisManager* ana_man) override;
    void StoreEvent(const G4Event*) override;
    void TrackingActionPre(const G4Track* aTrack) override;

    [[nodiscard]] inline bool StoreAlways() const override { return true; }

  protected:

    [[nodiscard]] inline std::string GetNtuplenameFlat() const override { return "optPhotons"; }

  private:
    
    // RMGVOutputScheme Variablen.
    std::unique_ptr<G4GenericMessenger> fMessenger;
    void DefineCommands();
    MyEventAction* fEventAction = nullptr;

    // Output Register:
    G4int optPhotonsRegister = 12130;
    G4int physVolRegister = 12131;
    G4int materialRegister = 12132;

    // Mappings: Physisches Volumen und Material
    std::map<std::string, int> physVolumeMapping;
    std::map<std::string, int> materialMapping;

};

#endif
