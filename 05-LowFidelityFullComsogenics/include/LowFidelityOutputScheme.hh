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
    G4int OutputRegisterID = 13121;


    // Neutron Capture Info
    G4int nCNeutronID;
    G4double nCGlobalTime;
    G4int nCPhysVolumeID;
    G4int nCMaterialID;
    G4int nCfGe77; // Ist in dem Neutronschauer, in dem das Primary Neutron produziert wurde, Ge77 entstanden?
    G4int nCGammaAmount;
    G4double nCGammaTotalEnergy;

    // PMT Info
    std::vector<G4int> hitPMTUID;
    std::vector<G4double> hitTimes;
    // std::vector<G4double> hitxPositions;
    // std::vector<G4double> hityPositions;
    // std::vector<G4double> hitzPositions;
    std::vector<G4double> hitWaveLengths;
    // G4int fCaptureDetected;
};

#endif
