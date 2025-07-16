#!/bin/zsh
# Caminho para o diretório de plugins SQL dentro do bundle
PROJECT_NAME=SequelFast
QTBINPATH=~/Qt/6.9.1/macos/bin
INSTAPATH=installer_macos_intel
#make clean
mkdir $INSTAPATH
cd $INSTAPATH
$QTBINPATH/qmake CONFIG+=release ..
make -j$(sysctl -n hw.logicalcpu)
# $QTBINPATH/macdeployqt $PROJECT_NAME.app -verbose=1
rm -fR "$PROJECT_NAME".dmg
# $QTBINPATH/macdeployqt $PROJECT_NAME.app -dmg
mkdir "$PROJECT_NAME"DMG
cp -R $PROJECT_NAME.app "$PROJECT_NAME"DMG/
ln -s /Applications "$PROJECT_NAME"DMG/Applications
hdiutil create -volname "$PROJECT_NAME" -srcfolder "$PROJECT_NAME"DMG -ov -format UDZO $PROJECT_NAME.dmg
rm -fR $PROJECT_NAME.app
rm -fR "$PROJECT_NAME"DMG

