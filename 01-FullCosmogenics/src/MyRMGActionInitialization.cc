#include "MyRMGActionInitialization.hh"
#include "MyEventAction.hh"
#include "MySteppingAction.hh"
// ggf. weitere Includes

RMGActionInitialization::RMGActionInitialization() {}
RMGActionInitialization::~RMGActionInitialization() {}

void RMGActionInitialization::Build() const {
  auto* eventAction = new MyEventAction();
  SetUserAction(eventAction);

  auto* steppingAction = new MySteppingAction(eventAction);
  SetUserAction(steppingAction);

  // ggf. weitere Actions
}

void RMGActionInitialization::BuildForMaster() const {
  // Falls RunAction spezielle Behandlung im Master braucht
  // z.B. SetUserAction(new MyRunAction());
}
