// #include "MyPhotonHit.hh"
#include "MyTrackInfo.hh"
// #include "MyPhotonHitsCollection.hh"
#include "MySteppingAction.hh"

#include "RMGOpticalDetector.hh"
#include "RMGLog.hh"
#include "RMGManager.hh"
#include "RMGHardware.hh"

#include "G4HCofThisEvent.hh"
#include "G4OpticalPhoton.hh"
#include "G4SDManager.hh"
#include "G4ParticleDefinition.hh"
#include "G4ParticleTypes.hh"
#include "G4VProcess.hh"
#include "G4RunManager.hh"
#include "G4Neutron.hh"
#include "G4OpticalPhoton.hh"
#include "G4Step.hh"

MySteppingAction::MySteppingAction() {
        this->DefineCommands();
    }

// Destructor
MySteppingAction::~MySteppingAction() {}    


// Inheriting Primary ID and 1st Gen Neutron ID. For Muon track, the 1st Gen Neutron ID is set to -1.
// Also save Gamma data from Neutron captures.
void MySteppingAction::UserSteppingAction(const G4Step* step) {
    const G4Track* track = step->GetTrack();

    if (track->GetParentID() == 0) { // Primary Particle
        auto* info = new MyTrackInfo(
            -1,                                 // TrackID
            G4ThreeVector(-1., -1., -1.),       // nC Pos
            -1.,                                // nC Time
            "",                                 // nC Phys Vol
            "",                                 // nC Material
            -1,                                 // nC Gamma Amount
            -1.,                                // nC Gamma Total Energy
            false,                              // nC fGe77
            G4ThreeVector(-1., -1., -1.),       // Gamma Momentum Direction
            -1.                                 // Gamma Kinetic Energy
        );
        track->SetUserInformation(info);
    }

    // Muon is the primary particle
    const auto* userInfo = track->GetUserInformation();
    if (!userInfo)  {
        RMGLog::OutDev(RMGLog::error, "No user information for the track.");
        return;
    }

    const auto* trackInfo = dynamic_cast<const MyTrackInfo*>(userInfo);
    if (!trackInfo) {
        RMGLog::OutDev(RMGLog::error, "No valid track information in user info.");
        return;
    }
    
    G4StepPoint* postStepPoint = step->GetPostStepPoint();

    // Check if the process is neutron capture (nCapture)
    const std::vector<const G4Track*>* secondaries = step->GetSecondaryInCurrentStep();
    MyTrackInfo* nonConstTrackInfo = const_cast<MyTrackInfo*>(trackInfo);
    if (postStepPoint->GetProcessDefinedStep()->GetProcessName() == "nCapture") {
        // Ensure the captured particle is a neutron
        if (track->GetParticleDefinition() == G4Neutron::Definition()) {
            // G4cout << "Neutron capture detected" << G4endl;
            // Use const_cast to remove the const qualifier and modify the object
            const G4VPhysicalVolume* physicalVolume = track->GetVolume();
            G4String physVolumeName = "";
            G4String materialName = "";
            if (physicalVolume) {
                physVolumeName = physicalVolume->GetName();
                G4Material* material = physicalVolume->GetLogicalVolume()->GetMaterial();
                if (material) {
                    materialName = material->GetName();
                }
            }

            nonConstTrackInfo->SetnCTrackID(track->GetTrackID());
            nonConstTrackInfo->SetnCPos(track->GetVertexPosition());
            nonConstTrackInfo->SetnCTime(track->GetGlobalTime());
            nonConstTrackInfo->SetnCPhysVol(physVolumeName);
            nonConstTrackInfo->SetnCMaterial(materialName);

            G4int gammaCount = 0;
            G4double totalGammaEnergy = 0.0;

            // Flag set if Ge77 was produced.
            for (const auto& secTrack : *secondaries) {
                const auto particle = secTrack->GetParticleDefinition();
                if (particle->IsGeneralIon()) {
                    int z = particle->GetAtomicNumber();
                    int a = particle->GetAtomicMass();
                    if (z == 32 && a == 77) { // Ge77?
                        // Remove const qualifier to modify nCfGe77
                        nonConstTrackInfo->SetnCfGe77(true);
                        G4cout << "Ge-77 erzeugt! 🎉" << G4endl;
                    }
                }

                // Zähle erzeugte Gammas
                if (particle == G4Gamma::Definition()) {
                    gammaCount++;
                    totalGammaEnergy += secTrack->GetKineticEnergy();
                }
            }
            nonConstTrackInfo->SetnCGammaAmount(gammaCount);
            nonConstTrackInfo->SetnCGammaTotalEnergy(totalGammaEnergy);
        }           
    }

    // Handling und speichern von optischen Photonen
    auto particle = step->GetTrack()->GetDefinition();
    if (particle == G4OpticalPhoton::OpticalPhotonDefinition()) {
        // This is actually irrelevant as optical photons do not truly carry the energy deposited
        // This yields the photon wavelength (in energy units)
        if (step->GetTotalEnergyDeposit() != 0) {

            // Get the physical volume of the detection point (post step). A step starts
            // at PreStepPoint and ends at PostStepPoint. If a boundary is reached, the
            // PostStepPoint belongs logically to the next volume. As we write down the
            // hit when the photon reaches the boundary we need to check the
            // PostStepPoint here
            auto touchable = step->GetPostStepPoint()->GetTouchableHandle();
            const auto pv_name = touchable->GetVolume()->GetName();
            const auto pv_copynr = touchable->GetCopyNumber();

            // check if physical volume is registered as optical detector
            auto det_cons = RMGManager::Instance()->GetDetectorConstruction();
            try {
                auto d_type = det_cons->GetDetectorMetadata({pv_name, pv_copynr}).type;
                if (d_type != RMGHardware::kOptical) {
                RMGLog::OutFormatDev(RMGLog::debug,
                    "Volume '{}' (copy nr. {} not registered as optical detector", pv_name, pv_copynr);
                return;
                }
            } catch (const std::out_of_range& e) {
                RMGLog::OutFormatDev(RMGLog::debug, "Volume '{}' (copy nr. {} not registered as detector",
                    pv_name, pv_copynr);
                return;
            }

            // retrieve data and unique id for persistency
            G4int det_uid = det_cons->GetDetectorMetadata({pv_name, pv_copynr}).uid;

            RMGLog::OutDev(RMGLog::debug, "Hit in optical detector nr. ", det_uid, " detected");

            G4double photon_energy = step->GetTotalEnergyDeposit();
            G4double photon_global_time = step->GetPostStepPoint()->GetGlobalTime();
            G4ThreeVector photon_position = step->GetPostStepPoint()->GetPosition();
            G4ThreeVector photon_momentum_direction = step->GetPostStepPoint()->GetMomentumDirection();

            G4int nCTrackID = nonConstTrackInfo->GetnCTrackID();
            G4ThreeVector nCPos = nonConstTrackInfo->GetnCPos();
            G4double nCTime = nonConstTrackInfo->GetnCTime();
            G4String nCPhysVol = nonConstTrackInfo->GetnCPhysVol();
            G4String nCMaterial = nonConstTrackInfo->GetnCMaterial();
            G4int nCGammaAmount = nonConstTrackInfo->GetnCGammaAmount();
            G4double nCGammaTotalEnergy = nonConstTrackInfo->GetnCGammaTotalEnergy();
            G4bool nCfGe77 = nonConstTrackInfo->GetnCfGe77();
            G4ThreeVector gammaMomentumDirection = nonConstTrackInfo->GetGammaMomentumDirection();
            G4double gammaKineticEnergy = nonConstTrackInfo->GetGammaKineticEnergy();

            auto rmg_man = RMGManager::Instance();
            if (rmg_man->IsPersistencyEnabled()) { 
                RMGLog::OutDev(RMGLog::debug, "Filling persistent data vectors");
                const auto ana_man = G4AnalysisManager::Instance();
                auto optPhotonsNTuple = rmg_man->GetNtupleID(optPhotonsRegister); 
                auto physVolumesNTuple = rmg_man->GetNtupleID(physVolRegister);
                auto materialsNTuple = rmg_man->GetNtupleID(materialRegister);

                if (physVolumeMapping.find(physVolumeName) == physVolumeMapping.end()) {
                    const G4int physicalVolumeMappingID = physVolumeMapping.size();
                    physVolumeMapping.emplace(physVolumeName, physicalVolumeMappingID);
                    //Speichern         
                    int vol_col_id = 0;
                    ana_man->FillNtupleIColumn(physVolumesNTuple, vol_col_id++, physicalVolumeMappingID);
                    ana_man->FillNtupleSColumn(physVolumesNTuple, vol_col_id++, physVolumeName);
                    ana_man->AddNtupleRow(physVolumesNTuple);
                    
                }
                G4int physVolumeID = physVolumeMapping[physVolumeName];
                
            
                G4String materialName = hit->GetnCMaterial();
            
                if (materialMapping.find(materialName) == materialMapping.end()) {
                    const G4int materialMappingID = materialMapping.size();
                    materialMapping.emplace(materialName, materialMappingID);
                    // Speichern
                    int mat_col_id = 0;
                    ana_man->FillNtupleIColumn(materialsNTuple, mat_col_id++, materialMappingID);
                    ana_man->FillNtupleSColumn(materialsNTuple, mat_col_id++, materialName);
                    ana_man->AddNtupleRow(materialsNTuple);

                }
                G4int materialID = materialMapping[materialName];
            

            
                // -> Speicher die Infos raus (Position, Zeit, Energie, ...)
                int col_id = 0;
                // Output: Was Ge77 produced in this event?
                ana_man->FillNtupleIColumn(optPhotonsNTuple, col_id++, event->GetEventID());
                ana_man->FillNtupleIColumn(optPhotonsNTuple, col_id++, nCTrackID);
                ana_man->FillNtupleDColumn(optPhotonsNTuple, col_id++, nCTime/u::s);
                ana_man->FillNtupleDColumn(optPhotonsNTuple, col_id++, nCPos.getX()/u::m);
                ana_man->FillNtupleDColumn(optPhotonsNTuple, col_id++, nCPos.getY()/u::m);
                ana_man->FillNtupleDColumn(optPhotonsNTuple, col_id++, nCPos.getZ()/u::m); 
                ana_man->FillNtupleIColumn(optPhotonsNTuple, col_id++, physVolumeID);
                ana_man->FillNtupleIColumn(optPhotonsNTuple, col_id++, materialID);

                ana_man->FillNtupleIColumn(optPhotonsNTuple, col_id++, nCGammaAmount);
                ana_man->FillNtupleDColumn(optPhotonsNTuple, col_id++, nCGammaTotalEnergy/u::keV);
                ana_man->FillNtupleIColumn(optPhotonsNTuple, col_id++, nCfGe77);
                
                ana_man->FillNtupleDColumn(optPhotonsNTuple, col_id++, gammaMomentumDirection.getX());
                ana_man->FillNtupleDColumn(optPhotonsNTuple, col_id++, gammaMomentumDirection.getY());
                ana_man->FillNtupleDColumn(optPhotonsNTuple, col_id++, gammaMomentumDirection.getZ()); 
                ana_man->FillNtupleDColumn(optPhotonsNTuple, col_id++, gammaKineticEnergy/u::keV);

                ana_man->FillNtupleIColumn(optPhotonsNTuple, col_id++, det_uid);
                ana_man->FillNtupleDColumn(optPhotonsNTuple, col_id++, photon_energy/u::keV);
                ana_man->FillNtupleDColumn(optPhotonsNTuple, col_id++, photon_global_time/u::s);
                ana_man->FillNtupleDColumn(optPhotonsNTuple, col_id++, photon_position.getX()/u::m);
                ana_man->FillNtupleDColumn(optPhotonsNTuple, col_id++, photon_position.getY()/u::m);
                ana_man->FillNtupleDColumn(optPhotonsNTuple, col_id++, photon_position.getZ()/u::m);
                ana_man->FillNtupleDColumn(optPhotonsNTuple, col_id++, photon_momentum_direction.getX());
                ana_man->FillNtupleDColumn(optPhotonsNTuple, col_id++, photon_momentum_direction.getY());
                ana_man->FillNtupleDColumn(optPhotonsNTuple, col_id++, photon_momentum_direction.getZ());

                ana_man->AddNtupleRow(optPhotonsNTuple);
            }
        }  
    }


    // Inherit Track Info to secondary particles
    for (const auto& secTrack : *secondaries) {
        auto* inheritedInfo = new MyTrackInfo(
            nonConstTrackInfo->GetnCTrackID(),
            nonConstTrackInfo->GetnCPos(),
            nonConstTrackInfo->GetnCTime(),
            nonConstTrackInfo->GetnCPhysVol(),
            nonConstTrackInfo->GetnCMaterial(),
            nonConstTrackInfo->GetnCGammaAmount(),
            nonConstTrackInfo->GetnCGammaTotalEnergy(),
            nonConstTrackInfo->GetnCfGe77(),
            nonConstTrackInfo->GetGammaMomentumDirection(),
            nonConstTrackInfo->GetGammaKineticEnergy()
        );
        // Wenn das Secondary ein Gamma ist, speichere Energie & Impulsrichtung
        if (secTrack->GetParticleDefinition() == G4Gamma::Definition()) {
            inheritedInfo->SetGammaKineticEnergy(secTrack->GetKineticEnergy());
            inheritedInfo->SetGammaMomentumDirection(secTrack->GetMomentumDirection());
        };

        // Set user information for the secondary track
        const_cast<G4Track*>(secTrack)->SetUserInformation(inheritedInfo);
    }    
}


void MySteppingAction::DefineCommands() {
  
}
