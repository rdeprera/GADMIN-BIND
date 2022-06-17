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

extern char RESOLV_PATH_BUF[1024];


void populate_resolvers(struct w *widgets)
{
    FILE *fp;
    long file_size = 0;
    char *line, *buffy;
    gchar *utf8=NULL;
    GtkTextBuffer *text_buffer;

    text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(widgets->resolvers_textview));

    if((fp=fopen(RESOLV_PATH_BUF, "r"))==NULL)
    {
        printf("Couldnt open: %s\n", RESOLV_PATH_BUF);
        return;
    }
    fseek(fp, 0, SEEK_END);
    file_size = ftell(fp);
    rewind(fp);

    line = allocate(file_size+1);
    buffy = allocate(file_size+1);

    if( file_size > 1 )
    while(fgets(line, file_size, fp)!=NULL)
      strcat(buffy, line);

    fclose(fp);
    free(line);

    utf8 = g_locale_to_utf8(buffy, strlen(buffy), NULL, NULL, NULL);
    free(buffy);

    if( utf8!=NULL )
    {
	/* Strlen error on NULL, thats why.. */
        gtk_text_buffer_set_text(text_buffer, utf8, strlen(utf8));
        g_free(utf8);
    }
}
