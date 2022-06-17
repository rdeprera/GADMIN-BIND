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
#include "select_first_resource.h"
#include "reverse_address.h"

extern char CHROOT_PATH_BUF[1024];

extern int MAX_CONF_LINE;

extern char global_domain_name[1024];
extern char global_domain_ipv4_address[1024];
extern char global_domain_ipv6_address[1024];
extern char global_domain_type[1024];

char *get_forward_ip_from_ns_name(char *hostname);



char *get_forward_ip_from_ns_name(char *hostname)
{
    FILE *fp;
    long file_size = 0;
    char *forward_ip;
    char *line, *tmp1, *tmp2, *tmp3, *tmp4;
    gchar *forward_zone_file;

    
    forward_zone_file = g_strdup_printf("%s/etc/sites/%s/forward.zone", CHROOT_PATH_BUF, global_domain_name);
    if((fp=fopen(forward_zone_file, "r"))==NULL)
    {
        printf("Could not read the forward zone file: \n%s\n", forward_zone_file);
	g_free(forward_zone_file);
        return "ENOFORWARDFILE";
    }
    fseek(fp, 0, SEEK_END);
    file_size = ftell(fp);
    rewind(fp);

    line = allocate(file_size+1);
    forward_ip = allocate(MAX_CONF_LINE);
    tmp1 = allocate(MAX_CONF_LINE);
    tmp2 = allocate(MAX_CONF_LINE);
    tmp3 = allocate(MAX_CONF_LINE);
    tmp4 = allocate(MAX_CONF_LINE);

    if( file_size > 1 )
    while(fgets(line, file_size, fp)!=NULL)
    {
        if( strlen(line) > MAX_CONF_LINE-3 )
	    continue;

	tmp1[0]='\0'; tmp2[0]='\0'; tmp3[0]='\0'; tmp4[0]='\0';
	sscanf(line, "%s %s %s %s", tmp1, tmp2, tmp3, tmp4);

	/* Skip ';' comments */
	if( strstr(tmp1, ";") )
	  continue;

	/* This is the line we where looking for */
	if( !strcmp(tmp1, hostname) && (!strcmp(tmp2, "A") || !strcmp(tmp3, "A") ) )
	{
	    if( strlen(tmp4) > 6 && strlen(tmp4) < MAX_CONF_LINE-3 )
	    {
		snprintf(forward_ip, MAX_CONF_LINE-3, "%s", tmp4);	    
	    }
	    else
	    if( strlen(tmp3) > 6 && strlen(tmp3) < MAX_CONF_LINE-3 )
	    {
		snprintf(forward_ip, MAX_CONF_LINE-3, "%s", tmp3);	    
	    }
	}
    }
    free(line);
    free(tmp1); free(tmp2); free(tmp3); free(tmp4);
    fclose(fp);
    g_free(forward_zone_file);
    
    return forward_ip;
}


