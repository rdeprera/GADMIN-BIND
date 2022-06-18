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
#include "reread_conf.h"
#include "commands.h"
#include "reverse_address.h"

extern int MAX_CONF_LINE;
extern char CHROOT_PATH_BUF[1024];

extern char global_domain_name[1024];
extern char global_domain_ipv4_address[1024];
extern char global_domain_ipv6_address[1024];



void delete_domain(struct w *widgets)
{
    FILE *fp;
    long file_size=0;
    char *new_conf;
    char *line;
    gchar *info, *path;
    gchar *rm_zone_dir_cmd;
    char *reversed_address;
    gchar *forward_zone_line, *reverse_zone_line;

    /* Delete forward and reverse zones in named.conf
     * Delete the site directory: rm -rf /var/named/etc/sites/_site_ */
    if( strlen(global_domain_name)==0 )
    {
       info = g_strdup_printf(_("Missing domain name, domain not deleted\n"));
       show_info(info);
       g_free(info);
       return;
    }
    if( strlen(global_domain_ipv4_address)==0
    &&  strlen(global_domain_ipv6_address)==0 )
    {
       info = g_strdup_printf(_("Missing network address, domain not deleted\n"));
       show_info(info);
       g_free(info);
       return;
    }

    rm_zone_dir_cmd = g_strdup_printf("rm -rf %s/etc/sites/%s/", CHROOT_PATH_BUF, global_domain_name);
    if( ! run_command(rm_zone_dir_cmd) )
    {
	info = g_strdup_printf(_("Error: could not run remove zone directory command:\n%s\n"), rm_zone_dir_cmd);
        show_info(info);
	g_free(info);
	g_free(rm_zone_dir_cmd);
	return;
    }

    g_free(rm_zone_dir_cmd);


    /* Collect everything in named.conf but this sites 
     * forward and reverse zones */
    path = g_strdup_printf("%s%s", CHROOT_PATH_BUF, BIND_CONF);

    if((fp=fopen(path, "r"))==NULL)
    {
        info = g_strdup_printf(_("Could not open named.conf here: %s\n"), path);
        show_info(info);
	g_free(info);
	g_free(path);
        return;
    }
    fseek(fp, 0, SEEK_END);
    file_size = ftell(fp);
    rewind(fp);

    new_conf = allocate(file_size+1);
    line = allocate(file_size+1);

    forward_zone_line = g_strdup_printf("\"%s\"", global_domain_name);

    reversed_address = reverse_address(global_domain_ipv4_address);
    reverse_zone_line = g_strdup_printf("\"%s.in-addr.arpa\"", reversed_address);
    free(reversed_address);

    if( file_size > 1 )
    while(fgets(line, file_size, fp)!=NULL)
    {
	/* Skip the forward zone and its options */
	if( strstr(line, "zone ") && strstr(line, forward_zone_line) )
	{
	    while(fgets(line, file_size, fp)!=NULL)
	    {
		/* This is the end of the zone */
		if( strstr(line, "};") && strlen(line) < 4 )
		  break;
	    }
	}
	else
	/* Skip the reverse zone and its options */
	if( strstr(line, "zone ") && strstr(line, reverse_zone_line) )
	{
	    while(fgets(line, file_size, fp)!=NULL)
	    {
		/* This is the end of the zone */
		if( strstr(line, "};") && strlen(line) < 4 )
		  break;
	    }
	}
	else
	  strcat(new_conf, line);
    }
    g_free(forward_zone_line); 
    g_free(reverse_zone_line);
    fclose(fp);
    free(line);
    
    /* Write the new conf */    
    if((fp=fopen(path, "w+"))==NULL)
    {
        info = g_strdup_printf(_("Could not write named.conf here: %s\n"), path);
        show_info(info);
	g_free(info);
	g_free(path);
        return;
    }
    fputs(new_conf, fp);
    fclose(fp);

    free(new_conf);
    g_free(path);    
}
