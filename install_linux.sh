#!/bin/bash
sudo apt -y install libxcb-cursor0 libxcb-cursor-dev libgl1-mesa-dev
mkdir -p ~/Applications/SequelFast
cp -fR * ~/Applications/SequelFast
mkdir -p ~/.local/share/applications
mkdir -p ~/.local/share/icons/hicolor/1024x1024/apps/
sed -i "s/USER/$USER/g" org.sequelFast.SequelFast.desktop
cp -fR icon_1024.png ~/.local/share/icons/hicolor/1024x1024/apps/org.sequelFast.SequelFast.png
cp -fR org.sequelFast.SequelFast.desktop ~/.local/share/applications/org.sequelFast.SequelFast.desktop
chmod +x ~/.local/share/applications/org.sequelFast.SequelFast.desktop
update-desktop-database ~/.local/share/applications/