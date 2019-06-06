/*  =========================================================================
    alert - Alert representation

    Copyright (C) 2014 - 2018 Eaton

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

/*
@header
    alert - Alert representation
@discuss
@end
*/

#include "alert.h"
#include "fty_alert_engine_classes.h"

std::string
s_replace_tokens (
        std::string desc,
        std::string severity,
        std::string name,
        std::string ename,
        std::string logical_asset,
        std::string logical_asset_ename,
        std::string normal_state,
        std::string port)
{
    std::string rule_result = severity;
    std::transform (rule_result.begin(), rule_result.end(), rule_result.begin(), ::tolower);

    std::vector<std::string> patterns = {"__severity__", "__name__", "__ename__", "__logicalasset_iname__", "__logicalasset__", "__normalstate__", "__port__", "__rule_result__"};
    std::vector<std::string> replacements = {severity, name, ename, logical_asset, logical_asset_ename, normal_state, port, rule_result};

    std::string result = desc;
    int i = 0;
    for (auto &p : patterns)
    {
        size_t pos = 0;
        while ((pos = result.find (p, pos)) != std::string::npos){
            result.replace (pos, p.length(), replacements.at (i));
            pos += replacements.at (i).length ();
        }
        ++i;
    }

    return result;
}

void
Alert::update (fty_proto_t *msg)
{
    if (fty_proto_id (msg) != FTY_PROTO_ALERT) {
        std::string msg ("Wrong fty-proto type");
        throw std::runtime_error (msg);
    }
    std::string outcome = fty_proto_aux_string (msg, "outcome", "OK");
    m_Outcome = outcome;
    if (m_Ctime == 0)
        m_Ctime = fty_proto_time (msg);
    m_Ttl = fty_proto_ttl (msg);
    m_Severity = m_Results[outcome]["severity"][0];
    m_Description = m_Results[outcome]["description"][0];
    m_Actions = m_Results[outcome]["actions"];
}

void
Alert::overwrite (fty_proto_t *msg)
{
    if (fty_proto_id (msg) != FTY_PROTO_ALERT) {
        std::string msg ("Wrong fty-proto type");
        throw std::runtime_error (msg);
    }
    if (!isAckState (m_State))
        m_State = StringToAlertState (fty_proto_state (msg));
    m_Ctime = fty_proto_time (msg);
    m_Mtime = fty_proto_time (msg);
}

void
Alert::overwrite (Rule rule)
{
    m_Id = rule.id ();
    m_Results = rule.results ();
    m_State = RESOLVED;
    m_Outcome = "OK";
    m_Ctime = 0;
    m_Mtime = 0;
    m_Ttl = std::numeric_limits<uint64_t>::max ();
    m_Severity.clear ();
    m_Description.clear ();
    m_Actions.clear ();
}

void
Alert::cleanup ()
{
    uint64_t now = zclock_mono ()/1000;
    m_State = RESOLVED;
    m_Outcome = "OK";
    m_Ctime = now;
    m_Mtime = now;
    m_Severity.clear ();
    m_Description.clear ();
    m_Actions.clear ();
}

int
Alert::switchState (std::string state_str) {
    if (state_str == "RESOLVED") {
        // allow this transition always
        m_State = RESOLVED;
    }
    else if (state_str == "ACK-IGNORE") {
        if (m_State == RESOLVED)
            return -1;
        else
            m_State = ACKIGNORE;
    }
    else if (state_str == "ACK-PAUSE") {
        if (m_State == RESOLVED)
            return -1;
        else
            m_State = ACKPAUSE;
    }
    else if (state_str == "ACK-SILENCE") {
        if (m_State == RESOLVED)
            return -1;
        else
            m_State = ACKSILENCE;
    }
    else if (state_str == "ACK-WIP") {
        if (m_State == RESOLVED)
            return -1;
        else
            m_State = ACKWIP;
    }
    else if (state_str == "ACTIVE") {
        if (isAckState (m_State))
            return -1;
        else
            m_State = ACTIVE;
    }
    return 0;
}

zmsg_t *
Alert::toFtyProto (
        std::string ename,
        std::string logical_asset,
        std::string logical_asset_ename,
        std::string normal_state,
        std::string port)
{
    zhash_t *aux = zhash_new ();
    zhash_insert (aux, "ctime", (void *) m_Ctime);

    zlist_t *actions = zlist_new ();
    zlist_autofree (actions);
    for (auto action : m_Actions) {
        zlist_append (actions, (void *) action.c_str ());
    }

    int sep = m_Id.find ('@');
    std::string rule = m_Id.substr (0, sep-1);
    std::string name = m_Id.substr (sep+1);

    std::string description = s_replace_tokens (
            m_Description,
            m_Severity,
            name,
            ename,
            logical_asset,
            logical_asset_ename,
            normal_state,
            port);

    zmsg_t *tmp = fty_proto_encode_alert (
            aux,
            m_Mtime,
            m_Ttl,
            rule.c_str (),
            name.c_str (),
            AlertStateToString (m_State).c_str (),
            m_Severity.c_str (),
            description.c_str (),
            actions
            );

    return tmp;
}

