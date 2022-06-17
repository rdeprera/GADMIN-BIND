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



#include <gtk/gtk.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "widgets.h"
#include "gettext.h"
#include "allocate.h"
#include "show_info.h"

extern int MAX_READ_POPEN;



void set_version(struct w *widgets)
{
    FILE *fp;
    char *version;
    int ver_found=0;
    gchar *command;
    gchar *ver=NULL;
    gchar *utf8=NULL;
    gchar *info;
    GdkColor color;

    command = g_strdup_printf("%s -v 2>&1", NAMED_BINARY);
    if((fp=popen(command, "r"))==NULL)
    {
       info = g_strdup_printf(_("Set version failed\n"));
       show_info(info);
       g_free(info);
       g_free(command);
       return;
    }
    fflush(fp);

    version = allocate(MAX_READ_POPEN);

    while(fgets(version, MAX_READ_POPEN, fp)!=NULL)
    {
       if( strstr(version, "BIND") && strlen(version) < 100 )
       {
          ver = g_strdup_printf(_(" Information: %s"), version);
	  utf8 = g_locale_to_utf8(ver, strlen(ver)-1, NULL, NULL, NULL);
          gtk_label_set_text(GTK_LABEL(widgets->version_label), utf8);
	  g_free(utf8);
	  g_free(ver);
	  ver_found = 1;
	  break;
       }
    }
    free(version);
    g_free(command);    
    pclose(fp);

    if( ! ver_found )
    {
        ver = g_strdup_printf(_(" ISC BIND is not installed or not in your path."));
	utf8 = g_locale_to_utf8(ver, strlen(ver), NULL, NULL, NULL);
        gtk_label_set_text(GTK_LABEL(widgets->version_label), utf8);
	g_free(utf8);
	g_free(ver);

	/* Set status color */
	gdk_color_parse("red", &color);
	gtk_widget_modify_fg(widgets->version_label, GTK_STATE_NORMAL, &color);
    }
}
