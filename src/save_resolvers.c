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
#include "show_info.h"

extern char RESOLV_PATH_BUF[1024];



void save_resolvers(struct w *widgets)
{
    /* Save the resolvers. */
    FILE *fp;
    GtkTextBuffer *resolvers_textbuffer;
    GtkTextIter start, end;
    gchar *info = NULL;
    gchar *conf = NULL;

    if((fp=fopen(RESOLV_PATH_BUF, "w+"))==NULL)
    {
	info = g_strdup_printf(_("Error: can not write resolv.conf here: %s\n"), RESOLV_PATH_BUF);
	show_info(info);
	g_free(info);
	return;
    }

    resolvers_textbuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(widgets->resolvers_textview));
    gtk_text_buffer_get_bounds(resolvers_textbuffer, &start, &end);
    conf = gtk_text_iter_get_text(&start, &end);

    fputs(conf, fp);
    fclose(fp);

    if( conf!=NULL )
      g_free(conf);
}
