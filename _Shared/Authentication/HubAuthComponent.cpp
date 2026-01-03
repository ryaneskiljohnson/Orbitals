/*
  ==============================================================================

    HubAuthComponent.cpp
    Created: 18 Jan 2025 10:43:12pm
    Author:  Garrett Fleischer
    Adapted for Orbitals plugins

  ==============================================================================
*/

#include "HubAuthComponent.h"

namespace NNAudio::Authentication
{
/**
 * @brief Constructor
 * @param productId The product ID for this plugin
 * @note Product ID can be passed directly or loaded from BinaryData::product_id_txt
 */
HubAuthComponent::HubAuthComponent(const juce::String& productId)
  : m_product_id(productId.isEmpty() ? 
                 juce::String::fromUTF8(BinaryData::product_id_txt, BinaryData::product_id_txtSize) : 
                 productId)
{
  setInterceptsMouseClicks(true, false);
  setOpaque(true);

  startTimer(1);
}

/**
 * @brief Paint the authentication overlay with background, logo, and message
 */
void HubAuthComponent::paint(juce::Graphics& g)
{
  // Load images from BinaryData (will be embedded in each plugin)
  static juce::Image background = juce::ImageCache::getFromMemory(BinaryData::hub_auth_background_png,
                                                                  BinaryData::hub_auth_background_pngSize);
  static juce::Image logo = juce::ImageCache::getFromMemory(BinaryData::hub_auth_logo_png,
                                                            BinaryData::hub_auth_logo_pngSize);
  constexpr auto text_height = 24;

  const auto bounds = getLocalBounds().toFloat();
  g.drawImage(background, bounds);

  const auto logo_bounds = bounds.withSizeKeepingCentre(logo.getWidth(), logo.getHeight())
                                 .translated(0, -text_height * 2);
  g.drawImage(logo, logo_bounds);

  g.setColour(juce::Colours::white);
  g.setFont(juce::FontOptions{}.withHeight(text_height));
  g.drawMultiLineText(
    "Your plugin's authentication needs to be refreshed.\nPlease launch the NNAudio Access app to continue.",
    bounds.getX(),
    logo_bounds.getBottom() + text_height * 2,
    bounds.getWidth(),
    juce::Justification::centred,
    false);
}

/**
 * @brief Timer callback - checks authorization status periodically
 */
void HubAuthComponent::timerCallback()
{
  checkAuthorization();
}

/**
 * @brief Resize to match parent component bounds
 */
void HubAuthComponent::parentSizeChanged()
{
  setBounds(getParentComponent()->getLocalBounds());
}

/**
 * @brief Check authorization by reading license file and validating product ID
 * 
 * License file format:
 * - First token: ISO8601 expiration date
 * - Remaining tokens: comma-separated product IDs
 * 
 * If authorized: hides overlay, checks every 15 minutes
 * If not authorized: shows overlay, checks every 5 seconds
 */
void HubAuthComponent::checkAuthorization()
{
  stopTimer();
  const auto decrypted_text = loadAndDecryptFileContents();

  juce::StringArray product_list;
  product_list.addTokens(decrypted_text, ",", "\"");
  product_list.trim();
  product_list.removeEmptyStrings();

  auto expiration_date = juce::Time::getCurrentTime();
  if (!product_list.isEmpty())
    expiration_date = juce::Time::fromISO8601(product_list[0]);

  if (expiration_date > juce::Time::getCurrentTime() && product_list.contains(m_product_id))
  {
    // plugin is authorized
    setVisible(false);
    startTimer(1000 * 60 * 15); // now re-check once every 15 minutes while the plugin is running
  }
  else
  {
    toFront(true);
    setVisible(true);
    startTimer(5000);
  }
}

/**
 * @brief Get the path to the license file
 * @return File object pointing to license.dat
 * 
 * Location:
 * - macOS: ~/Library/Application Support/NNAudio Access/license.dat
 * - Windows: %APPDATA%/NNAudio Access/license.dat
 */
juce::File HubAuthComponent::getAuthFile()
{
  juce::File app_data_dir;

#if JUCE_MAC
  app_data_dir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                 .getChildFile("Application Support")
                 .getChildFile("NNAudio Access");
#else
  app_data_dir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory).
    getChildFile("NNAudio Access");
#endif

  if (!app_data_dir.exists())
    auto _ = app_data_dir.createDirectory();

  auto auth_file = app_data_dir.getChildFile("license.dat");
  return auth_file;
}

/**
 * @brief Load and decrypt license file contents
 * @return Decrypted license file contents
 * 
 * Process:
 * 1. Load file as string (Base64 encoded)
 * 2. Decode from Base64
 * 3. Decrypt using BlowFish cipher
 * 4. Convert to UTF-8 string
 */
juce::String HubAuthComponent::loadAndDecryptFileContents()
{
  const auto auth_file = getAuthFile();

  if (!auth_file.existsAsFile())
    return {};

  const auto file_contents = auth_file.loadFileAsString();

  juce::MemoryOutputStream mo;
  if (!juce::Base64::convertFromBase64(mo, file_contents))
    return {};

  juce::MemoryBlock raw_data(mo.getData(), mo.getDataSize());

  static auto key = "TrXkVijSb2SCGrHY$vPpTpq^5Vxim%4qAl@cj#iB9!1*C0jE3p&!V@Kkw9TBG&yY2fQeh7yO";
  const juce::BlowFish blow_fish(key, static_cast<int>(std::strlen(key)));

  blow_fish.decrypt(raw_data.getData(), raw_data.getSize());

  auto decrypted_text = juce::String::fromUTF8(
    static_cast<const char*>(raw_data.getData()),
    static_cast<int>(raw_data.getSize()));

  return decrypted_text;
}
}
