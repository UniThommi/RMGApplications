#ifndef _HARDWARE_QE_OVERRIDE_HH_
#define _HARDWARE_QE_OVERRIDE_HH_

#include "RMGHardware.hh"

class HardwareQEOverride : public RMGHardware {

public:
  G4VPhysicalVolume *Construct() override;
};

#endif
