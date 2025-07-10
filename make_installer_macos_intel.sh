#!/bin/zsh
# Caminho para o diretório de plugins SQL dentro do bundle
QTBINPATH=~/Qt/6.9.1/macos/bin
#make clean
mkdir install_macos_intel
cd install_macos_intel
$QTBINPATH/qmake CONFIG+=release ..
make -j$(sysctl -n hw.logicalcpu)
macdeployqt SequelFast.app -verbose=1
rm -f SequelFast.dmg
$QTBINPATH/macdeployqt SequelFast.app -dmg
rm -fR SequelFast_DMG
mkdir SequelFast_DMG
cp -R SequelFast.app SequelFast_DMG/
ln -s /Applications SequelFast_DMG/Applications
hdiutil create -volname "SequelFast" -srcfolder SequelFast_DMG -ov -format UDZO SequelFast.dmg
rm -fR SequelFast.app
