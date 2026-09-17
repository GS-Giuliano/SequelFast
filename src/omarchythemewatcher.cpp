#include "omarchythemewatcher.h"
#include "omarchytheme.h"

#include <QDir>
#include <QFile>
#include <QFileSystemWatcher>
#include <QTimer>

OmarchyThemeWatcher& OmarchyThemeWatcher::instance()
{
    static OmarchyThemeWatcher watcher;
    return watcher;
}

OmarchyThemeWatcher::OmarchyThemeWatcher(QObject* parent)
    : QObject(parent)
    , m_watcher(new QFileSystemWatcher(this))
    , m_debounce(new QTimer(this))
{
    m_debounce->setSingleShot(true);
    m_debounce->setInterval(150);
    connect(m_debounce, &QTimer::timeout, this, &OmarchyThemeWatcher::themeChanged);

    connect(m_watcher, &QFileSystemWatcher::directoryChanged, this, &OmarchyThemeWatcher::onPathChanged);
    connect(m_watcher, &QFileSystemWatcher::fileChanged, this, &OmarchyThemeWatcher::onPathChanged);

    rearm();
}

void OmarchyThemeWatcher::onPathChanged()
{
    // `omarchy-theme-set` replaces the watched directory/file (rm -rf + mv), so
    // QFileSystemWatcher silently drops the now-gone paths; re-add whatever
    // exists again on every change, then let observers re-read the new colors.
    rearm();
    m_debounce->start();
}

void OmarchyThemeWatcher::rearm()
{
    if (!m_watcher->directories().isEmpty())
        m_watcher->removePaths(m_watcher->directories());
    if (!m_watcher->files().isEmpty())
        m_watcher->removePaths(m_watcher->files());

    const QString currentDir = OmarchyTheme::currentStateDir();
    QDir parent(currentDir);
    parent.cdUp();

    if (parent.exists())
        m_watcher->addPath(parent.absolutePath());
    if (QDir(currentDir).exists())
        m_watcher->addPath(currentDir);

    const QString themeDir = currentDir + "/theme";
    if (QDir(themeDir).exists()) {
        m_watcher->addPath(themeDir);

        const QString colorsFile = themeDir + "/colors.toml";
        if (QFile::exists(colorsFile))
            m_watcher->addPath(colorsFile);
    }
}
