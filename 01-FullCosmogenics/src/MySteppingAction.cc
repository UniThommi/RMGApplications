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
        G4cout << "NeutronCapture happened" << G4endl;
        // Ensure the captured particle is a neutron
        if (track->GetParticleDefinition() == G4Neutron::Definition()) {
            G4cout << "Neutron of NeutronCapture" << G4endl;

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
                }
            }

            const auto* userInfo = track->GetUserInformation();
            const auto* trackInfo = dynamic_cast<const MyTrackInfo*>(userInfo);
            if (!trackInfo) {
                G4cout << "Neue TrackInfo für nC Daten" << G4endl;
                auto* info = new MyTrackInfo(
                    track->GetTrackID(),                                 // TrackID
                    track->GetVertexPosition(),       // nC Pos
                    track->GetGlobalTime(),                                // nC Time
                    physVolumeName,                                 // nC Phys Vol
                    materialName,                                 // nC Material
                    gammaCount,                                 // nC Gamma Amount
                    totalGammaEnergy,                                // nC Gamma Total Energy
                    fGe77,                              // nC fGe77
                    G4ThreeVector(-1., -1., -1.),       // Gamma Momentum Direction
                    -1.                                 // Gamma Kinetic Energy
                );
                track->SetUserInformation(info);
            }
            else {
                G4cout << "Überschreibe alte TrackInfo mit nC Daten" << G4endl;
                MyTrackInfo* nonConstTrackInfo = const_cast<MyTrackInfo*>(trackInfo);
                nonConstTrackInfo->SetnCTrackID(track->GetTrackID());
                nonConstTrackInfo->SetnCPos(track->GetVertexPosition());
                nonConstTrackInfo->SetnCTime(track->GetGlobalTime());
                nonConstTrackInfo->SetnCPhysVol(physVolumeName);
                nonConstTrackInfo->SetnCMaterial(materialName);
                nonConstTrackInfo->SetnCGammaAmount(gammaCount);
                nonConstTrackInfo->SetnCGammaTotalEnergy(totalGammaEnergy);
                nonConstTrackInfo->SetnCfGe77(fGe77);
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
            nonConstTrackInfo->GetGammaMomentumDirection(),
            nonConstTrackInfo->GetGammaKineticEnergy()
        );
        // Wenn das Secondary ein Gamma ist, speichere Energie & Impulsrichtung
        if (postStepPoint->GetProcessDefinedStep()->GetProcessName() == "nCapture" && secTrack->GetParticleDefinition() == G4Gamma::Definition()) {
            G4cout << "Set Gamma Energy and Momentum Direction to Gamma" << G4endl;
            inheritedInfo->SetGammaKineticEnergy(secTrack->GetKineticEnergy());
            inheritedInfo->SetGammaMomentumDirection(secTrack->GetMomentumDirection());
        };

        // Set user information for the secondary track
        const_cast<G4Track*>(secTrack)->SetUserInformation(inheritedInfo);
    }
}    


void MySteppingAction::DefineCommands() {
  
}
