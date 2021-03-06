/*  =========================================================================
    fty-alert-engine - 42ity service evaluating rules written in Lua and producing alerts

    Copyright (C) 2019 - 2019 Eaton

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
    =========================================================================
*/

#ifndef FTY_ALERT_ENGINE_H_H_INCLUDED
#define FTY_ALERT_ENGINE_H_H_INCLUDED

//  Include the project library file
#include "fty_alert_engine_library.h"

//  Add your own public definitions here, if you need them
#define RULES_SUBJECT "rfc-evaluator-rules"
#define LIST_RULE_MB "RULE_HANDLING"

/// config path
#define CONFIG_FILE "/etc/fty-alert-engine/fty-alert-engine.cfg"
/// path to the directory, where rules are stored. Attention: without last slash!
#define RULE_PATH_DEFAULT "/var/lib/fty/fty-alert-engine"
/// path to the directory, where templates are stored. Attention: without last slash!
#define TEMPLATE_PATH_DEFAULT "/usr/share/bios/fty-autoconfig"
/// default timeout [ms]
#define DEFAULT_TIMEOUT "30000"

/// trigger name
//TODO: FIXME: renamed to use old names, after release should be changed properly
//#define TRIGGER_AGENT_NAME_MAILBOX "fty-alert-trigger"
//#define TRIGGER_AGENT_NAME_STREAM "fty-alert-trigger-stream"
#define TRIGGER_AGENT_NAME_MAILBOX "fty-alert-engine"
#define TRIGGER_AGENT_NAME_STREAM "fty-alert-engine-stream"

/// config name
//TODO: FIXME: renamed to use old names, after release should be changed properly
//#define CONFIG_AGENT_NAME "fty-alert-config"
#define CONFIG_AGENT_NAME "fty-autoconfig"

/// list name
#define LIST_AGENT_NAME "fty-alert-list"

/// malamute endpoint
#define ENDPOINT "ipc://@/malamute"

#endif
