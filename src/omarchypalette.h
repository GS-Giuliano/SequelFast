#pragma once

#include <QColor>
#include <QObject>

// Exposes the real, currently active color palette (Omarchy's, when
// available, otherwise a dark/light fallback) to QML, so QtQuick screens can
// bind to the exact same colors the QSS-themed Qt Widgets use instead of a
// hardcoded two-way dark/light guess. Re-reads and re-emits every color as
// soon as OmarchyThemeWatcher reports the system-wide theme changed, so an
// already-open QML dialog updates live, the same way Omarchy's quickshell
// plugins hot-reload on theme switch.
class OmarchyPalette : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool active READ active NOTIFY changed)
    Q_PROPERTY(bool dark READ dark NOTIFY changed)
    Q_PROPERTY(QColor background READ background NOTIFY changed)
    Q_PROPERTY(QColor foreground READ foreground NOTIFY changed)
    Q_PROPERTY(QColor accent READ accent NOTIFY changed)
    Q_PROPERTY(QColor accentBright READ accentBright NOTIFY changed)
    Q_PROPERTY(QColor selection READ selection NOTIFY changed)
    Q_PROPERTY(QColor field READ field NOTIFY changed)
    Q_PROPERTY(QColor fieldAlt READ fieldAlt NOTIFY changed)
    Q_PROPERTY(QColor panel READ panel NOTIFY changed)
    Q_PROPERTY(QColor panelAlt READ panelAlt NOTIFY changed)
    Q_PROPERTY(QColor panelHover READ panelHover NOTIFY changed)
    Q_PROPERTY(QColor disabledText READ disabledText NOTIFY changed)
    Q_PROPERTY(QColor handle READ handle NOTIFY changed)
    Q_PROPERTY(QColor handleLight READ handleLight NOTIFY changed)

public:
    explicit OmarchyPalette(QObject* parent = nullptr);

    bool active() const { return m_active; }
    bool dark() const { return m_dark; }
    QColor background() const { return m_background; }
    QColor foreground() const { return m_foreground; }
    QColor accent() const { return m_accent; }
    QColor accentBright() const { return m_accentBright; }
    QColor selection() const { return m_selection; }
    QColor field() const { return m_field; }
    QColor fieldAlt() const { return m_fieldAlt; }
    QColor panel() const { return m_panel; }
    QColor panelAlt() const { return m_panelAlt; }
    QColor panelHover() const { return m_panelHover; }
    QColor disabledText() const { return m_disabledText; }
    QColor handle() const { return m_handle; }
    QColor handleLight() const { return m_handleLight; }

public slots:
    void refresh();

signals:
    void changed();

private:
    bool m_active = false;
    bool m_dark = true;
    QColor m_background;
    QColor m_foreground;
    QColor m_accent;
    QColor m_accentBright;
    QColor m_selection;
    QColor m_field;
    QColor m_fieldAlt;
    QColor m_panel;
    QColor m_panelAlt;
    QColor m_panelHover;
    QColor m_disabledText;
    QColor m_handle;
    QColor m_handleLight;
};
