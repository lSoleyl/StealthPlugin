#include "plugin.hpp"

#include <sstream>

BAKKESMOD_PLUGIN(StealthPlugin, "Stealth Cars", "0.1", PLUGINTYPE_FREEPLAY | PLUGINTYPE_CUSTOM_TRAINING)


// Debugging helper
#define OutputDebugStream(msg) do { std::ostringstream str; str << std::boolalpha << msg; cv->log(str.str()); } while(false)
//#define OutputDebugStream(msg)


bool StealthPlugin::shouldApplyStealth() const {
  return gameWrapper->IsInGame() && !gameWrapper->IsInOnlineGame() && !gameWrapper->IsInFreeplay() && !gameWrapper->IsInCustomTraining();
}

void StealthPlugin::applyStealth() const {
  auto cars = gameWrapper->GetGameEventAsServer().GetCars();
  for (int i = 0; i < cars.Count(); ++i) {
    auto car = cars.Get(i);

    //TODO: this constant should be made configurable
    if (car.GetBoostComponent().GetPercentBoostFull() > 0.75f) {
      car.SetHidden2(TRUE);
      car.SetbHiddenSelf(TRUE);
    } else {
      car.SetHidden2(FALSE);
      car.SetbHiddenSelf(FALSE);
    }
  }
}


void StealthPlugin::onTick() const {
  
  if (shouldApplyStealth()) {
    applyStealth();
    // Set high tick rate only if we are ingame
    gameWrapper->SetTimeout([this](GameWrapper*) { onTick(); }, 0.1f);
  } else {
    // Otherwise check once per second
    gameWrapper->SetTimeout([this](GameWrapper*) { onTick(); }, 1.0f);
  }
}


void StealthPlugin::onLoad() {
  // We use GameWrapper::setTimeout() to reliably call back into our code at a steady rate since I don't know any events
  // which I could use for this. OnGameTimeUpdated would be one, but it doesn't get called if the 
  onTick();
  

  //TODO: add some configuration variable to control the amount of boost needed for stealth
  //TODO: add some configuration variable to enable/disable the plugin
  //cvarManager->registerNotifier("stealth", stealthCommand, "Activate stealth", PERMISSION_ALL);
}

// No need to disable the timeouts as Bakkes takes care of that.
void StealthPlugin::onUnload() {}

