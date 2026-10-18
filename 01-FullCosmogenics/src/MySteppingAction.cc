// Records every primary neutron capture (NC) of an event and labels the tracks that
// descend from it.
//
// At each capture step the stepping action collects the capture gammas, checks whether
// 77Ge was produced and hands the data to MyNeutronCaptureOutputScheme (one row per NC)
// and MyGammaCaptureOutputScheme (one row per gamma). It then attaches a MyTrackInfo
// (muon ID, NC ID, gamma ID; class of the remage fork) to the neutron and its gammas and
// passes it on to all their secondaries, so that optical hits can later be assigned to
// the capture that produced them.

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


// Stores primary NCs with their gammas and passes the (muon ID, NC ID, gamma ID) label
// on to the secondaries of labelled tracks. The muon itself is not labelled; the
// captured neutron gets gamma ID -1.
void MySteppingAction::UserSteppingAction(const G4Step* step) {
    const G4Track* track = step->GetTrack();
    G4StepPoint* postStepPoint = step->GetPostStepPoint();

    // Check if the process is neutron capture: "nCapture" is the Geant4 process,
    // "RMGnCapture" the remage process used with /RMG/Processes/UseGrabmayrsGammaCascades.
    const std::vector<const G4Track*>* secondaries = step->GetSecondaryInCurrentStep();
    if (
        postStepPoint->GetProcessDefinedStep()->GetProcessName() == "nCapture" ||
        postStepPoint->GetProcessDefinedStep()->GetProcessName() == "RMGnCapture"
    ) {
        // Ensure the captured particle is a neutron
        if (track->GetParticleDefinition() == G4Neutron::Definition()) {
            // Check whether NC info already exists (= secondary NC). Only tracks that
            // descend from an already recorded NC carry a MyTrackInfo.
            const auto* userInfo = track->GetUserInformation();
            const auto* trackInfo = dynamic_cast<const MyTrackInfo*>(userInfo);

            // If trackInfo exists -> secondary NC -> do NOT store it!
            if (trackInfo) {
                G4cout << "⚠ Secondary NC detected (track " << track->GetTrackID()
                    << "), primary NC (track " << trackInfo->GetNCID() << ") already stored" << G4endl;
                // Do nothing - only primary NCs are stored
            }
            else {
                // PRIMARY NC -> store the NC info and all gammas
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

                // Each MUSUN event contains one muon, so the event ID serves as muon ID.
                G4int muonTrackID = G4EventManager::GetEventManager()->GetConstCurrentEvent()->GetEventID();
                G4int ncTrackID = track->GetTrackID();
                G4int gammaCount = 0;
                G4double totalGammaEnergy = 0.0;
                G4bool fGe77 = false;

                std::vector<MyGammaCaptureOutputScheme::GammaInfo> gammas;
                std::vector<const G4Track*> gammaTracks; // parallel to 'gammas' (same order)

                // Collect all gammas and check for Ge77
                for (const auto& secTrack : *secondaries) {
                    const auto particle = secTrack->GetParticleDefinition();

                    if (particle->IsGeneralIon()) {
                        int z = particle->GetAtomicNumber();
                        int a = particle->GetAtomicMass();
                        if (z == 32 && a == 77) {
                            fGe77 = true;
                            G4cout << "Ge-77 produced! 🎉" << G4endl;
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

                // Sort a COPY by energy (descending) for the top 4.
                // 'gammas' itself stays in the original order, parallel to 'gammaTracks'.
                std::vector<MyGammaCaptureOutputScheme::GammaInfo> sortedGammas = gammas;
                std::sort(sortedGammas.begin(), sortedGammas.end(),
                    [](const MyGammaCaptureOutputScheme::GammaInfo& a,
                       const MyGammaCaptureOutputScheme::GammaInfo& b) {
                    return a.energy > b.energy;
                });

                // Build the top-4 vector for the NC output scheme; missing gammas are
                // filled with zero direction and energy, so every NC row has four entries.
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

                // Store the NC info in the NeutronCaptureOutputScheme
                MyNeutronCaptureOutputScheme::NCInfo ncInfo;
                // Capture point (global), NOT the creation/vertex position of the neutron.
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

                // Store the gammas and set their UserInfo. The gamma ID is the index of
                // the gamma among the capture secondaries.
                // gammaTracks[gi] and gammas[gi] are guaranteed to be consistent (same order).
                for (size_t gi = 0; gi < gammaTracks.size(); ++gi) {
                    G4int gammaID = static_cast<G4int>(gi);

                    // Store in the GammaCaptureOutputScheme
                    MyGammaCaptureOutputScheme::AddPendingGamma(
                        ncTrackID, gammaID, gammas[gi]
                    );

                    // Set the UserInfo of the gamma. The secondaries are not tracked
                    // yet, so their labels can still be set here (Geant4 only hands out
                    // const tracks, hence the const_cast).
                    auto* gammaInfo = new MyTrackInfo(muonTrackID, ncTrackID, gammaID);
                    const_cast<G4Track*>(gammaTracks[gi])->SetUserInformation(gammaInfo);
                }

                // Set the UserInfo of the NC track itself (gamma ID -1)
                auto* info = new MyTrackInfo(muonTrackID, ncTrackID, -1);
                const_cast<G4Track*>(track)->SetUserInformation(info);
            }
        }
    }

    // Every labelled track passes a copy of its label on to its secondaries, so all
    // particles (including optical photons) produced downstream of an NC carry its IDs.
    const auto* userInfo = track->GetUserInformation();
    const auto* trackInfo = dynamic_cast<const MyTrackInfo*>(userInfo);
    if (!trackInfo) {
        return;
    }

    // BEGIN UNUSED CODE — nonConstTrackInfo is never read
    MyTrackInfo* nonConstTrackInfo = const_cast<MyTrackInfo*>(trackInfo);
    // END UNUSED CODE

    // Pass the UserInfo on to all secondaries
    for (const auto& secTrack : *secondaries) {
        // Skip gammas produced directly by the NC (they already have their own UserInfo)
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


// No messenger commands are defined; kept as a placeholder.
void MySteppingAction::DefineCommands() {

}
