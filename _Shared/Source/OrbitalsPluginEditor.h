/*
  ==============================================================================

    Orbitals Shared Plugin Editor Base Class
    Common WebView-based UI loading logic for all Orbitals plugins

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
 * Helper class for loading WebView-based plugin UIs
 * All Orbitals plugin editors can use these static methods
 */
class OrbitalsEditorHelpers
{
public:
    /**
     * Load HTML content with all CSS/JS inlined
     * @param pluginName Name of the plugin folder (e.g. "Tidal", "Apogee")
     * @return Complete HTML string ready to load in WebView
     */
    static juce::String loadPluginHTML(const juce::String& pluginName)
    {
        auto projectRoot = juce::File ("/Users/rjmacbookpro/Development/Orbitals");
        auto htmlFile = projectRoot.getChildFile(pluginName).getChildFile("UI").getChildFile("index.html");
        
        if (!htmlFile.existsAsFile())
            return {};
        
        auto htmlContent = htmlFile.loadFileAsString();
        auto uiDir = htmlFile.getParentDirectory();
        auto sharedDir = projectRoot.getChildFile("_Shared").getChildFile("UI");
        
        // Inline plugin-specific CSS
        auto cssFile = uiDir.getChildFile("styles.css");
        if (cssFile.existsAsFile())
        {
            htmlContent = htmlContent.replace("<link rel=\"stylesheet\" href=\"styles.css\">",
                                             "<style>" + cssFile.loadFileAsString() + "</style>");
        }
        
        // Inline shared design system CSS
        auto designSystemFile = sharedDir.getChildFile("orbitals-design-system.css");
        if (designSystemFile.existsAsFile())
        {
            auto designSystemContent = designSystemFile.loadFileAsString();
            
            // Replace logo image path with relative path for temp directory
            // Handle both single and double quotes
            juce::String logoPattern = "../../_Shared/Assets/logos/nnaudio-logo.png";
            juce::String logoOldPattern1 = "url('" + logoPattern + "')";
            juce::String logoOldPattern2 = "url(\"" + logoPattern + "\")";
            juce::String logoNewPattern = "url('nnaudio-logo.png')";
            designSystemContent = designSystemContent.replace (logoOldPattern1, logoNewPattern);
            designSystemContent = designSystemContent.replace (logoOldPattern2, logoNewPattern);
            
            htmlContent = htmlContent.replace("<link rel=\"stylesheet\" href=\"../../_Shared/UI/orbitals-design-system.css\">",
                                             "<style>" + designSystemContent + "</style>");
        }
        
        // Inline plugin-specific JS
        auto jsFile = uiDir.getChildFile("app.js");
        if (jsFile.existsAsFile())
        {
            htmlContent = htmlContent.replace("<script src=\"app.js\"></script>",
                                             "<script>" + jsFile.loadFileAsString() + "</script>");
        }
        
        // Inline shared JS files
        const char* sharedJSFiles[] = {
            "orbitals-animations.js",
            "orbitals-particles.js",
            "orbitals-components.js"
        };
        
        for (auto filename : sharedJSFiles)
        {
            auto jsFile = sharedDir.getChildFile(filename);
            if (jsFile.existsAsFile())
            {
                juce::String scriptTag = "<script src=\"../../_Shared/UI/" + juce::String(filename) + "\"></script>";
                htmlContent = htmlContent.replace(scriptTag, "<script>" + jsFile.loadFileAsString() + "</script>");
            }
        }
        
        // Handle background image as base64
        auto backgroundImage = projectRoot.getChildFile("_Shared/Assets/backgrounds")
                                         .getChildFile(pluginName.toLowerCase() + "-background.png");
        if (backgroundImage.existsAsFile())
        {
            juce::MemoryBlock imageData;
            if (backgroundImage.loadFileAsData(imageData))
            {
                juce::String base64 = juce::Base64::toBase64(imageData.getData(), imageData.getSize());
                juce::String relativePath = "../../_Shared/Assets/backgrounds/" + pluginName.toLowerCase() + "-background.png";
                htmlContent = htmlContent.replace(relativePath, "data:image/png;base64," + base64);
            }
        }
        
        // Disable right-click context menu
        juce::String disableRightClickScript = R"(<script>
            document.addEventListener('contextmenu', function(e) { e.preventDefault(); return false; });
            document.addEventListener('selectstart', function(e) { e.preventDefault(); return false; });
        </script>)";
        
        // Inject script before closing body tag
        if (htmlContent.contains("</body>"))
            htmlContent = htmlContent.replace("</body>", disableRightClickScript + "</body>");
        else if (htmlContent.contains("</html>"))
            htmlContent = htmlContent.replace("</html>", disableRightClickScript + "</html>");
        else
            htmlContent += disableRightClickScript;
        
        return htmlContent;
    }
};
