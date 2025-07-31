#include "MyTrackInfo.hh"
#include "MySteppingAction.hh"

#include "RMGOpticalDetector.hh"
#include "RMGLog.hh"
#include "RMGManager.hh"
#include "RMGHardware.hh"

#include "G4OpticalPhoton.hh"
#include "G4SDManager.hh"
#include "G4ParticleDefinition.hh"
#include "G4ParticleTypes.hh"
#include "G4VProcess.hh"
#include "G4RunManager.hh"
#include "G4Neutron.hh"
#include "G4OpticalPhoton.hh"
#include "G4Step.hh"
#include "G4EventManager.hh"

namespace u = CLHEP;

MySteppingAction::MySteppingAction() {
        this->DefineCommands();
    }

// Destructor
MySteppingAction::~MySteppingAction() {}    


// Inheriting Primary ID and 1st Gen Neutron ID. For Muon track, the 1st Gen Neutron ID is set to -1.
// Also save Gamma data from Neutron captures.
void MySteppingAction::UserSteppingAction(const G4Step* step) {
    const G4Track* track = step->GetTrack();
    G4StepPoint* postStepPoint = step->GetPostStepPoint();

    // Check if the process is neutron capture (nCapture)
    const std::vector<const G4Track*>* secondaries = step->GetSecondaryInCurrentStep();
    if (postStepPoint->GetProcessDefinedStep()->GetProcessName() == "nCapture") {
    // Ensure the captured particle is a neutron
        if (track->GetParticleDefinition() == G4Neutron::Definition()) {

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

            G4int gammaCount = 0;
            G4double totalGammaEnergy = 0.0;
            G4bool fGe77 = false;

            struct GammaInfo {
                G4ThreeVector dir;
                G4double energy;
            };
            std::vector<GammaInfo> gammas;

            // Flag set if Ge77 was produced.
            for (const auto& secTrack : *secondaries) {
                const auto particle = secTrack->GetParticleDefinition();
                if (particle->IsGeneralIon()) {
                    int z = particle->GetAtomicNumber();
                    int a = particle->GetAtomicMass();
                    if (z == 32 && a == 77) { // Ge77?
                        // Remove const qualifier to modify nCfGe77
                        fGe77 = true;
                        G4cout << "Ge-77 erzeugt! 🎉" << G4endl;
                    }
                }

                // Zähle erzeugte Gammas
                if (particle == G4Gamma::Definition()) {
                    gammaCount++;
                    totalGammaEnergy += secTrack->GetKineticEnergy();
                    gammas.push_back({secTrack->GetMomentumDirection(), secTrack->GetKineticEnergy()});
                }
            }

            std::sort(gammas.begin(), gammas.end(), [](const GammaInfo& a, const GammaInfo& b) {
                return a.energy > b.energy;
            });

            while (gammas.size() < 4) {
                gammas.push_back({G4ThreeVector(0., 0., 0.), 0.});
            }

            const auto* userInfo = track->GetUserInformation();
            const auto* trackInfo = dynamic_cast<const MyTrackInfo*>(userInfo);
            if (!trackInfo) {
                auto* info = new MyTrackInfo(
                    track->GetTrackID(),                        // TrackID
                    track->GetVertexPosition(),                 // nC Pos
                    track->GetGlobalTime(),                     // nC Time
                    physVolumeName,                             // nC Phys Vol
                    materialName,                               // nC Material
                    gammaCount,                                 // nC Gamma Amount
                    totalGammaEnergy,                           // nC Gamma Total Energy
                    fGe77,                                      // nC fGe77
                    gammas[0].dir, gammas[0].energy,
                    gammas[1].dir, gammas[1].energy,
                    gammas[2].dir, gammas[2].energy,
                    gammas[3].dir, gammas[3].energy       
                );
                track->SetUserInformation(info);
            }
            else {
                MyTrackInfo* nonConstTrackInfo = const_cast<MyTrackInfo*>(trackInfo);
                nonConstTrackInfo->SetnCTrackID(track->GetTrackID());
                nonConstTrackInfo->SetnCPos(track->GetVertexPosition());
                nonConstTrackInfo->SetnCTime(track->GetGlobalTime());
                nonConstTrackInfo->SetnCPhysVol(physVolumeName);
                nonConstTrackInfo->SetnCMaterial(materialName);
                nonConstTrackInfo->SetnCGammaAmount(gammaCount);
                nonConstTrackInfo->SetnCGammaTotalEnergy(totalGammaEnergy);
                nonConstTrackInfo->SetnCfGe77(fGe77);
                nonConstTrackInfo->SetGammaMomentumDirection(0, gammas[0].dir);
                nonConstTrackInfo->SetGammaKineticEnergy(0, gammas[0].energy);
                nonConstTrackInfo->SetGammaMomentumDirection(1, gammas[1].dir);
                nonConstTrackInfo->SetGammaKineticEnergy(1, gammas[1].energy);
                nonConstTrackInfo->SetGammaMomentumDirection(2, gammas[2].dir);
                nonConstTrackInfo->SetGammaKineticEnergy(2, gammas[2].energy);
                nonConstTrackInfo->SetGammaMomentumDirection(3, gammas[3].dir);
                nonConstTrackInfo->SetGammaKineticEnergy(3, gammas[3].energy);
            }            
        }           
    }

    const auto* userInfo = track->GetUserInformation();
    const auto* trackInfo = dynamic_cast<const MyTrackInfo*>(userInfo);
    if (!trackInfo) {
        return;
    }

    MyTrackInfo* nonConstTrackInfo = const_cast<MyTrackInfo*>(trackInfo);
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
            nonConstTrackInfo->GetGammaMomentumDirection(0), nonConstTrackInfo->GetGammaKineticEnergy(0),
            nonConstTrackInfo->GetGammaMomentumDirection(1), nonConstTrackInfo->GetGammaKineticEnergy(1),
            nonConstTrackInfo->GetGammaMomentumDirection(2), nonConstTrackInfo->GetGammaKineticEnergy(2),
            nonConstTrackInfo->GetGammaMomentumDirection(3), nonConstTrackInfo->GetGammaKineticEnergy(3)
        );

        // Wenn das Secondary ein Gamma ist, speichere Energie für Gamma Zuordnung
        if (postStepPoint->GetProcessDefinedStep()->GetProcessName() == "nCapture" && secTrack->GetParticleDefinition() == G4Gamma::Definition()) {
            inheritedInfo->SetPhotonGammaKineticEnergy(secTrack->GetKineticEnergy());
        };

        // Set user information for the secondary track
        const_cast<G4Track*>(secTrack)->SetUserInformation(inheritedInfo);
    }
}    


void MySteppingAction::DefineCommands() {
  
}
