#pragma once

#include <bakkesmod/plugin/bakkesmodplugin.h>


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

  /** Sets all cars visible. This is called just when the onTick() loop is stopped 
   *  by setting stealth_enabled = 0 midgame.
   *  The car visibility is only changed if this is a locally hosted match.
   */
  void showAllCars();

  /** Stealth should only be applied in locally hosted private matches. This 
   *  method returns true for locally hosted matches (not training).
   */
  bool shouldApplyStealth() const;

  /** Used to update the boostRange member from the new cvar's value
   */
  void updateBoostRange(const std::string& rangeValue);

  /** Updates the plugin's enabled state according to the cvar's current value 
   *  This will start/stop the onTick timeout loop if necessary
   */
  void updateEnabledState(bool enabled);

  // Apply stealth if boost is inside this range (both ends are inclusive)
  // The values are normed between 0 and 1 for simpler computation
  std::pair<float, float> boostRange;



  /** This helper function is my deperate attempt to prevent RL from crashing right after a player
   *  joins the game. I looked into the crashdumps and it crashes inside BoostWrapper::GetPercentBoostFull() 
   *  because some pointer is null, so I want to check this beforehand.
   *
   * @param boostComponent a car's boost component
   *
   * @return true if the component seems to be valid, false otherwise
   */
  static bool isBoostComponentValid(const BoostWrapper& boostComponent);


  /** State enum to properly handle the plugin statethe plugin and to not accidentially
   *  start multiple timeouts when disabling/enabling the plugin quickly.
   */
  enum class State { STOPPED, RUNNING, STOP_REQUESTED } state;
  

  // Tick duration in seconds if plugin is active and in a locally hosted lobby
  static const float ACTIVE_TICK_DURATION;

  // Tick duration in seconds if plugin is active, but not in a suitable game lobby
  static const float IDLE_TICK_DURATION;
};
