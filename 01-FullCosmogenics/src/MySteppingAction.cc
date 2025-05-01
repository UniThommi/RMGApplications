#include "MyPhotonHit.hh"
#include "MyTrackInfo.hh"
#include "MyPhotonHitsCollection.hh"

#include "RMGOpticalDetector.hh"

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

MySteppingAction::MySteppingAction(MyEventAction* eventAction)
    : fEventAction(eventAction) {}

// Destructor
MySteppingAction::~MySteppingAction() {}    


// Inheriting Primary ID and 1st Gen Neutron ID. For Muon track, the 1st Gen Neutron ID is set to -1.
// Also save Gamma data from Neutron captures.
void MySteppingAction::UserSteppingAction(const G4Step* step) {
    const G4Track* track = step->GetTrack();

    if (track->GetParentID() == 0) { // Primary Particle
        auto* info = new MyTrackInfo(
            -1,                     // TrackID
            (-1., -1., -1.),        // nC Pos
            -1.,                    // nC Time
            -1,                     // nC Phys Vol
            -1,                     // nC Material
            -1,                     // nC Gamma Amount
            -1.,                    // nC Gamma Total Energy
            false,                  // nC fGe77
            (-1., -1., -1.),        // Gamma Momentum Direction
            -1.                     // Gamma Kinetic Energy
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
            G4string physVolumeName = "";
            G4string materialName = "";
            if (physicalVolume) {
                physVolumeName = physicalVolume->GetName()
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


    // Inherit Track Info to secondary particles
    for (const auto& secTrack : *secondaries) {
        auto* inheritedInfo = new MyTrackInfo(
            nonConstTrackInfo->GetnCNeutronID(),
            nonConstTrackInfo->GetnCfGe77()
        );
        // Wenn das Secondary ein Gamma ist, speichere Energie & Impulsrichtung
        if (secTrack->GetParticleDefinition() == G4Gamma::Definition()) {
            inheritedInfo->SetGammaKineticEnergy(secTrack->GetKineticEnergy());
            inheritedInfo->SetGammaMomentumDirection(secTrack->GetMomentumDirection());
        };

        // Set user information for the secondary track
        const_cast<G4Track*>(secTrack)->SetUserInformation(inheritedInfo);
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
                if (d_type != RMGDetectorType::kOptical) {
                RMGLog::OutFormatDev(RMGLog::debug,
                    "Volume '{}' (copy nr. {} not registered as optical detector", pv_name, pv_copynr);
                return false;
                }
            } catch (const std::out_of_range& e) {
                RMGLog::OutFormatDev(RMGLog::debug, "Volume '{}' (copy nr. {} not registered as detector",
                    pv_name, pv_copynr);
                return false;
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
            G4int nCPhysVol = nonConstTrackInfo->GetnCPhysVol();
            G4string nCMaterial = nonConstTrackInfo->GetnCMaterial();
            G4int nCGammaAmount = nonConstTrackInfo->GetnCGammaAmount();
            G4double nCGammaTotalEnergy = nonConstTrackInfo->GetnCGammaTotalEnergy();
            G4bool nCfGe77 = nonConstTrackInfo->GetnCfGe77();
            G4ThreeVector gammaMomentumDirection = nonConstTrackInfo->GetGammaMomentumDirection();
            G4double gammaKineticEnergy = nonConstTrackInfo->GetGammaKineticEnergy();

            // Saving Data To Hit Allocator
            // Erstelle ein PhotonHit-Objekt und speichere die Daten
            PhotonHit* hit = new PhotonHit();
            hit->SetDetectorUID(det_uid);
            hit->SetOptPhotonEnergy(photon_energy);
            hit->SetOptPhotonglobalTime(photon_global_time);
            hit->SetOptPhotonPosition(photon_position);
            hit->SetOptPhotonMomentumDirection(photon_momentum_direction);
            hit->SetnCTrackID(nCTrackID);
            hit->SetnCPos(nCPos);
            hit->SetnCTime(nCTime);
            hit->SetnCPhysVol(nCPhysVol);
            hit->SetnCMaterial(nCMaterial);
            hit->SetnCGammaAmount(nCGammaAmount);
            hit->SetnCGammaTotalEnergy(nCGammaTotalEnergy);
            hit->SetnCfGe77(nCfGe77);
            hit->SetGammaMomentumDirection(gammaMomentumDirection);
            hit->SetGammaKineticEnergy(gammaKineticEnergy);

            // Hole HitsCollection aus G4Event
            auto hitsCollection = fEventAction->GetPhotonHitsCollection();
            if (hitsCollection) {

            if (hitsCollection)
                hitsCollection->insert(hit);
            else {
                G4cout << "FEHLER: Hits Collection existiert nicht, PhotonHit wird nicht gespeichert!!!" << G4endl;
            }           
        }
    }
}

