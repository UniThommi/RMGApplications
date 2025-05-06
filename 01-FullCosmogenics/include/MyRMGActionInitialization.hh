#ifndef MY_RMG_ACTION_INITIALIZATION_HH
#define MY_RMG_ACTION_INITIALIZATION_HH 

#include "G4VUserActionInitialization.hh"

class RMGActionInitialization : public G4VUserActionInitialization {
public:
  RMGActionInitialization();
  virtual ~RMGActionInitialization();

  virtual void Build() const override;
  virtual void BuildForMaster() const override;
};

#endif //MY_RMG_ACTION_INITIALIZATION_HH
