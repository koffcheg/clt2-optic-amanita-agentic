#!/bin/sh

install -D -t /usr/lib/turretgui/ turretgui turretgui.json turretgui-log.xml
ln -sf /usr/lib/turretgui/turretgui /usr/bin/turretgui

desktop-file-install --dir=/usr/share/applications turretgui.desktop
install turretgui.png /usr/share/icons/hicolor/128x128/apps/turretgui.png

gtk-update-icon-cache /usr/share/icons/hicolor
update-desktop-database /usr/share/applications