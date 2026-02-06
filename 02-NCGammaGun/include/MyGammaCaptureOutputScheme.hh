#ifndef _MY_GAMMA_CAPTURE_OUTPUT_SCHEME_HH_
#define _MY_GAMMA_CAPTURE_OUTPUT_SCHEME_HH_

#include "RMGVOutputScheme.hh"
#include "G4ThreeVector.hh"
#include <vector>
#include <tuple>

class MyGammaCaptureOutputScheme : public RMGVOutputScheme {
public:
    struct GammaInfo {
        G4ThreeVector dir;
        G4double energy;
        G4ThreeVector polarization;
    };

    MyGammaCaptureOutputScheme();
    ~MyGammaCaptureOutputScheme();

    void AssignOutputNames(G4AnalysisManager* ana_man) override;
    void StoreEvent(const G4Event* event) override;
    void ClearBeforeEvent() override;

    // Static method für MySteppingAction
    static void AddPendingGamma(G4int ncID, G4int gammaID, const GammaInfo& gamma);

private:
    void DefineCommands();
    void ProcessPendingGammas();

    // Ntuple ID
    static constexpr int gammasRegisterID = 5001;

    // Daten-Vektoren
    std::vector<G4int> ncIDs;
    std::vector<G4int> gammaIDs;
    std::vector<G4ThreeVector> gammaMomenta;
    std::vector<G4double> gammaEnergies;
    std::vector<G4ThreeVector> gammaPolarizations;

    // Thread-local storage für pending Gammas  
    static thread_local std::vector<std::tuple<G4int, G4int, GammaInfo>> fPendingGammas;
};

#endif