void populate_resource_treeview(struct w *widgets)
{
    FILE *fp;
    GtkTreeIter iter;
    gchar *utf8=NULL;
    GtkTreePath *path;
    gboolean edit=0;
    int set_cursorpath=0;
    long file_size=0;
    char *line, *forward_ip;

    gchar *text, *info;
    gchar *forward_zone_file;
    gchar *reverse_zone_file;
    int i, nameservers = 0;
    char *tmp1, *tmp2, *tmp3, *tmp4;

    
    gtk_list_store_clear(widgets->resource_store);

    if( global_domain_name == NULL )
	return;
 
    if( strlen(global_domain_name) < 3 )
	return;


    /* List all resources for the selected domain */
    forward_zone_file = g_strdup_printf("%s/etc/sites/%s/forward.zone", CHROOT_PATH_BUF, global_domain_name);
    reverse_zone_file = g_strdup_printf("%s/etc/sites/%s/reverse.zone", CHROOT_PATH_BUF, global_domain_name);
    
    if((fp=fopen(forward_zone_file, "r"))==NULL)
    {
       printf("Could not read the forward zone file: \n%s\n", forward_zone_file);
       g_free(forward_zone_file);
       g_free(reverse_zone_file);
       return;
    }
    fseek(fp, 0, SEEK_END);
    file_size = ftell(fp);
    rewind(fp);

    line = allocate(file_size+1);

    tmp1 = allocate(MAX_CONF_LINE);
    tmp2 = allocate(MAX_CONF_LINE);
    tmp3 = allocate(MAX_CONF_LINE);
    tmp4 = allocate(MAX_CONF_LINE);

    /* Read this file 4 times */
    for(i=0; i < 5; i++)
    {
	if( file_size > 1 )
	while(fgets(line, file_size, fp)!=NULL)
	{
	    if( strlen(line) > MAX_CONF_LINE-3 )
		continue;

	    tmp1[0]='\0'; tmp2[0]='\0'; tmp3[0]='\0'; tmp4[0]='\0';
	    sscanf(line, "%s %s %s %s", tmp1, tmp2, tmp3, tmp4);

	    /* Skip ';' comments */
	    if( strstr(tmp1, ";") )
	      continue;

	    if( i == 0 && !strcmp(tmp2, "NS") ) /* IN NS */
	    {
		nameservers++;

		text = g_strdup_printf("NS-Record");
		gtk_list_store_append(GTK_LIST_STORE(widgets->resource_store), &iter);
		utf8 = g_locale_to_utf8(text, strlen(text), NULL, NULL, NULL);
		gtk_list_store_set(GTK_LIST_STORE(widgets->resource_store), &iter, 0, utf8, -1);

		/* Hostname */
		utf8 = g_locale_to_utf8(tmp3, strlen(tmp3), NULL, NULL, NULL);
		gtk_list_store_set(GTK_LIST_STORE(widgets->resource_store), &iter, 1, utf8, -1);

		/* Get the IP-Address from the forward zone's A-Record for this NS */
		forward_ip = get_forward_ip_from_ns_name(tmp3);
		utf8 = g_locale_to_utf8(forward_ip, strlen(forward_ip), NULL, NULL, NULL);
		gtk_list_store_set(GTK_LIST_STORE(widgets->resource_store), &iter, 2, utf8, -1);
		free(forward_ip);

		set_cursorpath = 1;
	    }
	    else
	    if( i == 0 && !strcmp(tmp1, "NS") ) /* No(IN) NS */
	    {
		nameservers++;

		text = g_strdup_printf("NS-Record");
		gtk_list_store_append(GTK_LIST_STORE(widgets->resource_store), &iter);
		utf8 = g_locale_to_utf8(text, strlen(text), NULL, NULL, NULL);
		gtk_list_store_set(GTK_LIST_STORE(widgets->resource_store), &iter, 0, utf8, -1);

		/* Hostname */
		utf8 = g_locale_to_utf8(tmp2, strlen(tmp2), NULL, NULL, NULL);
		gtk_list_store_set(GTK_LIST_STORE(widgets->resource_store), &iter, 1, utf8, -1);

		/* Get the IP-Address from the forward zone's A-Record for this NS */
		forward_ip = get_forward_ip_from_ns_name(tmp2);
		utf8 = g_locale_to_utf8(forward_ip, strlen(forward_ip), NULL, NULL, NULL);
		gtk_list_store_set(GTK_LIST_STORE(widgets->resource_store), &iter, 2, utf8, -1);
		free(forward_ip);

		set_cursorpath = 1;
	    }
	    else
	    if( i == 1 && !strcmp(tmp3, "A") ) /* IN A */
	    {
		/* Record type */
		text = g_strdup_printf("A-Record");
		gtk_list_store_append(GTK_LIST_STORE(widgets->resource_store), &iter);
		utf8 = g_locale_to_utf8(text, strlen(text), NULL, NULL, NULL);
		gtk_list_store_set(GTK_LIST_STORE(widgets->resource_store), &iter, 0, utf8, -1);

		/* Hostname */
		utf8 = g_locale_to_utf8(tmp1, strlen(tmp1), NULL, NULL, NULL);
		gtk_list_store_set(GTK_LIST_STORE(widgets->resource_store), &iter, 1, utf8, -1);

		/* IP-Address */
		utf8 = g_locale_to_utf8(tmp4, strlen(tmp4), NULL, NULL, NULL);
		gtk_list_store_set(GTK_LIST_STORE(widgets->resource_store), &iter, 2, utf8, -1);

		set_cursorpath = 1;
	    }
	    else
	    if( i == 1 && !strcmp(tmp2, "A") ) /* No(IN) A */
	    {
		/* Record type */
		text = g_strdup_printf("A-Record");
		gtk_list_store_append(GTK_LIST_STORE(widgets->resource_store), &iter);
		utf8 = g_locale_to_utf8(text, strlen(text), NULL, NULL, NULL);
		gtk_list_store_set(GTK_LIST_STORE(widgets->resource_store), &iter, 0, utf8, -1);

		/* Hostname */
		utf8 = g_locale_to_utf8(tmp1, strlen(tmp1), NULL, NULL, NULL);
		gtk_list_store_set(GTK_LIST_STORE(widgets->resource_store), &iter, 1, utf8, -1);

		/* IP-Address */
		utf8 = g_locale_to_utf8(tmp3, strlen(tmp3), NULL, NULL, NULL);
		gtk_list_store_set(GTK_LIST_STORE(widgets->resource_store), &iter, 2, utf8, -1);

		set_cursorpath = 1;
	    }
	    else
	    if( i == 2 && !strcmp(tmp3, "AAAA") ) /* IN AAAA */
	    {
		/* Record type */
		text = g_strdup_printf("AAAA-Record");
		gtk_list_store_append(GTK_LIST_STORE(widgets->resource_store), &iter);
		utf8 = g_locale_to_utf8(text, strlen(text), NULL, NULL, NULL);
		gtk_list_store_set(GTK_LIST_STORE(widgets->resource_store), &iter, 0, utf8, -1);

		/* Hostname */
		utf8 = g_locale_to_utf8(tmp1, strlen(tmp1), NULL, NULL, NULL);
		gtk_list_store_set(GTK_LIST_STORE(widgets->resource_store), &iter, 1, utf8, -1);

		/* IP-Address */
		utf8 = g_locale_to_utf8(tmp4, strlen(tmp4), NULL, NULL, NULL);
		gtk_list_store_set(GTK_LIST_STORE(widgets->resource_store), &iter, 2, utf8, -1);

		set_cursorpath = 1;
	    }
	    else
	    if( i == 2 && !strcmp(tmp2, "AAAA") ) /* No(IN) AAAA */
	    {
		/* Record type */
		text = g_strdup_printf("AAAA-Record");
		gtk_list_store_append(GTK_LIST_STORE(widgets->resource_store), &iter);
		utf8 = g_locale_to_utf8(text, strlen(text), NULL, NULL, NULL);
		gtk_list_store_set(GTK_LIST_STORE(widgets->resource_store), &iter, 0, utf8, -1);

		/* Hostname */
		utf8 = g_locale_to_utf8(tmp1, strlen(tmp1), NULL, NULL, NULL);
		gtk_list_store_set(GTK_LIST_STORE(widgets->resource_store), &iter, 1, utf8, -1);

		/* IP-Address */
		utf8 = g_locale_to_utf8(tmp3, strlen(tmp3), NULL, NULL, NULL);
		gtk_list_store_set(GTK_LIST_STORE(widgets->resource_store), &iter, 2, utf8, -1);

		set_cursorpath = 1;
	    }
	    else
	    if( i == 3 && strstr(tmp3, "CNAME") ) /* IN CNAME */
	    {
		text = g_strdup_printf("CNAME-Record");
		gtk_list_store_append(GTK_LIST_STORE(widgets->resource_store), &iter);
		utf8 = g_locale_to_utf8(text, strlen(text), NULL, NULL, NULL);
		gtk_list_store_set(GTK_LIST_STORE(widgets->resource_store), &iter, 0, utf8, -1);

		/* Point from */
		utf8 = g_locale_to_utf8(tmp4, strlen(tmp4), NULL, NULL, NULL);
		gtk_list_store_set(GTK_LIST_STORE(widgets->resource_store), &iter, 1, utf8, -1);

		/* Point to */
		utf8 = g_locale_to_utf8(tmp1, strlen(tmp1), NULL, NULL, NULL);
		gtk_list_store_set(GTK_LIST_STORE(widgets->resource_store), &iter, 2, utf8, -1);

		set_cursorpath = 1;
	    }
	    else
	    if( i == 3 && strstr(tmp2, "CNAME") ) /* No(IN) CNAME */
	    {
		text = g_strdup_printf("CNAME-Record");
		gtk_list_store_append(GTK_LIST_STORE(widgets->resource_store), &iter);
		utf8 = g_locale_to_utf8(text, strlen(text), NULL, NULL, NULL);
		gtk_list_store_set(GTK_LIST_STORE(widgets->resource_store), &iter, 0, utf8, -1);

		/* Point from */
		utf8 = g_locale_to_utf8(tmp3, strlen(tmp3), NULL, NULL, NULL);
		gtk_list_store_set(GTK_LIST_STORE(widgets->resource_store), &iter, 1, utf8, -1);

		/* Point to */
		utf8 = g_locale_to_utf8(tmp1, strlen(tmp1), NULL, NULL, NULL);
		gtk_list_store_set(GTK_LIST_STORE(widgets->resource_store), &iter, 2, utf8, -1);

		set_cursorpath = 1;
	    }
	    else
	    if( i == 4 && strstr(tmp2, "MX") ) /* IN MX */
	    {
		text = g_strdup_printf("MX-Record");
		gtk_list_store_append(GTK_LIST_STORE(widgets->resource_store), &iter);
		utf8 = g_locale_to_utf8(text, strlen(text), NULL, NULL, NULL);
		gtk_list_store_set(GTK_LIST_STORE(widgets->resource_store), &iter, 0, utf8, -1);

		utf8 = g_locale_to_utf8(tmp4, strlen(tmp4), NULL, NULL, NULL);
		gtk_list_store_set(GTK_LIST_STORE(widgets->resource_store), &iter, 1, utf8, -1);

		utf8 = g_locale_to_utf8(tmp3, strlen(tmp3), NULL, NULL, NULL);
		gtk_list_store_set(GTK_LIST_STORE(widgets->resource_store), &iter, 2, utf8, -1);

		set_cursorpath = 1;
	    }
	    else
	    if( i == 4 && strstr(tmp1, "MX") ) /* No(IN) MX */
	    {
		text = g_strdup_printf("MX-Record");
		gtk_list_store_append(GTK_LIST_STORE(widgets->resource_store), &iter);
		utf8 = g_locale_to_utf8(text, strlen(text), NULL, NULL, NULL);
		gtk_list_store_set(GTK_LIST_STORE(widgets->resource_store), &iter, 0, utf8, -1);

		utf8 = g_locale_to_utf8(tmp3, strlen(tmp3), NULL, NULL, NULL);
		gtk_list_store_set(GTK_LIST_STORE(widgets->resource_store), &iter, 1, utf8, -1);

		utf8 = g_locale_to_utf8(tmp2, strlen(tmp2), NULL, NULL, NULL);
		gtk_list_store_set(GTK_LIST_STORE(widgets->resource_store), &iter, 2, utf8, -1);

		set_cursorpath = 1;
	    }
	}
	rewind(fp);
    }
    free(tmp1);
    free(tmp2);
    free(tmp3);
    free(line);
    g_free(forward_zone_file);
    g_free(reverse_zone_file);


    if( nameservers < 1 )
    {
	info = g_strdup_printf(_("You must add a nameserver (NS-Record) for this domain now.\n"));
	show_info(info);
	g_free(info);
    }


    if( utf8!=NULL )
      g_free(utf8);

    if( ! set_cursorpath )
      return;
    
    /* FIXME.. Scroll to the correct global nic subnet and netmask (and if its set) set as curpath */
    path = gtk_tree_path_new_first();
    gtk_tree_view_set_cursor(GTK_TREE_VIEW(widgets->resource_treeview), path, NULL, edit);
    gtk_tree_path_free(path);

    select_first_resource(widgets);
}
