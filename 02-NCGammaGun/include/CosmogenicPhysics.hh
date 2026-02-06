#ifndef _COSMOGENIC_PHYSICS_HH_
#define _COSMOGENIC_PHYSICS_HH_

#include "RMGPhysics.hh"

class CosmogenicPhysics : public RMGPhysics {
public:
  CosmogenicPhysics();

  // Override the ConstructOptical to customize optical processes
  void ConstructOptical() override;
};

#endif
