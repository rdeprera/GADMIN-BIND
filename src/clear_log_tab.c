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

extern char SYSLOG_PATH_BUF[1024];



void clear_log_tab(struct w *widgets)
{
    FILE *fp;
    char *line, *new_syslog;
    long file_size = 0;
    gchar *info;

    /* Collect every line in syslog thats not referring to named */    
    if((fp=fopen(SYSLOG_PATH_BUF, "r"))==NULL)
    {
        info = g_strdup_printf(_("Couldnt open system log here: %s\n"), SYSLOG_PATH_BUF);
        show_info(info);
        g_free(info);
        return;
    }

    fseek(fp, 0, SEEK_END);
    file_size = ftell(fp);
    rewind(fp);

    line = allocate(file_size+16384);
    new_syslog = allocate(file_size+16384);

    if( file_size > 1 )
    while(fgets(line, file_size, fp)!=NULL)
      if( ! strstr(line, "named") )
        strcat(new_syslog, line);

    fclose(fp);    

    /* Write the modified syslog */
    if((fp=fopen(SYSLOG_PATH_BUF, "w+"))==NULL)
    {
        info = g_strdup_printf(_("Could not write system log here: %s\n"), SYSLOG_PATH_BUF);
        show_info(info);
        g_free(info);
	free(line);
	free(new_syslog);
        return;
    }
    fputs(new_syslog, fp);
    free(line);
    free(new_syslog);
    fclose(fp);    

    populate_log_tab(widgets);
}
