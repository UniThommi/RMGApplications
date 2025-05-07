#include "MyActionInitialization.hh"
#include "MyEventAction.hh"
#include "MySteppingAction.hh"
// ggf. weitere Includes

MyActionInitialization::MyActionInitialization() {
    this->Build();
}
MyActionInitialization::~MyActionInitialization() {};

void MyActionInitialization::Build() const {
  auto* eventAction = new MyEventAction();
  SetUserAction(eventAction);

  auto* steppingAction = new MySteppingAction(eventAction);
  SetUserAction(steppingAction);

  // ggf. weitere Actions
}
