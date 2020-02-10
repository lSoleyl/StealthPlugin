#include "plugin.hpp"

#include <sstream>

BAKKESMOD_PLUGIN(StealthPlugin, "Stealth Cars", "0.1", PLUGINTYPE_FREEPLAY | PLUGINTYPE_CUSTOM_TRAINING)


// Debugging helper
#define OutputDebugStream(msg) do { std::ostringstream str; str << std::boolalpha << msg; cv->log(str.str()); } while(false)
//#define OutputDebugStream(msg)


StealthPlugin::StealthPlugin() : boostThresholdCVar(new int(0)) {}

bool StealthPlugin::shouldApplyStealth() const {
  return gameWrapper->IsInGame() && !gameWrapper->IsInOnlineGame() && !gameWrapper->IsInFreeplay() && !gameWrapper->IsInCustomTraining();
}

void StealthPlugin::applyStealth() {
  auto cars = gameWrapper->GetGameEventAsServer().GetCars();

  float thresholdValue = *boostThresholdCVar / 100.0f;

  for (int i = 0; i < cars.Count(); ++i) {
    auto car = cars.Get(i);

    // By using >= a threshold of 0 can be used to always stay invisible
    if (car.GetBoostComponent().GetPercentBoostFull() >= thresholdValue) {
      car.SetHidden2(TRUE);
      car.SetbHiddenSelf(TRUE);
    } else {
      car.SetHidden2(FALSE);
      car.SetbHiddenSelf(FALSE);
    }
  }
}


void StealthPlugin::onTick() {
  
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
  //TODO: find out how add a neat little plugin UI for configuring these values

  // Register cvar to make the boost threshold configurable
  auto cvar = cvarManager->registerCvar("stealth_boost_threshold", "75", "Stealth activates if boost is at least at this value", true, true, 0.0f, true, 100.0f, true);

  // This will cause the cvar to update our int pointer whenever the cvar gets changed
  cvar.bindTo(boostThresholdCVar);


  // We use GameWrapper::setTimeout() to reliably call back into our code at a steady rate since I don't know any events
  // which I could use for this. OnGameTimeUpdated would be one, but it doesn't get called if the clock isn't running (before kickoff).
  onTick();
}

// No need to disable the timeouts as Bakkes takes care of that.
void StealthPlugin::onUnload() {}

