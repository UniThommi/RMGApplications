#include "CosmogenicPhysics.hh"
#include "RNGCerenkov.hh"

#include "G4OpAbsorption.hh"
#include "G4OpBoundaryProcess.hh"
#include "G4OpRayleigh.hh"
#include "G4OpWLS.hh"
#include "G4OpticalParameters.hh"
#include "G4OpticalPhoton.hh"
#include "G4ProcessManager.hh"
#include "G4Scintillation.hh"

#include "RMGLog.hh"

CosmogenicPhysics::CosmogenicPhysics() : RMGPhysics() {}

void CosmogenicPhysics::ConstructOptical() {

  RMGLog::Out(RMGLog::detail, "Adding optical physics");

  G4OpticalParameters *op_par = G4OpticalParameters::Instance();
  op_par->SetScintTrackSecondariesFirst(true);
  // For now this is necessary to ensure the RNG is unaffected by optical
  // processes
  op_par->SetCerenkovTrackSecondariesFirst(false);
  op_par->SetCerenkovMaxBetaChange(-1.);
  op_par->SetCerenkovMaxPhotonsPerStep(-1);
  op_par->SetScintByParticleType(true);
  op_par->SetBoundaryInvokeSD(true);

  // no scintillation process for now
  auto scint_proc = new G4Scintillation("Scintillation");
  scint_proc->SetTrackSecondariesFirst(true);
  scint_proc->SetVerboseLevel(G4VModularPhysicsList::verboseLevel);

  // optical processes
  auto absorption_proc = new G4OpAbsorption();
  auto boundary_proc = new G4OpBoundaryProcess();
  auto rayleigh_scatt_proc = new G4OpRayleigh();
  auto wls_proc = new G4OpWLS();
  auto cerenkov_proc = new RNGCerenkov();

  G4cout << "Maximum beta change per step: "
         << op_par->GetCerenkovMaxBetaChange() << G4endl;
  G4cout << "Maximum photons per step: "
         << op_par->GetCerenkovMaxPhotonsPerStep() << G4endl;
  G4cout << "Track secondaries first: "
         << op_par->GetCerenkovTrackSecondariesFirst() << G4endl;
  G4cout << "Stack photons: " << op_par->GetCerenkovStackPhotons() << G4endl;
  G4cout << "Verbose level: " << op_par->GetCerenkovVerboseLevel() << G4endl;

  absorption_proc->SetVerboseLevel(G4VModularPhysicsList::verboseLevel);
  boundary_proc->SetVerboseLevel(G4VModularPhysicsList::verboseLevel);
  wls_proc->SetVerboseLevel(G4VModularPhysicsList::verboseLevel);

  GetParticleIterator()->reset();
  while ((*GetParticleIterator())()) {
    auto particle = GetParticleIterator()->value();
    auto proc_manager = particle->GetProcessManager();
    auto particle_name = particle->GetParticleName();

    if (scint_proc->IsApplicable(*particle)) {
      proc_manager->AddProcess(scint_proc);
      //  This messes with the random engine (probably changes the order)
      //  proc_manager->SetProcessOrderingToLast(scint_proc,
      //  G4ProcessVectorDoItIndex::idxAtRest);
      //  proc_manager->SetProcessOrderingToLast(scint_proc,
      //  G4ProcessVectorDoItIndex::idxPostStep);
    }

    if (cerenkov_proc->IsApplicable(*particle)) {
      proc_manager->AddProcess(cerenkov_proc);
      proc_manager->SetProcessOrdering(cerenkov_proc,
                                       G4ProcessVectorDoItIndex::idxPostStep);
    }

    if (particle_name == "opticalphoton") {
      proc_manager->AddDiscreteProcess(absorption_proc);
      proc_manager->AddDiscreteProcess(boundary_proc);
      proc_manager->AddDiscreteProcess(rayleigh_scatt_proc);
      proc_manager->AddDiscreteProcess(wls_proc);
    }
  }
}
