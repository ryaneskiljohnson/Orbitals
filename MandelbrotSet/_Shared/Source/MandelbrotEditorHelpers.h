/*
  ==============================================================================

    Mandelbrot Set Shared Plugin Editor Base Class
    Common WebView-based UI loading logic for all Mandelbrot Set plugins

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
 * Helper class for loading WebView-based plugin UIs
 * All Mandelbrot Set plugin editors can use these static methods
 */
class MandelbrotEditorHelpers
{
public:
    /**
     * Load HTML content with all CSS/JS inlined
     * @param pluginName Name of the plugin folder (e.g. "Singularity", "Fabric")
     * @return Complete HTML string ready to load in WebView
     */
    static juce::String loadPluginHTML(const juce::String& pluginName)
    {
        auto projectRoot = juce::File ("/Users/rjmacbookpro/Development/Orbitals/MandelbrotSet");
        auto htmlFile = projectRoot.getChildFile(pluginName).getChildFile("UI").getChildFile("index.html");
        
        if (!htmlFile.existsAsFile())
            return {};
        
        auto htmlContent = htmlFile.loadFileAsString();
        auto uiDir = htmlFile.getParentDirectory();
        auto sharedDir = projectRoot.getChildFile("_Shared").getChildFile("UI");
        
        // Handle background image as base64 first (needed for CSS replacement)
        juce::String backgroundDataURL;
        auto backgroundImage = projectRoot.getChildFile("_Shared/Assets/backgrounds")
                                         .getChildFile(pluginName.toLowerCase() + ".png");
        if (backgroundImage.existsAsFile())
        {
            juce::MemoryBlock imageData;
            if (backgroundImage.loadFileAsData(imageData))
            {
                juce::String base64 = juce::Base64::toBase64(imageData.getData(), imageData.getSize());
                backgroundDataURL = "data:image/png;base64," + base64;
            }
        }
        
        // Inline plugin-specific CSS
        auto cssFile = uiDir.getChildFile("styles.css");
        if (cssFile.existsAsFile())
        {
            auto cssContent = cssFile.loadFileAsString();
            
            // Replace background image path in CSS with base64 data URL
            if (backgroundDataURL.isNotEmpty())
            {
                juce::String bgPattern = "../../_Shared/Assets/backgrounds/" + pluginName.toLowerCase() + ".png";
                juce::String bgOldPattern1 = "url('" + bgPattern + "')";
                juce::String bgOldPattern2 = "url(\"" + bgPattern + "\")";
                cssContent = cssContent.replace(bgOldPattern1, "url('" + backgroundDataURL + "')");
                cssContent = cssContent.replace(bgOldPattern2, "url(\"" + backgroundDataURL + "\")");
            }
            
            htmlContent = htmlContent.replace("<link rel=\"stylesheet\" href=\"styles.css\">",
                                             "<style>" + cssContent + "</style>");
        }
        
        // Inline shared design system CSS
        auto designSystemFile = sharedDir.getChildFile("mandelbrot-design-system.css");
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
            
            htmlContent = htmlContent.replace("<link rel=\"stylesheet\" href=\"../../_Shared/UI/mandelbrot-design-system.css\">",
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
            "mandelbrot-animations.js",
            "mandelbrot-particles.js",
            "mandelbrot-components.js"
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
        
        // Background image already handled above in CSS inlining
        
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
