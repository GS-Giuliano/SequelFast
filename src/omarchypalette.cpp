#include "omarchypalette.h"
#include "omarchythemewatcher.h"
#include "omarchytheme.h"

namespace {

// Fallback palette used when no Omarchy theme is installed/readable, mirroring
// the hardcoded colors QML dialogs used before they could read the real theme.
struct FallbackPalette
{
    QColor background, foreground, accent, accentBright, selection, field,
        fieldAlt, panel, panelAlt, panelHover, disabledText, handle, handleLight;
};

FallbackPalette fallbackPalette(bool dark)
{
    if (dark) {
        return FallbackPalette{
            QColor("#192028"), QColor("#DFE1E2"), QColor("#1A72BB"), QColor("#3D8FDA"),
            QColor("#346792"), QColor("#253344"), QColor("#2B3A4D"), QColor("#212B36"),
            QColor("#28323F"), QColor("#33404F"), QColor("#8A97A3"), QColor("#4E5E6E"),
            QColor("#5C6E80"),
        };
    }
    return FallbackPalette{
        QColor("#F4F4F4"), QColor("#19232D"), QColor("#73C7FF"), QColor("#4FA8E8"),
        QColor("#9FCBFF"), QColor("#FFFFFF"), QColor("#F0F0F0"), QColor("#E9E9E9"),
        QColor("#E0E0E0"), QColor("#D6D6D6"), QColor("#6E7A85"), QColor("#B7C0C8"),
        QColor("#A2ADB6"),
    };
}

} // namespace

// Set once at startup (and on every theme change) in MainWindow::changeTheme(),
// this is the single source of truth for whether the active look is dark.
extern bool currentThemeIsDark;

OmarchyPalette::OmarchyPalette(QObject* parent)
    : QObject(parent)
{
    refresh();
    connect(&OmarchyThemeWatcher::instance(), &OmarchyThemeWatcher::themeChanged, this, &OmarchyPalette::refresh);
}

void OmarchyPalette::refresh()
{
    const QMap<QString, QColor> t = OmarchyTheme::tokens();
    m_active = !t.isEmpty();
    // tokens() recomputes OmarchyTheme::isDark() as a side effect, so prefer it
    // over the (possibly not-yet-refreshed) global when Omarchy is the active
    // theme; otherwise fall back to the app's own manual dark/light choice.
    m_dark = m_active ? OmarchyTheme::isDark() : currentThemeIsDark;

    if (m_active) {
        m_background = t.value("background");
        m_foreground = t.value("foreground");
        m_accent = t.value("accent");
        m_accentBright = t.value("accentBright");
        m_selection = t.value("selection");
        m_field = t.value("field");
        m_fieldAlt = t.value("fieldAlt");
        m_panel = t.value("panel");
        m_panelAlt = t.value("panelAlt");
        m_panelHover = t.value("panelHover");
        m_disabledText = t.value("disabledText");
        m_handle = t.value("handle");
        m_handleLight = t.value("handleLight");
    } else {
        const FallbackPalette f = fallbackPalette(m_dark);
        m_background = f.background;
        m_foreground = f.foreground;
        m_accent = f.accent;
        m_accentBright = f.accentBright;
        m_selection = f.selection;
        m_field = f.field;
        m_fieldAlt = f.fieldAlt;
        m_panel = f.panel;
        m_panelAlt = f.panelAlt;
        m_panelHover = f.panelHover;
        m_disabledText = f.disabledText;
        m_handle = f.handle;
        m_handleLight = f.handleLight;
    }

    emit changed();
}
