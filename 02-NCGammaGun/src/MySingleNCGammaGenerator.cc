#include "MySingleNCGammaGenerator.hh"
#include "MyTrackInfo.hh"
#include "G4Run.hh"
#include "G4Event.hh"
#include "G4ParticleTable.hh"
#include "G4PrimaryVertex.hh"
#include "G4PrimaryParticle.hh"
#include "G4RunManager.hh"
#include "H5Cpp.h"
#include <iostream>

#ifndef H5_NO_NAMESPACE
using namespace H5;
#endif

namespace u = CLHEP;

template<typename T>
std::vector<T> CustomNCGammaSingleGenerator::ReadDataset(H5::H5File& file, const std::string& dataset_path) {
    DataSet dataset = file.openDataSet(dataset_path);
    DataSpace dataspace = dataset.getSpace();
    hsize_t dims[1];
    dataspace.getSimpleExtentDims(dims, nullptr);
    std::vector<T> data(dims[0]);
    dataset.read(data.data(), dataset.getDataType());
    return data;
}

CustomNCGammaSingleGenerator::CustomNCGammaSingleGenerator()
    : RMGVGenerator("NCGammas_Single"), fTotalNCs(0), fCurrentEventID(0) {
    this->DefineCommands();
    fGun = std::make_unique<G4ParticleGun>();
}

CustomNCGammaSingleGenerator::~CustomNCGammaSingleGenerator() {}

void CustomNCGammaSingleGenerator::BeginOfRunAction(const G4Run* run) {
    LoadData();
    
    // Validate requested event count
    G4int requestedEvents = run->GetNumberOfEventToBeProcessed();
    ValidateEventCount(requestedEvents);
}

void CustomNCGammaSingleGenerator::LoadData() {
    G4cout << "=== Loading NC Data from " << fInputFile << " ===" << G4endl;
    
    try {
        H5File file(fInputFile, H5F_ACC_RDONLY);
        
        // Read metadata
        H5::Attribute attr = file.openAttribute("total_ncs");
        attr.read(H5::PredType::NATIVE_INT, &fTotalNCs);
        G4cout << "Total NCs in file: " << fTotalNCs << G4endl;
        
        // Read NC data
        G4cout << "Loading NC data..." << G4endl;
        nc_evtid = ReadDataset<int>(file, "/ncs/evtid");
        nc_id = ReadDataset<int>(file, "/ncs/nc_id");
        nc_x = ReadDataset<double>(file, "/ncs/nc_x");
        nc_y = ReadDataset<double>(file, "/ncs/nc_y");
        nc_z = ReadDataset<double>(file, "/ncs/nc_z");
        nc_time = ReadDataset<double>(file, "/ncs/nc_time");
        
        // Read Gamma data
        G4cout << "Loading Gamma data..." << G4endl;
        gamma_evtid = ReadDataset<int>(file, "/gammas/evtid");
        gamma_nc_id = ReadDataset<int>(file, "/gammas/nc_id");
        gamma_id = ReadDataset<int>(file, "/gammas/gamma_id");
        gamma_px = ReadDataset<double>(file, "/gammas/gamma_px");
        gamma_py = ReadDataset<double>(file, "/gammas/gamma_py");
        gamma_pz = ReadDataset<double>(file, "/gammas/gamma_pz");
        gamma_E = ReadDataset<double>(file, "/gammas/gamma_E");
        gamma_pol_x = ReadDataset<double>(file, "/gammas/gamma_pol_x");
        gamma_pol_y = ReadDataset<double>(file, "/gammas/gamma_pol_y");
        gamma_pol_z = ReadDataset<double>(file, "/gammas/gamma_pol_z");
        
        file.close();
        
        // Build lookup map: nc_id → gamma indices
        G4cout << "Building NC → Gamma lookup table..." << G4endl;
        for (size_t i = 0; i < gamma_nc_id.size(); ++i) {
            nc_to_gamma_indices[gamma_nc_id[i]].push_back(i);
        }
        
        G4cout << "✅ Loaded " << nc_id.size() << " NCs" << G4endl;
        G4cout << "✅ Loaded " << gamma_id.size() << " Gammas" << G4endl;
        G4cout << "✅ Built lookup for " << nc_to_gamma_indices.size() << " unique NCs" << G4endl;
        
    } catch (H5::Exception& error) {
        error.printErrorStack();
        G4Exception("CustomNCGammaSingleGenerator::LoadData", "HDF5Error", 
                    FatalException, "Failed to load NC data file");
    }
}

