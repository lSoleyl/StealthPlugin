#pragma once

#include <bakkesmod/plugin/bakkesmodplugin.h>

//TODO: write plugin description
class StealthPlugin : public BakkesMod::Plugin::BakkesModPlugin {
public:
  StealthPlugin();

  /** Called whenever the plugin gets loaded
   */
  virtual void onLoad() override;

  /** Called whenever the plugin gets unloaded
   */
  virtual void onUnload() override;

private:
  /** Will setup a timeout loop to repeatedly call itself an check the stealth
   *  conditions for each car in a regular interval.
   */
  void onTick();

  /** Checks the stealth conditions for each car in the game and hides/shows 
   *  them according to the configured boost threshold.
   *
   * @pre shouldApplyStealth() == true
   */
  void applyStealth();

  /** Stealth should only be applied in locally hosted private matches. This 
   *  method returns true for locally hosted matches (not training).
   */
  bool shouldApplyStealth() const;

  /** Used to update the boostRange member from the new cvar's value
   */
  void updateBoostRange(const std::string& rangeValue);

  // Apply stealth if boost is inside this range (both ends are inclusive)
  // The values are normed between 0 and 1 for simpler computation
  std::pair<float, float> boostRange;

  // Tick duration in seconds if plugin is active and in a locally hosted lobby
  static const float ACTIVE_TICK_DURATION;

  // Tick duration in seconds if plugin is active, but not in a suitable game lobby
  static const float IDLE_TICK_DURATION;
};
