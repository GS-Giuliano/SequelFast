#!/bin/zsh
# Caminho para o diretório de plugins SQL dentro do bundle
QTBINPATH=~/Qt/6.9.1/macos/bin

#make clean
$QTBINPATH/qmake CONFIG+=release
make -j$(sysctl -n hw.logicalcpu)

#cd build/Desktop-Debug
#SQL_PLUGIN_DIR="SequelFast.app/Contents/PlugIns/sqldrivers"
rm -f SequelFast.dmg
$QTBINPATH/macdeployqt SequelFast.app -dmg
rm -fR SequelFast.app
