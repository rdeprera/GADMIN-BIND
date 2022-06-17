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
#include "gettext.h"
#include <gtk/gtk.h>
#include "widgets.h"
#include "show_info.h"
#include "commands.h"
#include "rndc_reload.h"

extern int activated;



void rndc_reload(struct w *widgets)
{
    gchar *info, *cmd;

    if( ! activated )
    {
	info = g_strdup_printf(_("The server must be activated in order to reload its zone and configuration data.\n"));
	show_info(info);
	g_free(info);
	return;
    }
    
    cmd = g_strdup_printf("%s reload", RNDC_BINARY);
    if( ! run_command(cmd) )
    {
	info = g_strdup_printf(_("Error: could not run rndc reload, maybe you need to generate\na secret key in the settings and restart the server ?\n"));
	show_info(info);
	g_free(info);
    }
    else
      {
	  info = g_strdup_printf(_("The servers zones and configuration data has been successfully reloaded.\n"));
	  show_info(info);
	  g_free(info);
      }    
}
