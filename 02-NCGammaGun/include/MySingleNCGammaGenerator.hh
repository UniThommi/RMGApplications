#ifndef _MY_SINGLE_NC_GAMMA_GENERATOR_HH_
#define _MY_SINGLE_NC_GAMMA_GENERATOR_HH_

#include "RMGVGenerator.hh"
#include "G4ParticleGun.hh"
#include "G4GenericMessenger.hh"
#include "H5Cpp.h"
#include <memory>
#include <vector>

class CustomNCGammaSingleGenerator : public RMGVGenerator {
public:
    CustomNCGammaSingleGenerator();
    ~CustomNCGammaSingleGenerator();

    void BeginOfRunAction(const G4Run*) override;
    void GeneratePrimaries(G4Event* event) override;

    void SetNCFile(G4String pathToFile);
    void SetParticlePosition(G4ThreeVector) override { /* Not used */ }

private:
    void DefineCommands();
    void LoadData();
    void ValidateEventCount(G4int requestedEvents);
    
    template<typename T>
    std::vector<T> ReadDataset(H5::H5File& file, const std::string& dataset_path);

    std::unique_ptr<G4ParticleGun> fGun;
    std::unique_ptr<G4GenericMessenger> fMessenger;
    G4String fInputFile;

    // Metadata
    G4int fTotalNCs;
    G4int fCurrentEventID;

    // NC data
    std::vector<int> nc_evtid;
    std::vector<int> nc_id;
    std::vector<double> nc_x, nc_y, nc_z;
    std::vector<double> nc_time;
    
    // Gamma data
    std::vector<int> gamma_evtid;
    std::vector<int> gamma_nc_id;
    std::vector<int> gamma_id;
    std::vector<double> gamma_px, gamma_py, gamma_pz;
    std::vector<double> gamma_E;
    std::vector<double> gamma_pol_x, gamma_pol_y, gamma_pol_z;
    
    // Lookup: nc_id → indices in gamma arrays
    std::map<int, std::vector<size_t>> nc_to_gamma_indices;
};

#endif