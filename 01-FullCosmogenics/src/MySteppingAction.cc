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
    // if (postStepPoint->GetProcessDefinedStep()->GetProcessName() == "nCapture") {  // Nur korrekt wenn Grabmayr Kaskaden nicht aktiv
    if (postStepPoint->GetProcessDefinedStep()->GetProcessName() == "RMGnCapture") {
    // Ensure the captured particle is a neutron
        if (track->GetParticleDefinition() == G4Neutron::Definition()) {
            // Prüfe ob bereits NC-Info existiert (= sekundärer NC)
            const auto* userInfo = track->GetUserInformation();
            const auto* trackInfo = dynamic_cast<const MyTrackInfo*>(userInfo);
            
            // WENN trackInfo existiert → sekundärer NC → NICHT überschreiben!
            if (trackInfo) {
                G4cout << "⚠ Sekundärer NC detektiert (Track " << track->GetTrackID() 
                    << "), behalte primären NC (Track " << trackInfo->GetnCTrackID() << ")" << G4endl;
                // Nichts tun - primäre NC-Info bleibt erhalten
            }
            else{
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

                auto* info = new MyTrackInfo(
                    track->GetTrackID(),                        
                    track->GetVertexPosition(),                 
                    track->GetGlobalTime(),                     
                    physVolumeName,                             
                    materialName,                               
                    gammaCount,                                 
                    totalGammaEnergy,                           
                    fGe77,                                      
                    -1.,                                        
                    gammas[0].dir, gammas[0].energy,
                    gammas[1].dir, gammas[1].energy,
                    gammas[2].dir, gammas[2].energy,
                    gammas[3].dir, gammas[3].energy       
                );
                track->SetUserInformation(info);
            }  
        }           
    }

    // Inheritance nur wenn NC-Info existiert
    const auto* userInfo = track->GetUserInformation();
    const auto* trackInfo = dynamic_cast<const MyTrackInfo*>(userInfo);
    if (!trackInfo) {
        return;
    }

    MyTrackInfo* nonConstTrackInfo = const_cast<MyTrackInfo*>(trackInfo);
    
    // Vererbe NC-Info zu secondaries (außer Neutronen)
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
            nonConstTrackInfo->GetPhotonGammaKineticEnergy(),
            nonConstTrackInfo->GetGammaMomentumDirection(0), nonConstTrackInfo->GetGammaKineticEnergy(0),
            nonConstTrackInfo->GetGammaMomentumDirection(1), nonConstTrackInfo->GetGammaKineticEnergy(1),
            nonConstTrackInfo->GetGammaMomentumDirection(2), nonConstTrackInfo->GetGammaKineticEnergy(2),
            nonConstTrackInfo->GetGammaMomentumDirection(3), nonConstTrackInfo->GetGammaKineticEnergy(3)
        );

        // Gammas vom NC: Speichere ihre spezifische Energie
        if (postStepPoint->GetProcessDefinedStep()->GetProcessName() == "nCapture" 
            && secTrack->GetParticleDefinition() == G4Gamma::Definition()) {
            inheritedInfo->SetPhotonGammaKineticEnergy(secTrack->GetKineticEnergy());
        }

        const_cast<G4Track*>(secTrack)->SetUserInformation(inheritedInfo);
    }
}    


void MySteppingAction::DefineCommands() {
  
}
