#!/bin/sh
#
# written by Patrick Winnertz <patrick.winnertz@skolelinux.org> and
# Michael Meskes <meskes@debian.org>
# and placed under GPLv2
#
# this is based on a script by
# InnoTek VirtualBox
#
# Copyright (C) 2006 InnoTek Systemberatung GmbH
#
# This file is part of VirtualBox Open Source Edition (OSE), as
# available from http://www.virtualbox.org. This file is free software;
# you can redistribute it and/or modify it under the terms of the GNU
# General Public License as published by the Free Software Foundation,
# in version 2 as it comes in the "COPYING" file of the VirtualBox OSE
# distribution. VirtualBox OSE is distributed in the hope that it will
# be useful, but WITHOUT ANY WARRANTY of any kind.

PATH="/usr/bin:/bin:/usr/sbin:/sbin"
CONFIG="/etc/vbox/vbox.cfg"

if [ "$VBOX_USER_HOME" = "" ]; then
    if [ ! -d "$HOME/.VirtualBox" ]; then
        mkdir -p "$HOME/.VirtualBox"
    fi
    LOG="$HOME/.VirtualBox/VBoxSVC.log"
else
    if [ ! -d "$VBOX_USER_HOME" ]; then
        mkdir -p "$VBOX_USER_HOME"
    fi
    LOG="$VBOX_USER_HOME/VBoxSVC.log"
fi

if [ ! -r "$CONFIG" ]; then
    echo "Could not find VirtualBox installation. Please reinstall."
    exit 1
fi

. "$CONFIG_DIR/$CONFIG"

# Note: This script must not fail if the module was not successfully installed
#       because the user might not want to run a VM but only change VM params!

if [ ! -c /dev/vboxdrv ]; then
    cat << EOF
WARNING: The character device /dev/vboxdrv does not exist.
	 Please install the virtualbox-modules package for your kernel.

	 You will not be able to start VMs until this problem is fixed.
EOF
elif [ ! -w /dev/vboxdrv ]; then
    if [ "`id | grep vboxusers`" = "" ]; then
        cat << EOF
WARNING: You are not a member of the "vboxusers" group.  Please add yourself
         to this group before starting VirtualBox.

	 You will not be able to start VMs until this problem is fixed.
EOF
    else
        cat << EOF
WARNING: /dev/vboxdrv not writable for some reason. If you recently added the
         current user to the vboxusers group then you have to logout and
	 re-login to take the change effect.

	 You will not be able to start VMs until this problem is fixed.
EOF
    fi
fi

export LD_LIBRARY_PATH="$INSTALL_DIR"

APP=`which $0`
APP=${APP##/*/}
case "$APP" in
  virtualbox)
    exec "$INSTALL_DIR/VirtualBox" "$@"
  ;;
  vboxmanage)
    exec "$INSTALL_DIR/VBoxManage" "$@"
  ;;
  vboxsdl)
    exec "$INSTALL_DIR/VBoxSDL" "$@"
  ;;
  *)
    echo "Unknown application - $APP"
  ;;
esac