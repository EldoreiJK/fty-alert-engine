#!/bin/sh

#
#   Copyright (c) 2019 Eaton
#
#   This file is part of the Eaton 42ity project.
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License along
#   with this program; if not, write to the Free Software Foundation, Inc.,
#   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#
#! \file    23-alert-setup.sh
#  \brief   Set up hardcoded rules (example: warranty.rule)
#  \author  Clement Perrette <ClementPerrette@Eaton.com>
#

mkdir -p /var/lib/fty/fty-alert-engine/
ln  -s /usr/share/fty-alert-engine/data/*.rule /var/lib/fty/fty-alert-engine/

exit 0
