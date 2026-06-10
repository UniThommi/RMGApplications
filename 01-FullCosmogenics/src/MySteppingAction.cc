#include "MySteppingAction.hh"
#include "MyTrackInfo.hh"
#include "MyGammaCaptureOutputScheme.hh"
#include "MyNeutronCaptureOutputScheme.hh"

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
    if (
        postStepPoint->GetProcessDefinedStep()->GetProcessName() == "nCapture" ||
        postStepPoint->GetProcessDefinedStep()->GetProcessName() == "RMGnCapture"
    ) {
        // Ensure the captured particle is a neutron
        if (track->GetParticleDefinition() == G4Neutron::Definition()) {
            // Prüfe ob bereits NC-Info existiert (= sekundärer NC)
            const auto* userInfo = track->GetUserInformation();
            const auto* trackInfo = dynamic_cast<const MyTrackInfo*>(userInfo);
            
            // WENN trackInfo existiert → sekundärer NC → NICHT speichern!
            if (trackInfo) {
                G4cout << "⚠ Sekundärer NC detektiert (Track " << track->GetTrackID() 
                    << "), primärer NC (Track " << trackInfo->GetNCID() << ") bereits gespeichert" << G4endl;
                // Nichts tun - nur primäre NCs werden gespeichert
            }
            else {
                // PRIMÄRER NC → Speichere NC-Info + alle Gammas
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

                G4int muonTrackID = G4EventManager::GetEventManager()->GetConstCurrentEvent()->GetEventID();
                G4int ncTrackID = track->GetTrackID();
                G4int gammaCount = 0;
                G4double totalGammaEnergy = 0.0;
                G4bool fGe77 = false;

                std::vector<MyGammaCaptureOutputScheme::GammaInfo> gammas;
                std::vector<const G4Track*> gammaTracks; // parallel zu 'gammas' (gleiche Reihenfolge)

                // Sammle alle Gammas und prüfe auf Ge77
                for (const auto& secTrack : *secondaries) {
                    const auto particle = secTrack->GetParticleDefinition();
                    
                    if (particle->IsGeneralIon()) {
                        int z = particle->GetAtomicNumber();
                        int a = particle->GetAtomicMass();
                        if (z == 32 && a == 77) {
                            fGe77 = true;
                            G4cout << "Ge-77 erzeugt! 🎉" << G4endl;
                        }
                    }

                    if (particle == G4Gamma::Definition()) {
                        gammaCount++;
                        G4double E = secTrack->GetKineticEnergy();
                        totalGammaEnergy += E;
                        
                        MyGammaCaptureOutputScheme::GammaInfo info;
                        info.dir = secTrack->GetMomentumDirection();
                        info.energy = E;
                        info.polarization = secTrack->GetPolarization();
                        gammas.push_back(info);
                        gammaTracks.push_back(secTrack);
                    }
                }

                // Sortiere eine KOPIE nach Energie (absteigend) für die Top-4.
                // 'gammas' selbst bleibt in Original-Reihenfolge, parallel zu 'gammaTracks'.
                std::vector<MyGammaCaptureOutputScheme::GammaInfo> sortedGammas = gammas;
                std::sort(sortedGammas.begin(), sortedGammas.end(),
                    [](const MyGammaCaptureOutputScheme::GammaInfo& a,
                       const MyGammaCaptureOutputScheme::GammaInfo& b) {
                    return a.energy > b.energy;
                });

                // Erstelle Top-4 Vektor für NC-OutputScheme
                struct Top4Gamma {
                    G4ThreeVector dir;
                    G4double energy;
                };
                std::vector<Top4Gamma> top4Gammas;
                for (size_t i = 0; i < std::min(sortedGammas.size(), size_t(4)); ++i) {
                    top4Gammas.push_back({sortedGammas[i].dir, sortedGammas[i].energy});
                }
                while (top4Gammas.size() < 4) {
                    top4Gammas.push_back({G4ThreeVector(0., 0., 0.), 0.});
                }

                // Speichere NC-Info im NeutronCaptureOutputScheme
                MyNeutronCaptureOutputScheme::NCInfo ncInfo;
                // Capture-Punkt (global), NICHT die Erzeugungs-/Vertex-Position des Neutrons.
                ncInfo.pos = postStepPoint->GetPosition();
                ncInfo.time = track->GetGlobalTime();
                ncInfo.physVol = physVolumeName;
                ncInfo.material = materialName;
                ncInfo.gammaAmount = gammaCount;
                ncInfo.gammaTotalEnergy = totalGammaEnergy;
                ncInfo.fGe77 = fGe77;
                ncInfo.gamma1_dir = top4Gammas[0].dir;
                ncInfo.gamma1_E = top4Gammas[0].energy;
                ncInfo.gamma2_dir = top4Gammas[1].dir;
                ncInfo.gamma2_E = top4Gammas[1].energy;
                ncInfo.gamma3_dir = top4Gammas[2].dir;
                ncInfo.gamma3_E = top4Gammas[2].energy;
                ncInfo.gamma4_dir = top4Gammas[3].dir;
                ncInfo.gamma4_E = top4Gammas[3].energy;
                MyNeutronCaptureOutputScheme::AddPendingNC(ncTrackID, ncInfo);

                // Speichere Gammas und setze UserInfo.
                // gammaTracks[gi] und gammas[gi] sind garantiert konsistent (gleiche Reihenfolge).
                for (size_t gi = 0; gi < gammaTracks.size(); ++gi) {
                    G4int gammaID = static_cast<G4int>(gi);

                    // Speichere im GammaCaptureOutputScheme
                    MyGammaCaptureOutputScheme::AddPendingGamma(
                        ncTrackID, gammaID, gammas[gi]
                    );

                    // Setze UserInfo für Gamma
                    auto* gammaInfo = new MyTrackInfo(muonTrackID, ncTrackID, gammaID);
                    const_cast<G4Track*>(gammaTracks[gi])->SetUserInformation(gammaInfo);
                }

                // Setze UserInfo für NC-Track selbst
                auto* info = new MyTrackInfo(muonTrackID, ncTrackID, -1);
                const_cast<G4Track*>(track)->SetUserInformation(info);
            }  
        }           
    }

    // Vererbung der UserInfo an alle Sekundärteilchen
    const auto* userInfo = track->GetUserInformation();
    const auto* trackInfo = dynamic_cast<const MyTrackInfo*>(userInfo);
    if (!trackInfo) {
        return;
    }

    MyTrackInfo* nonConstTrackInfo = const_cast<MyTrackInfo*>(trackInfo);

    // Vererbe UserInfo zu allen Sekundärteilchen
    for (const auto& secTrack : *secondaries) {
        // Skip Gammas die direkt aus NC entstehen (haben bereits UserInfo)
        if (postStepPoint->GetProcessDefinedStep() &&
            (postStepPoint->GetProcessDefinedStep()->GetProcessName() == "nCapture" ||
             postStepPoint->GetProcessDefinedStep()->GetProcessName() == "RMGnCapture") &&
            secTrack->GetParticleDefinition() == G4Gamma::Definition()) {
            continue;
        }

        auto* inheritedInfo = new MyTrackInfo(trackInfo->GetMuonID(), trackInfo->GetNCID(), trackInfo->GetGammaID());
        const_cast<G4Track*>(secTrack)->SetUserInformation(inheritedInfo);
    }
}    


void MySteppingAction::DefineCommands() {
  
}
