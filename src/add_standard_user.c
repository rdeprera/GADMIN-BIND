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
#include "gettext.h"
#include "widgets.h"
#include "commands.h"
#include "functions.h"
#include "add_standard_user.h"
#include "show_info.h"



int add_standard_user()
{
    /* Adds the user/group NAMED_USER if it doesnt exist and locks the account */
    gchar *cmd, *info, *username;

    username = g_strdup_printf("%s", NAMED_USER);

    /* Add the system group NAMED_USER if it doesnt exist */
    if( ! group_exists(username) )
    {
        cmd = g_strdup_printf("%s %s", GROUPADD_BINARY, username);
        if( ! run_command(cmd) )
        {
	    info = g_strdup_printf(_("Error: Failed to add the system group:\n%s\nwith command: %s\n"), username, cmd);
    	    show_info(info);
    	    g_free(info);
    	    g_free(cmd);
    	    g_free(username);
            return 0;
        }
        g_free(cmd);
    }


    if( ! user_exists(username) )
    {
        /* Add the NAMED_USER user to the NAMED_USER group and lock the account */
        cmd = g_strdup_printf("%s '%s' -l -d '/dev/null' -c 'Nobody' -s '/dev/null' -g '%s'", USERADD_BINARY, username, username);
        if( ! run_command(cmd) )
        {
    	    info = g_strdup_printf(_("Error: Failed to add the system user:\n%s\nwith command: %s\n"), username, cmd);
    	    show_info(info);
    	    g_free(info);
            g_free(cmd);
            g_free(username);
            return 0;
        }
        g_free(cmd);

	/* To be really sure we check it again with the same function */
	if( ! user_exists(username) )
	{
    	    info = g_strdup_printf(_("Error: Failed to add the system user:\n%s\n"), username);
    	    show_info(info);
    	    g_free(info);
    	    g_free(username);
    	    return 0;
	}
    }

    g_free(username);

    return 1;
}