zmsg_t *
Alert::StaleToFtyProto()
{
    zhash_t *aux = zhash_new ();
    zlist_t *actions = zlist_new ();

    int sep = m_Id.find ('@');
    std::string rule = m_Id.substr (0, sep-1);
    std::string name = m_Id.substr (sep+1);

    zmsg_t *tmp = fty_proto_encode_alert (
            aux,
            m_Mtime,
            m_Ttl,
            rule.c_str (),
            name.c_str (),
            AlertStateToString (m_State).c_str (),
            "",
            "",
            actions
            );
}

zmsg_t *
Alert::TriggeredToFtyProto()
{
    zhash_t *aux = zhash_new ();
    zhash_autofree (aux);
    zhash_insert (aux, "outcome", (void *) m_Outcome.c_str ());

    zlist_t *actions = zlist_new ();

    int sep = m_Id.find ('@');
    std::string rule = m_Id.substr (0, sep-1);
    std::string name = m_Id.substr (sep+1);

    zmsg_t *tmp = fty_proto_encode_alert (
            aux,
            m_Mtime,
            m_Ttl,
            rule.c_str (),
            name.c_str (),
            AlertStateToString (m_State).c_str (),
            "",
            "",
            actions
            );

    return tmp;
}
//  --------------------------------------------------------------------------
//  Self test of this class

// If your selftest reads SCMed fixture data, please keep it in
// src/selftest-ro; if your test creates filesystem objects, please
// do so under src/selftest-rw.
// The following pattern is suggested for C selftest code:
//    char *filename = NULL;
//    filename = zsys_sprintf ("%s/%s", SELFTEST_DIR_RO, "mytemplate.file");
//    assert (filename);
//    ... use the "filename" for I/O ...
//    zstr_free (&filename);
// This way the same "filename" variable can be reused for many subtests.
#define SELFTEST_DIR_RO "src/selftest-ro"
#define SELFTEST_DIR_RW "src/selftest-rw"

