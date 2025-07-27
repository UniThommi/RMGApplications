#ifndef _HARDWARE_QE_OVERRIDE_HH_
#define _HARDWARE_QE_OVERRIDE_HH_

#include "RMGHardware.hh"
#include <string>

class HardwareQEOverride : public RMGHardware {

public:
  HardwareQEOverride(const std::string &surfaceType = "SSD"); // <--- Konstruktor
  G4VPhysicalVolume *Construct() override;

private:
  std::string selectedSurfaceType;
};

#endif
