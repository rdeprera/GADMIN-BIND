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
#include <string.h>
#include "widgets.h"
#include "gettext.h"
#include "commands.h"
#include "allocate.h"
#include "load_settings.h"


extern char CHROOT_PATH_BUF[1024];
//extern char SITES_DIR[1024];
//extern char NAMED_CONF_BUF[1024];
extern char SYSLOG_PATH_BUF[1024];
extern char RESOLV_PATH_BUF[1024];
//extern char RNDC_CONF_BUF[1024];



void load_settings(struct w *widgets)
{
    FILE *fp;
    long file_size;
    char *line;
    gchar *settings_file;

    /* For compat with the old gbindadmin */
    gchar *old_settings_dir, *new_settings_dir, *cmd;

    /* SETTINGS_DIR is defined at compiletime */
    settings_file = g_strdup_printf("%s/settings.conf", SETTINGS_DIR);

    /* Compat */
    old_settings_dir = g_strdup_printf("/etc/gbindadmin");
    new_settings_dir = g_strdup_printf("/etc/gadmin-bind");
    if( file_exists(old_settings_dir) && ! file_exists(new_settings_dir) )
    {
	cmd = g_strdup_printf("mv %s %s", old_settings_dir, new_settings_dir);    
	if((fp=popen(cmd, "r"))==NULL)
	{
	}
	else
          pclose(fp);

	g_free(cmd);
    }    
    g_free(old_settings_dir);
    g_free(new_settings_dir);
    


    
    /* Read the settings file */	    
    if((fp=fopen(settings_file, "r"))==NULL)
    {
        /* No need to notify the user, itll be created on save */
	g_free(settings_file);
        return;
    }
    fseek(fp, 0, SEEK_END);
    file_size = ftell(fp);
    rewind(fp);
					    
    line = allocate(file_size+1);

    if( file_size > 1 )
    while(fgets(line, file_size, fp)!=NULL)
    {
	if( strlen(line) > 1000 )
	  continue;
	
	if( strstr(line, "chroot_directory:") )
	  sscanf(line, "%*s %s", CHROOT_PATH_BUF);

	if( strstr(line, "syslog_file_path:") )
	  sscanf(line, "%*s %s", SYSLOG_PATH_BUF);

	if( strstr(line, "resolv_conf_path:") )
	  sscanf(line, "%*s %s", RESOLV_PATH_BUF);
    }
    fclose(fp);
    free(line);
    g_free(settings_file);
}
