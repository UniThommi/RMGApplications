#ifndef _MY_GAMMA_CAPTURE_OUTPUT_SCHEME_HH_
#define _MY_GAMMA_CAPTURE_OUTPUT_SCHEME_HH_

#include "RMGVOutputScheme.hh"
#include "G4ThreeVector.hh"
#include <vector>
#include <tuple>

// remage output scheme that writes every gamma of every primary neutron capture to the
// ntuple "CaptureGammas" (event ID, NC ID, gamma ID, direction, energy, polarisation).
// MyNeutronCaptureOutputScheme keeps only the four most energetic gammas per NC; this
// complete list is what merge_nc_data_for_gun.py passes on to 02-NCGammaGun, which
// re-shoots exactly these gammas.
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

    // Static method for MySteppingAction, which has no access to the output scheme
    // instance. The gamma is buffered until StoreEvent() of the same thread.
    static void AddPendingGamma(G4int ncID, G4int gammaID, const GammaInfo& gamma);

private:
    void DefineCommands();
    void ProcessPendingGammas();

    // Ntuple ID
    static constexpr int gammasRegisterID = 5001;

    // Data vectors
    std::vector<G4int> ncIDs;
    std::vector<G4int> gammaIDs;
    std::vector<G4ThreeVector> gammaMomenta;
    std::vector<G4double> gammaEnergies;
    std::vector<G4ThreeVector> gammaPolarizations;

    // Thread-local storage for pending gammas (one buffer per worker thread)
    static thread_local std::vector<std::tuple<G4int, G4int, GammaInfo>> fPendingGammas;
};

#endif