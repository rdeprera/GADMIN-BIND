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
#include <string.h>
#include "widgets.h"
#include "gettext.h"
#include "commands.h"
#include "show_info.h"

extern int activated;



void deactivate_button_clicked(struct w *widgets)
{
    gchar *stop, *info;

    if( ! activated )
      return;

    stop = g_strdup_printf("killall -9 %s", NAMED_BINARY);
    if( ! run_command(stop) )
    {
	info = g_strdup_printf("Stopping named failed with this command:\n%s\n", stop);
	show_info(info);
	g_free(info);
    }
    g_free(stop);

    /* Do not start the server at boot */
    if( ! strstr(SYSINIT_STOP_CMD, "none") )
    {
	if( ! run_command(SYSINIT_STOP_CMD) )
	{
	    // Fixme, not translated...
	    info = g_strdup_printf(_("Error: Could not run SYSINIT_STOP_CMD.\n"));
	    show_info(info);
	    g_free(info);
	}
    }
}
