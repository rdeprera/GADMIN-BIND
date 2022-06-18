/* GADMIN-BIND - An easy to use GTK+ frontend for the ISC BIND DNS Server.
 * Copyright (C) 2008 - 2010 Magnus Loef <magnus-swe@telia.com> 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
*/



#include "../config.h"
#include <gtk/gtk.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "widgets.h"
#include "gettext.h"
#include "commands.h"
#include "chars_are_digits.h"
#include "clear_log_tab.h"
#include "populate_log_tab.h"
#include "activate_button_clicked.h"

extern int activated;
extern char CHROOT_PATH_BUF[1024];


void write_sysinit_script()
{
    FILE *fp;
    gchar *cmd, *script, *script_path;
    
    script_path = g_strdup_printf("%s/%s", SYSINIT_SCRIPTS_DIR, SYSINIT_SCRIPT_NAME);

    if((fp=fopen(script_path, "w+"))==NULL)
    {
        printf("Could not write the sysinit script here: %s\n", script_path);
	g_free(script_path);
        return;
    }
    script = g_strconcat(
    "#!/bin/bash\n",
    "#\n",
    "#\n",
    "# chkconfig: 2345 10 30\n",
    "#\n",
    "# processname: gadmin-bind\n",
    "# description: This shell script takes care of starting and stopping \\n",
    "# gadmin-bind.\n",
    "#\n",
    "### BEGIN INIT INFO\n",
    "# Provides: gadmin-bind\n",
    "# Required-Start: network NetworkManager\n",
    "# Required-Stop: NetworkManager\n",
    "# Default-Start: 2 3 4 5\n",
    "# Default-Stop: 0 1 6\n",
    "# Short-Description: start and stop gadmin-bind\n",
    "# Description: GAdmin-BIND starts and stops bind.\n",
    "### END INIT INFO\n",
    "\n",
    "\n",
    "case \"$1\" in\n",
    "		start)\n",
    "\n",
    NAMED_BINARY, " -u ", NAMED_USER, " -t ", CHROOT_PATH_BUF, " -c ", BIND_CONF, "\n",
    "\n",
    ";;\n",
    "\n",
    "		stop)\n",
    "\n",
    "killall -9 named\n",
    "\n",
    ";;\n",
    "		*)\n",
    "echo \"usage gadmin-bind {start|stop}\"\n",
    "exit 1\n",
    ";;\n",
    "esac\n",
    "\n",
    "exit 0\n",
    "\n",
    NULL);

    fputs(script, fp);
    fclose(fp);
    g_free(script);

    /* Chmod the script to 755 */
    cmd = g_strdup_printf("chmod 755 %s", script_path);
    if( ! run_command(cmd) )
    {
    	printf("Error chmodding the sysinit script.\n");
    }
    g_free(cmd);

    g_free(script_path);
}


void activate_button_clicked(struct w *widgets)
{
    gchar *start, *test, *info;

    if( activated )
      return;

    clear_log_tab(widgets);

    start = g_strdup_printf("%s -u %s -t %s -c %s", NAMED_BINARY, NAMED_USER, CHROOT_PATH_BUF, BIND_CONF);
    if( ! run_command(start) )
    {
	/* Shows any errors QA:d */
        test = g_strdup_printf("%s -fg -u %s -t %s -c %s 2>&1", NAMED_BINARY, NAMED_USER, CHROOT_PATH_BUF, BIND_CONF);
	run_command_show_err(test);
	g_free(test);
	activated = 0;
	return;
    }
    g_free(start);

    activated = 1;

    if( ! chars_are_digits(CMD_SLEEP_LOG_SECS) )
    {
	info = g_strdup_printf(_("CMD_SLEEP_LOG_SECS should be in seconds, please reinstall the program.\n"));
	show_info(info);
	g_free(info);
	return;
    }
    if( strlen(CMD_SLEEP_LOG_SECS) > 2 ) /* 99 seconds max */
    {
	info = g_strdup_printf(_("CMD_SLEEP_LOG_SECS max length is > 2 chars, please reinstall the program.\n"));
	show_info(info);
	g_free(info);
	return;
    }

    sleep(atoi(CMD_SLEEP_LOG_SECS));

    populate_log_tab(widgets);

    /* Write the sysinit script */
    write_sysinit_script();

    /* Start the server at boot */
    if( ! strstr(SYSINIT_START_CMD, "none") )
    {
	if( ! run_command(SYSINIT_START_CMD) )
	{
	    // Fixme, not translated...
	    info = g_strdup_printf(_("Error: Could not make the server start at boot.\n"));
	    show_info(info);
	    g_free(info);
	}
    }
}
