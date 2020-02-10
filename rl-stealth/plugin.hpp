#pragma once

#include <bakkesmod/plugin/bakkesmodplugin.h>

//TODO: write plugin description
class StealthPlugin : public BakkesMod::Plugin::BakkesModPlugin {
public:

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
  void onTick() const;

  /** Checks the stealth conditions for each car in the game and hides/shows 
   *  them according to the configured boost threshold.
   *
   * @pre shouldApplyStealth() == true
   */
  void applyStealth() const;

  /** Stealth should only be applied in locally hosted private matches. This 
   *  method returns true for locally hosted matches (not training).
   */
  bool shouldApplyStealth() const;
};
