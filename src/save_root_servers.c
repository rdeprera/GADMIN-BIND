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
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "widgets.h"
#include "gettext.h"
#include "commands.h"
#include "populate_root_servers_tab.h"
#include "allocate.h"

extern char CHROOT_PATH_BUF[1024];

extern int activated;



void save_root_servers(struct w *widgets)
{
    /* Save the root servers tab. */
    FILE *fp;
    GtkTextBuffer *rs_textbuffer;
    GtkTextIter start, end;
    gchar *conf = NULL;
    gchar *utf8 = NULL;
    gchar *root_hints;

    rs_textbuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(widgets->rs_textview));
    gtk_text_buffer_get_bounds(rs_textbuffer, &start, &end);

    conf = gtk_text_iter_get_text(&start, &end);

    root_hints = g_strdup_printf("%s/etc/root.hints", CHROOT_PATH_BUF);

    if((fp=fopen(root_hints, "w+"))==NULL)
    {
        if( conf!=NULL )
    	  g_free(conf);

	if( utf8!=NULL )
          g_free(utf8);

	g_free(root_hints);
	return;
    }
    fputs(conf, fp);
    fclose(fp);

    g_free(root_hints);

    if( conf!=NULL )
      g_free(conf);

    if( utf8!=NULL )
      g_free(utf8);

    populate_root_servers_tab(widgets);
}
