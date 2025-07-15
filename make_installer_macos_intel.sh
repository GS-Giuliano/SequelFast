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
macdeployqt $PROJECT_NAME.app -verbose=1
rm -f $PROJECT_NAME.dmg
$QTBINPATH/macdeployqt $PROJECT_NAME.app -dmg
rm -fR $PROJECT_NAME_DMG
mkdir $PROJECT_NAME_DMG
cp -R $PROJECT_NAME.app $PROJECT_NAME_DMG/
ln -s /Applications $PROJECT_NAME_DMG/Applications
hdiutil create -volname "$PROJECT_NAME" -srcfolder $PROJECT_NAME_DMG -ov -format UDZO $PROJECT_NAME.dmg
rm -fR $PROJECT_NAME.app

