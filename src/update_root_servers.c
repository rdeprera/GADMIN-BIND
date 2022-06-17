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
#include "commands.h"
#include "populate_root_servers_tab.h"
#include "allocate.h"

extern char CHROOT_PATH_BUF[1024];
extern int activated;


void update_root_servers(struct w *widgets)
{
    FILE *fp;
    char *buffy, *conf;
    long file_size, num_lines = 0;
    gchar *info, *command, *root_hints;

    char tmpfile[]="/tmp/gadmin-bind-XXXXXX";
    int tmpfd = -1;

    tmpfd = mkstemp(tmpfile);
    if( tmpfd < 0 )
    {
	printf("Error: Could not create temporary dig file: %s\n", tmpfile);
	return;
    }
    close(tmpfd);

    command = g_strdup_printf("%s > %s", DIG_BINARY, tmpfile);
    if( ! run_command(command) )
      printf("Dig failed: could not get root server listing.\n");

    g_free(command);

    if((fp=fopen(tmpfile, "r"))==NULL)
    {
	printf("Cannot open temporary dig file: %s", tmpfile);
	return;
    }

    fseek(fp, 0, SEEK_END);
    file_size = ftell(fp);
    rewind(fp);

    buffy = allocate(file_size+1);
    conf = allocate(file_size+1);
    
    while(fgets(buffy, file_size, fp)!=NULL)
    {
	strcat(conf, buffy);
	num_lines++;
    }
    fclose(fp);
    free(buffy);

    /* Remove the temp file if it exists */
    if( file_exists(tmpfile) )
	unlink(tmpfile);
    
    if( num_lines < 20 )
    {
	info = g_strdup_printf(_("Received less then 20 lines of root server information.\nMake sure that the resolvers tab has working nameservers listed.\n"));
	show_info(info);
	g_free(info);

	free(conf);
	return;
    }

    root_hints = g_strdup_printf("%s/etc/root.hints", CHROOT_PATH_BUF);        

    if((fp=fopen(root_hints, "w+"))==NULL)
    {
	info = g_strdup_printf(_("Can not write root.hints here:\n%s\n"), root_hints);
	show_info(info);
	g_free(info);

	free(conf);
	g_free(root_hints);
	return;
    }
    fputs(conf, fp);
    fclose(fp);

    free(conf);
    g_free(root_hints);

    populate_root_servers_tab(widgets);
}
