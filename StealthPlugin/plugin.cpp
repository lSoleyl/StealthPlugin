#include "plugin.hpp"

#include <sstream>
#include <regex>

BAKKESMOD_PLUGIN(StealthPlugin, "Stealth plugin", "1.0", PLUGINTYPE_FREEPLAY | PLUGINTYPE_CUSTOM_TRAINING)


// Debugging helper
#define OutputDebugStream(msg) do { std::ostringstream str; str << std::boolalpha << msg; cvarManager->log(str.str()); } while(false)
//#define OutputDebugStream(msg)


const float StealthPlugin::ACTIVE_TICK_DURATION = 0.1f; // 10 ticks per second
const float StealthPlugin::IDLE_TICK_DURATION   = 1.0f; // check once per second when idle


StealthPlugin::StealthPlugin() : boostRange(0.75f, 1.0f), state(State::STOPPED) {}

bool StealthPlugin::shouldApplyStealth() const {
  return gameWrapper->IsInGame() && !gameWrapper->IsInOnlineGame() && !gameWrapper->IsInFreeplay() && !gameWrapper->IsInCustomTraining();
}

void StealthPlugin::applyStealth() {
  auto cars = gameWrapper->GetGameEventAsServer().GetCars();

  for (int i = 0; i < cars.Count(); ++i) {
    auto car = cars.Get(i);

    // What is the difference between GetPercentBoostFull() and GetCurrentBoostAmount() !?
    auto boostAmount = car.GetBoostComponent().GetPercentBoostFull(); // a value between 0 and 1 (incl.)
    if (boostRange.first <= boostAmount && boostAmount <= boostRange.second) {
      car.SetHidden2(TRUE);
      car.SetbHiddenSelf(TRUE);
    } else {
      car.SetHidden2(FALSE);
      car.SetbHiddenSelf(FALSE);
    }
  }
}

void StealthPlugin::showAllCars() {
  if (shouldApplyStealth()) {
    auto cars = gameWrapper->GetGameEventAsServer().GetCars();

    for (int i = 0; i < cars.Count(); ++i) {
      auto car = cars.Get(i);
      car.SetHidden2(FALSE);
      car.SetbHiddenSelf(FALSE);
    }
  }
}


void StealthPlugin::onTick() {
  if (state == State::STOP_REQUESTED) {
    // exit timeout loop... plugin has been disabled
    state = State::STOPPED;
    // but we have to turn all cars visible again if we are
    // currently inside a locally hosted match
    showAllCars();
    return;
  }

  if (shouldApplyStealth()) {
    applyStealth();
    // Set high tick rate only if we are ingame
    gameWrapper->SetTimeout([this](GameWrapper*) { onTick(); }, ACTIVE_TICK_DURATION);
  } else {
    // Otherwise check once per second
    gameWrapper->SetTimeout([this](GameWrapper*) { onTick(); }, IDLE_TICK_DURATION);
  }
}

namespace {
  // regex for matching new boost range string either as "(a,b)" or "a"
  // Allocated here to omit the sync code of static method member and to not 
  // repeatedly build the same regex
  std::regex rangeRegex("(\\(\\s*([0-9]+)(\\.[0-9]+)?\\s*,\\s*([0-9]+)(\\.[0-9]+)?\\s*\\))|(([0-9]+)(\\.[0-9]+)?)");
}



void StealthPlugin::updateBoostRange(const std::string& rangeValue) {
  std::smatch matches;
  if (std::regex_match(rangeValue, matches, rangeRegex)) {
    if (matches[1].matched) {
      // matched range (a,b)
      try {
        boostRange = std::make_pair(stoi(matches[2].str()) / 100.0f, stoi(matches[4].str()) / 100.0f);
      } catch (...) { // the conversion functions may throw if out of range or something like this
        OutputDebugStream("Boost range conversion failed for: " << rangeValue);
      }
    } else {
      // matched single number (group 7)-> implicitly treat as lower bound
      try {
        boostRange = std::make_pair(stoi(matches[7].str()) / 100.0f, 1.0f);
      } catch (...) { // the conversion functions may throw if out of range or something like this
        OutputDebugStream("Boost range conversion failed for: " << rangeValue);
      }
    }
  } // ignore non matching values
}


void StealthPlugin::updateEnabledState(bool enabled) {
  
  if (enabled) {
    if (state == State::STOPPED) {
      // No active timeout -> start a new loop
      state = State::RUNNING;

      // We use GameWrapper::setTimeout() to reliably call back into our code at a steady rate since I don't know any events
      // which I could use for this. OnGameTimeUpdated would be one, but it doesn't get called if the clock isn't running (before kickoff).
      // onTick() starts the timeout loop
      onTick();
    } else if (state == State::STOP_REQUESTED) {
      // Don't start a new loop here, just cancel the stop request
      state = State::RUNNING;
    }
    // otherwise nothing to do, already stopped
  } else { // !enabled
    if (state == State::RUNNING) {
      // Request timeout loop to exit
      state = State::STOP_REQUESTED;
    }
    // otherwise do nothing.
  }
}


void StealthPlugin::onLoad() {
  // Register cvar to make the boost stealth range configurable
  auto cvar = cvarManager->registerCvar("stealth_boost_range", "(75,100)", "Stealth activates if boost is in this range", true, true, 0.0f, true, 100.0f, true);

  // Initialize boost range member and add change listener
  updateBoostRange(cvar.getStringValue());
  cvar.addOnValueChanged([this](std::string, CVarWrapper cvar) { updateBoostRange(cvar.getStringValue()); });


  // Register cvar to enable/disable the stealth boost
  auto enabledVar = cvarManager->registerCvar("stealth_enabled", "1", "activate/deactivate stealth plugin");

  // Initialize plugin state according to cvars current value and add change listener
  updateEnabledState(enabledVar.getBoolValue());
  enabledVar.addOnValueChanged([this](std::string, CVarWrapper cvar) { updateEnabledState(cvar.getBoolValue()); });
}

// No need to disable the timeouts as Bakkes takes care of that.
// We also don't have to set the state to STOPPED as the whole DLL gets unloaded
// and the plugin object gets destroyed before loading it again.
void StealthPlugin::onUnload() {}

