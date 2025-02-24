#include "HardwareQEOverride.hh"

#include "G4OpticalSurface.hh"
#include "G4PhysicsOrderedFreeVector.hh"
#include "G4SurfaceProperty.hh"

#include <fstream>
#include <iostream>
#include <string>

namespace u = CLHEP;
// Thanks to Manuel Huber for the suggestion
// of this method
G4VPhysicalVolume *HardwareQEOverride::Construct() {
  auto world = RMGHardware::Construct();
  // Get the surface of the PMTs
  auto st = G4SurfaceProperty::GetSurfacePropertyTable();
  G4SurfaceProperty *s;
  for (auto x : *st) {
    if (x->GetName().find("PMTSurface") != std::string::npos)
      s = x;
  }
  // Get the MPT
  auto mpt = dynamic_cast<G4OpticalSurface *>(s)->GetMaterialPropertiesTable();

  // New vector to store the efficiency
  // This vector might never be deleted, but it should be only generated once so
  // whatever.
  G4PhysicsOrderedFreeVector *QuantumEfficiency =
      new G4PhysicsOrderedFreeVector;

  // Read in the correct quantum efficiency and add to vectors
  std::ifstream datafile;
  datafile.open("datasheets/R7081_QEWhitespace.csv");
  while (datafile.is_open()) {
    G4double wlen, queff;

    datafile >> wlen >> queff;

    if (datafile.eof())
      break;
    // convert the wavelength (supposed to be in nm) to energy
    QuantumEfficiency->InsertValues(1239.841939 * u::eV / wlen, queff / 100.);
  }
  datafile.close();
  // Overwrite efficiency with new read in efficiency
  mpt->AddProperty("EFFICIENCY", QuantumEfficiency);
  // std::cout << (mpt != nullptr) << std::endl;
  return world;
}
