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
#include "gettext.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include "allocate.h"
#include "widgets.h"
#include "chars_are_digits.h"
#include "commands.h"
#include "status_update.h"
#include "show_info.h"

extern int activated;
extern int MAX_READ_POPEN;


char * get_process_pid(char process[1024])
{
    FILE *fp;
    long num, file_size = 4000;
    char *line, *sub_proc_path, *pid;
    int x=0;
    struct dirent **namelist;

    pid = allocate(1024);
    strcpy(pid, "NONE");
    
    sub_proc_path = allocate(1024);

    num = scandir(PROC_PATH, &namelist, 0, alphasort);
    if( num < 0 )
    {
        perror("scandir");
        return pid;
    }
    else
    {
        /* List all directories in PROC_PATH */
        for(x=0; x<num; x++)
        {
	    /* Now list PROC_PATH/24207/cmdline */
	    snprintf(sub_proc_path, 1000, "%s/%s/cmdline", PROC_PATH, namelist[x]->d_name);

	    if((fp=fopen(sub_proc_path, "r"))==NULL)
	    {
        	free(namelist[x]);
	        continue;
	    }
	    line = allocate(file_size+1);
	    
	    if( file_size > 1 )
	    while(fgets(line, file_size, fp)!=NULL)
	    {
		/* If the following string is in this file its running */
		if( strstr(line, process) )
		{
		    snprintf(pid, 1000, "%s", namelist[x]->d_name);
		    break;
		}
	    }
	    fclose(fp);
	    free(line);
            free(namelist[x]);
	}
    }
    free(namelist);
    free(sub_proc_path);

    return pid;
}


int status_update(struct w *widgets)
{
    /* Activation status for named. */
    gchar *info, *utf8=NULL;
    char *pid;
    GdkColor color;

    pid = get_process_pid("named");

    /* First set activation status globally */
    if( strstr(pid, "NONE") )
      activated = 0;
    else
      activated = 1;

    /* If the daemon isnt running its deactivated */
    if( ! activated )
    {
	activated = 0;
	info = g_strdup_printf(_("Status: Deactivated"));
	utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);
	gtk_label_set_text(GTK_LABEL(widgets->status_label), utf8);
	g_free(info);
	free(pid);
	/* Set status color */
	gdk_color_parse("red", &color);
	gtk_widget_modify_fg(widgets->status_label, GTK_STATE_NORMAL, &color);

	return(TRUE);
    }
    else
      {
	  /* The daemon is running */
	  activated = 1;

	  info = g_strdup_printf(_("Status: Activated"));
    	  utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);
    	  gtk_label_set_text(GTK_LABEL(widgets->status_label), utf8);
	  g_free(info);

	  /* Set status color */
	  gdk_color_parse("dark green", &color);
	  gtk_widget_modify_fg(widgets->status_label, GTK_STATE_NORMAL, &color);    
      }

    free(pid);
    
    if( utf8!=NULL )
      g_free(utf8);

    return(TRUE);
}