void
alert_test (bool verbose)
{
    //  @selftest
    printf (" * alert: ");
    std::string rule = "average.temperature";
    std::string name = "datacenter-3";

    // put in proper results
    std::vector<std::string> severity_critical = {"CRITICAL"};
    std::vector<std::string> severity_warning = {"WARNING"};
    std::vector<std::string> description_high_critical = {"Average temperature in __ename__ is critically high"};
    std::vector<std::string> description_high_warning = {"Average temperature in __ename__ is high"};
    std::vector<std::string> description_low_warning = {"Average temperature in __ename__ is low"};
    std::vector<std::string> description_low_critical = {"Average temperature in __ename__ is critically low"};
    std::vector<std::string> actions = {"EMAIL, SMS"};

    std::map<std::string, std::vector<std::string>> tmp1;
    std::map<std::string, std::vector<std::string>> tmp2;
    std::map<std::string, std::vector<std::string>> tmp3;
    std::map<std::string, std::vector<std::string>> tmp4;
    std::map<std::string, std::map<std::string,std::vector<std::string>>> tmp5;
    tmp1.insert (std::pair<std::string, std::vector<std::string>> ("severity", severity_critical));
    tmp1.insert (std::pair<std::string, std::vector<std::string>> ("description", description_high_critical));
    tmp1.insert (std::pair<std::string, std::vector<std::string>> ("actions", actions));
    tmp2.insert (std::pair<std::string, std::vector<std::string>> ("severity", severity_warning));
    tmp2.insert (std::pair<std::string, std::vector<std::string>> ("description", description_high_warning));
    tmp2.insert (std::pair<std::string, std::vector<std::string>> ("actions", actions));
    tmp3.insert (std::pair<std::string, std::vector<std::string>> ("severity", severity_warning));
    tmp3.insert (std::pair<std::string, std::vector<std::string>> ("description", description_low_warning));
    tmp3.insert (std::pair<std::string, std::vector<std::string>> ("actions", actions));
    tmp4.insert (std::pair<std::string, std::vector<std::string>> ("severity", severity_critical));
    tmp4.insert (std::pair<std::string, std::vector<std::string>> ("description", description_low_critical));
    tmp4.insert (std::pair<std::string, std::vector<std::string>> ("actions", actions));
    tmp5.insert (std::pair<std::string, std::map<std::string, std::vector<std::string>>> ("HIGH_CRITICAL", tmp1));
    tmp5.insert (std::pair<std::string, std::map<std::string, std::vector<std::string>>> ("HIGH_WARNING", tmp2));
    tmp5.insert (std::pair<std::string, std::map<std::string, std::vector<std::string>>> ("LOW_WARNING", tmp3));
    tmp5.insert (std::pair<std::string, std::map<std::string, std::vector<std::string>>> ("LOW_CRITICAL", tmp4));
    Alert alert (rule + "@" + name, tmp5);

    // create fty-proto msg
    {
    zhash_t *aux = zhash_new ();
    zhash_autofree (aux);
    zhash_insert (aux, "outcome", "HIGH_CRITICAL");
    zlist_t *actions = zlist_new ();

    uint64_t now = zclock_time () / 1000;
    uint64_t mtime = now;
    uint64_t ttl = 5;

    zmsg_t *msg = fty_proto_encode_alert (
            aux,
            mtime,
            ttl,
            rule.c_str (),
            name.c_str (),
            "ACTIVE",
            "",
            "",
            actions
            );
    // do update and overwrite
    alert.update (msg);
    assert (alert.m_Outcome == HIGH_CRITICAL);
    assert (alert.m_Ctime === now);
    assert (alert.m_Ttl == ttl);
    assert (alert.m_Severity == "CRITICAL");
    assert (alert.m_Description == "Average temperature in __ename__ is critically high");
    assert (alert.m_Actions == {"EMAIL", "SMS"});

    alert.overwrite (msg);
    assert (alert.m_Ctime === now);
    assert (alert.m_Mtime == now);
    assert (alert.m_State == ACTIVE);

    // switch state
    alert.switch_state ("ACK-SILENCE");
    assert (alert.m_State == ACKSILENCE);
    }

    // do toFtyProto
    zmsg_t *alert_msg =  alert.toFtyProto ("DC-Roztoky", "", "", "", "");
    zhash_t *aux = fty_proto_aux (alert_msg);
    assert (fty_proto_aux_number (aux, "ctime", "") == now);
    assert (fty_proto_aux_string (aux, "outcome", "") == "HIGH_CRITICAL");
    assert (fty_proto_time (alert_msg) == now);
    assert (fty_proto_rule (alert_msg) == rule.c_str ());
    assert (fty_proto_name (alert_msg) == name.c_str ());
    assert (fty_proto_ttl (alert_msg) == ttl);
    assert (fty_proto_severity (alert_msg) == "CRITICAL");
    assert (fty_proto_state (alert_msg) == "ACK-SILENCE");
    assert (fty_proto_description (alert_msg) == "Average temperature in DC-Roztoky is critically high");
    assert (fty_proto_actions (alert_msg) == actions);

    // create alert2 - triggered
    Alert alert2 (rule + "@" + name, tmp5);
    alert2.setOutcome ("HIGH_WARNING");
    // call TriggeredToFtyProto
    zmsg_t *alert2_msg = alert2.triggeredToFtyProto ();
    zhash_t *aux = fty_proto_aux (alert_msg);
    assert (fty_proto_aux_string (aux, "outcome", "") == "HIGH_WARNING");
    assert (fty_proto_rule (alert_msg) == rule.c_str ());
    assert (fty_proto_name (alert_msg) == name.c_str ());

    // cleanup the first alert
    alert.cleanup ();
    assert (alert.m_State == RESOLVED);
    assert (alert.m_Outcome == OK);
    assert (alert.m_Severity == "");
    assert (alert.m_Description == "");
    assert (alert.m_Actions == actions);
    // call StaleToFtyProto
    zmsg_t *alert_stale_msg = alert.staleToFtyProto ();
    zhash_t *aux = fty_proto_aux (alert_msg);
    assert (fty_proto_aux_string (aux, "outcome", "") == "OK");
    assert (fty_proto_rule (alert_msg) == rule.c_str ());
    assert (fty_proto_name (alert_msg) == name.c_str ());
    assert (fty_proto_ttl (alert_msg) == ttl);
    assert (fty_proto_severity (alert_msg) == "");
    assert (fty_proto_state (alert_msg) == "RESOLVED");
    assert (fty_proto_description (alert_msg) == "");
    assert (fty_proto_actions (alert_msg) == actions);

    // create alert3, overwrite it with a Rule
    Alert alert3 (rule + "@" + name, tmp5);
    // update it from fty-proto
    // do toFtyProto
    //  @end
    printf ("OK\n");
}
