# 0.2.3
- Query errors are now added as comment in query editor
- Database connection improvements
- Connection dialog rewritten in QML (replaces the old Qt Widgets form)
- New Omarchy theme that follows the colors of the active system-wide Omarchy theme, with automatic fallback when Omarchy isn't installed
- App now uses the system's default font instead of a bundled custom font
- Fixed alternating row colors being unreadable in the light theme (invalid CSS in the stylesheet was silently breaking the whole theme)
- Linux releases are now fully self-contained (bundled Qt/glibc), runnable on any distro regardless of installed Qt version
- Added instructions for installing SequelFast's app menu entry on Omarchy Linux