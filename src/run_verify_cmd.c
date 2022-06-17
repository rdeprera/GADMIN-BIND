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
#include <stdlib.h>
#include <string.h>
#include "allocate.h"
#include "widgets.h"
#include "gettext.h"
#include "show_info.h"
#include "populate_log_tab.h"

#define MAX_READ_POPEN 16384


/* FIXME: Start a timer to break long running operations */
void run_verify_cmd(struct w *widgets)
{
    /* Shows command output in the verify tab */
    FILE *fp;
    char *line, *buffy;
    gchar *info, *utf8=NULL;
    gchar *cmd;
    GtkTextBuffer *text_buffer=NULL;
    G_CONST_RETURN gchar *entry_text;
        
    text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(widgets->verify_textview));
    entry_text = gtk_entry_get_text(GTK_ENTRY(widgets->verify_run_cmd_entry));

    cmd = g_strdup_printf("%s", entry_text);
    if((fp=popen(cmd, "r"))==NULL)
    {
        info = g_strdup_printf(_("Could not run command: %s\n"), cmd);
        show_info(info);
        g_free(info);
        
	g_free(cmd);
        return;
    }

    buffy = allocate(MAX_READ_POPEN+1);
    line = allocate(MAX_READ_POPEN+1);

    while(fgets(line, MAX_READ_POPEN, fp)!=NULL)
      strcat(buffy, line);

    fclose(fp);
    g_free(cmd);
    free(line);
                  
    utf8 = g_locale_to_utf8(buffy, strlen(buffy), NULL, NULL, NULL);
    free(buffy);
                                          
    if( utf8!=NULL )
    {
        gtk_text_buffer_set_text(text_buffer, utf8, strlen(utf8));
        g_free(utf8);
    }
}
