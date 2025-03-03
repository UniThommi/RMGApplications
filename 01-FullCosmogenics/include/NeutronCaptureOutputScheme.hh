#ifndef _NEUTRON_CAPTURE_OUTPUT_SCHEME_HH_
#define _NEUTRON_CAPTURE_OUTPUT_SCHEME_HH_

#include <optional>
#include <set>
#include <string>

#include "G4AnalysisManager.hh"
#include "G4GenericMessenger.hh"
#include "G4ThreeVector.hh"
#include "G4Run.hh"
#include "RMGVOutputScheme.hh"



class G4Event;
class NeutronCaptureOutputScheme : public RMGVOutputScheme {

  public:

    NeutronCaptureOutputScheme();
    ~NeutronCaptureOutputScheme();
    

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

    // Output Register:
    G4int neutronsRegister = 12120;
    G4int physVolRegister = 12121;
    G4int materialRegister = 12122;

    // Mappings: Physisches Volumen und Material
    std::map<std::string, int> physVolumeMapping;
    std::map<std::string, int> materialMapping;

    // Neutron Parameter
    std::vector<G4ThreeVector> gammaPositions;
    std::vector<G4ThreeVector> gammaMomentumDirections;
    std::vector<G4double> globalTimes;
    std::vector<G4double> gammaKinEnergies;
    std::vector<G4int> nCNeutronID;
    std::vector<G4int> nCPhysicalVolumes;
    std::vector<G4int> nCMaterials;
    std::vector<G4bool> fGe77; // True -> Ge77 wurde produziert.


    // Mapping Vektoren
    std::vector<G4int> physicalVolumeMappingIDs;
    std::vector<G4int> materialMappingIDs;

    std::vector<G4String> physicalVolumeMappingNames;
    std::vector<G4String> materialMappingNames;

    bool fPhysVolumeMapping;
    bool fMaterialMapping;
};

#endif
