#pragma once

#include <QObject>

class QFileSystemWatcher;
class QTimer;

// Watches Omarchy's "current theme" state directory and emits themeChanged()
// whenever `omarchy-theme-set` swaps it (rm -rf + mv, so watched paths must be
// re-armed after every change), so the app can react to a system-wide theme
// switch immediately instead of only at next launch -- the same way Omarchy's
// own quickshell-based shell plugins hot-reload.
class OmarchyThemeWatcher : public QObject
{
    Q_OBJECT

public:
    static OmarchyThemeWatcher& instance();

signals:
    void themeChanged();

private slots:
    void onPathChanged();

private:
    explicit OmarchyThemeWatcher(QObject* parent = nullptr);

    void rearm();

    QFileSystemWatcher* m_watcher;
    QTimer* m_debounce;
};
