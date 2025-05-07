#ifndef MY_ACTION_INITIALIZATION_HH
#define MY_ACTION_INITIALIZATION_HH 

#include "G4VUserActionInitialization.hh"

class MyActionInitialization : public G4VUserActionInitialization {
public:
  MyActionInitialization();
  virtual ~MyActionInitialization();

  virtual void Build() const override;
};

#endif //MY_ACTION_INITIALIZATION_HH
