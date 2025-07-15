#!/bin/bash
PROJECT_NAME=SequelFast
QTBINPATH=~/Qt/6.9.1/gcc_64/bin
QTDIRPATH=~/Qt/6.9.1/gcc_64
INSTAPATH=installer_linux

if ! python3 --version > /dev/null 2>&1; then
	echo "Python 3 needs to be installed"
	exit 1
fi
# Requirements
if [ ! -f linuxdeployqt6.py ];then
	wget https://raw.githubusercontent.com/gavv/linuxdeployqt6.py/main/linuxdeployqt6.py
fi
rm -fR $INSTAPATH
mkdir $INSTAPATH
cd $INSTAPATH
make clean
# Build the project
$QTBINPATH/qmake CONFIG+=release ..
make -j$(nproc)
rm -fR *.o *.cpp  *.h
mkdir install
cp ../install_linux.sh ./install
cp ../uninstall_linux.sh ./install
cp ../resources/icon_1024.png ./install/icon_1024.png
cp ../org.sequelFast.SequelFast.desktop ./install/org.sequelFast.SequelFast.desktop
cp ../org.sequelFast.SequelFast.metainfo.xml ./install/org.sequelFast.SequelFast.metainfo.xml
python3 ../linuxdeployqt6.py -force -qtdir $QTDIRPATH -out-dir ./install ./$PROJECT_NAME
rm -fR $PROJECT_NAME
mv install $PROJECT_NAME
zip -r $PROJECT_NAME.zip $PROJECT_NAME
rm -fR $PROJECT_NAME
rm -fR .qm Makefile qmake*
ls -l