void CustomNCGammaSingleGenerator::ValidateEventCount(G4int requestedEvents) {
    if (requestedEvents > fTotalNCs) {
        std::ostringstream msg;
        msg << "Requested " << requestedEvents << " events but only " 
            << fTotalNCs << " NCs available in file!";
        G4Exception("CustomNCGammaSingleGenerator::ValidateEventCount", 
                    "InsufficientData", FatalException, msg.str().c_str());
    }
    G4cout << "✅ Validation passed: " << requestedEvents << " <= " << fTotalNCs << G4endl;
}

void CustomNCGammaSingleGenerator::GeneratePrimaries(G4Event* event) {
    G4int eventID = event->GetEventID();
    
    if (eventID >= static_cast<G4int>(nc_id.size())) {
        G4cerr << "❌ ERROR: EventID " << eventID << " exceeds NC count!" << G4endl;
        return;
    }
    
    // Get NC data for this event
    int muon_evtid = nc_evtid[eventID];
    int ncID = nc_id[eventID];
    G4ThreeVector ncPosition(nc_x[eventID] * u::m, nc_y[eventID] * u::m, nc_z[eventID] * u::m);
    G4double ncTime = nc_time[eventID] * u::ns;
    
    // Create vertex at NC position and time
    G4PrimaryVertex* vertex = new G4PrimaryVertex(ncPosition, ncTime);
    
    // Find all gammas for this NC
    auto it = nc_to_gamma_indices.find(ncID);
    if (it == nc_to_gamma_indices.end()) {
        G4cout << "⚠️  Warning: No gammas found for NC " << ncID << G4endl;
        event->AddPrimaryVertex(vertex);
        return;
    }
    
    const auto& gamma_indices = it->second;
    
    // Get particle definition
    G4ParticleTable* particleTable = G4ParticleTable::GetParticleTable();
    G4ParticleDefinition* gamma = particleTable->FindParticle("gamma");
    
    // Create gamma particles
    for (size_t idx : gamma_indices) {
        G4ThreeVector momentum(gamma_px[idx], gamma_py[idx], gamma_pz[idx]);
        G4ThreeVector polarization(gamma_pol_x[idx], gamma_pol_y[idx], gamma_pol_z[idx]);
        G4double energy = gamma_E[idx] * u::keV;
        int gammaID = gamma_id[idx];
        
        G4PrimaryParticle* particle = new G4PrimaryParticle(gamma);
        particle->SetMomentumDirection(momentum);
        particle->SetKineticEnergy(energy);
        particle->SetPolarization(polarization);
        
        // Set UserInfo (muon_evtid, ncID, gammaID)
        auto* userInfo = new MyTrackInfo(ncID, gammaID);
        // Note: PrimaryParticle doesn't support UserInfo directly
        // We'll set it in the tracking action
        
        vertex->SetPrimary(particle);
    }
    
    event->AddPrimaryVertex(vertex);
    
    if (eventID % 1000 == 0) {
        G4cout << "Event " << eventID << ": NC " << ncID 
               << " with " << gamma_indices.size() << " gammas" << G4endl;
    }
}

void CustomNCGammaSingleGenerator::SetNCFile(G4String pathToFile) {
    fInputFile = pathToFile;
}

void CustomNCGammaSingleGenerator::DefineCommands() {
    fMessenger = std::make_unique<G4GenericMessenger>(
        this, "/Cosmogenics/Generator/",
        "Commands for NC Gamma Single Generator");
    
    fMessenger->DeclareMethod("SetNCFile", &CustomNCGammaSingleGenerator::SetNCFile)
        .SetGuidance("Set the merged NC+Gamma input HDF5 file")
        .SetParameterName("pathToFile", false)
        .SetToBeBroadcasted(true)
        .SetStates(G4State_PreInit, G4State_Idle);
}

// vim: tabstop=2 shiftwidth=2 expandtab