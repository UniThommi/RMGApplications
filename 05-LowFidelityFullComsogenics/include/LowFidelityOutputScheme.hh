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
class LowFidelityOutputScheme : public RMGVOutputScheme {

  public:

    LowFidelityOutputScheme();
    

    void ClearBeforeEvent() override;
    void AssignOutputNames(G4AnalysisManager* ana_man) override;
    void StoreEvent(const G4Event*) override;

  protected:

    [[nodiscard]] inline std::string GetNtuplenameFlat() const override { return "PMTEvents"; }

  private:
    
    // RMGVOutputScheme Variablen.
    std::unique_ptr<G4GenericMessenger> fMessenger;
    void DefineCommands();
    G4int OutputRegisterID = 12121;


    // Neutron Parameter
    std::vector<G4int> hitPMTUID;
    std::vector<G4double> hitTimes;
    std::vector<G4double> hitWaveLengths;
    //std::vector<G4double> hitEnergieDepositions;
    <G4int> neutronPhysicalVolume;
    <G4int> neutronMaterial;
    <G4int> fNeutronGe77; // Ist in dem Neutronschauer, in dem das Primary Neutron produziert wurde, Ge77 entstanden?
};

#endif
