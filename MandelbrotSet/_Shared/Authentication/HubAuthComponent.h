/*
  ==============================================================================

    HubAuthComponent.h
    Created: 18 Jan 2025 10:43:12pm
    Author:  Garrett Fleischer
    Adapted for Orbitals plugins

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

namespace NNAudio::Authentication
{
/**
 * @fileoverview HubAuthComponent - Authentication overlay for Orbitals plugins
 * @module Orbitals Authentication
 * 
 * Displays an authentication overlay when the plugin requires license validation.
 * Checks license file periodically and shows/hides the overlay based on authorization status.
 */
class HubAuthComponent : public juce::Component, public juce::Timer
{
public:
  /**
   * @brief Constructor
   * @param productId The product ID string for this plugin (e.g., "tidal", "apogee")
   */
  HubAuthComponent(const juce::String& productId);

  /**
   * @brief Paint the authentication overlay
   * @param g Graphics context
   */
  virtual void paint(juce::Graphics& g) override;

private:
  const juce::String m_product_id;
  
  /**
   * @brief Timer callback - checks authorization periodically
   */
  virtual void timerCallback() override;
  
  /**
   * @brief Called when parent size changes - resizes to match parent
   */
  virtual void parentSizeChanged() override;

  /**
   * @brief Check if plugin is authorized by reading and validating license file
   * @note Sets visibility and timer interval based on authorization status
   */
  void checkAuthorization();

  /**
   * @brief Get the path to the license file
   * @return File object pointing to license.dat in NNAudio Access directory
   * @note Creates directory if it doesn't exist
   */
  static juce::File getAuthFile();
  
  /**
   * @brief Load and decrypt the license file contents
   * @return Decrypted license file contents as string, or empty if file doesn't exist/invalid
   * @note File is Base64 encoded and BlowFish encrypted
   */
  static juce::String loadAndDecryptFileContents();

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HubAuthComponent)
};
}
