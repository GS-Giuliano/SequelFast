#pragma once

#include <QString>

// Builds a Qt stylesheet from the color palette of the currently active
// Omarchy theme (https://omarchy.org), so SequelFast can visually match
// whatever system-wide Omarchy theme the user has selected.
namespace OmarchyTheme
{
    // True if an active Omarchy theme's colors.toml could be found.
    bool isAvailable();

    // True if the active Omarchy theme is dark (based on background luminance).
    // Only meaningful after a successful styleSheet() call; defaults to true.
    bool isDark();

    // Returns the generated stylesheet, or an empty string if Omarchy isn't
    // installed or the active theme's colors could not be read.
    QString styleSheet();
}
