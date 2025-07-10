0QT       += core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    connection.cpp \
    functions.cpp \
    main.cpp \
    mainwindow.cpp \
    sql.cpp \
    sqlhighlighter.cpp \
    statistics.cpp \
    structure.cpp \
    tunnelsqlmanager.cpp

HEADERS += \
    connection.h \
    functions.h \
    mainwindow.h \
    sql.h \
    sqlhighlighter.h \
    statistics.h \
    structure.h \
    tunnelsqlmanager.h

FORMS += \
    connection.ui \
    mainwindow.ui \
    sql.ui \
    statistics.ui \
    structure.ui

TRANSLATIONS += \
    SequelFast_pt_BR.ts
CONFIG += lrelease
CONFIG += embed_translations
CONFIG += app_bundle # (This is the default, so this line is often omitted)

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    icons.qrc

# Ícone do executável no Windows
win32 {
    RC_FILE = icons/app_icon.rc
}

# Ícone do app no macOS
mac {
    ICON = resources/icon.icns

}
