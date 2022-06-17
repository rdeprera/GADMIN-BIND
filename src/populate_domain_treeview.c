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
#include "reverse_address.h"
#include "select_first_domain.h"

/* Compat for the old gbindadmin */
#include "commands.h"

extern char CHROOT_PATH_BUF[1024];
extern int MAX_CONF_LINE;

char * get_reverse_ip_for_zone(char *domain_name, char addr_type[128]);



/* Insert all forward zones and their network adresses */
void populate_zones(struct w *widgets)
{
    FILE *fp;
    long file_size = 0;
    GtkTreeIter iter;
    gchar *named_conf, *utf8=NULL;
    GtkTreePath *path;
    gboolean edit=0;
    int set_cursorpath=0;
    char *line, *domain_name, *fwd_addr = NULL;
    char *rev_addr, *rev_file, *type, *mask, *tmp, *tmp_addr;

    named_conf = g_strdup_printf("%s/etc/named.conf", CHROOT_PATH_BUF);
    if((fp=fopen(named_conf, "r"))==NULL)
    {
        printf("Couldnt find: %s\n", named_conf);
	g_free(named_conf);
        return;
    }
    fseek(fp, 0, SEEK_END);
    file_size = ftell(fp);
    rewind(fp);

    line = allocate(file_size+1);
    domain_name = allocate(MAX_CONF_LINE);
    rev_file = allocate(MAX_CONF_LINE);
    type = allocate(MAX_CONF_LINE);
    tmp = allocate(MAX_CONF_LINE);
    tmp_addr = allocate(MAX_CONF_LINE);
    mask = allocate(MAX_CONF_LINE);


    if( file_size > 1 )
    while(fgets(line, file_size, fp)!=NULL)
    {
	if( strlen(line) > MAX_CONF_LINE-3 )
	  continue;

	/* Skip comments */
	if( strstr(line, "/*") )
	{
	    while(fgets(line, file_size, fp)!=NULL)
	    {
		if( strstr(line, "*/") )
		  break;
	    }	
	}
	/* Skip comment */
	if( strstr(line, "//") )
	  continue;

	/* Ignore any options */
	if( ! strstr(line, "zone") && strstr(line, "{") && ! strstr(line, "}") )
	  while(fgets(line, file_size, fp)!=NULL)
	    if( strstr(line, "}") )
	      break;

	
	/* Forward zone begin */
	if(  strstr(line, "zone ") && strstr(line, "{")  // && strstr(line, "\"")
//	&& ! strstr(line, "forward.zone") 
	&& ! strstr(line, "localhost") 
	&& ! strstr(line, ".in-addr.arpa") 
	&& ! strstr(line, ".ip6.arpa")
	&& ! strstr(line, ".ip6.int")
	&& ! strstr(line, "hint") )
	{
	    sscanf(line, "%*s %s", domain_name);
	    snprintf(tmp, 8000, "%s", &domain_name[1]);
	    snprintf(domain_name, strlen(tmp), "%s", tmp);

	    /* Dont append root.hints or a malformed local zone */
	    if( ! strcmp(domain_name, ".") || ! strcmp(domain_name, "127.0.0") )
	      continue;

	    gtk_list_store_append(GTK_LIST_STORE(widgets->domain_store), &iter);
	    utf8 = g_locale_to_utf8(domain_name, strlen(domain_name), NULL, NULL, NULL);
	    gtk_list_store_set(GTK_LIST_STORE(widgets->domain_store), &iter, 0, utf8, -1);

	    /* Insert IPv4 forward network address */
	    rev_addr = get_reverse_ip_for_zone(domain_name, "ipv4");
	    if( rev_addr != NULL )
	    {
		// printf("Reverse IPv4 network address: [%s]\n", rev_addr);
		fwd_addr = reverse_address(rev_addr);

		utf8 = g_locale_to_utf8(fwd_addr, strlen(fwd_addr), NULL, NULL, NULL);
		gtk_list_store_set(GTK_LIST_STORE(widgets->domain_store), &iter, 1, utf8, -1);

		free(fwd_addr);
	    }
	    free(rev_addr);

	    /* Insert IPv6 forward network address */
	    rev_addr = get_reverse_ip_for_zone(domain_name, "ipv6");
	    if( rev_addr != NULL )
	    {
		//printf("Reverse IPv6 network address: [%s]\n", rev_addr);

		/* Add A host part to the network address */
    		snprintf(tmp, 1024, "9.7.b.9.a.e.e.f.f.f.f.8.3.1.2.0.%s.ip6.arpa", rev_addr);
    		snprintf(rev_addr, 1024, "%s", tmp);
	        // Some host part + named.conf zone network address part.

		// Special thing, there should be a comment with the mask
		// like this: zone "x.x.x.x.x.x.x" { #mask 64
		// Removed.. Howto get mask from ipv6...
		sscanf(line, "%*s %*s %*s %*s %*s %s", mask);

		fwd_addr = reverse_address(rev_addr);
		
//		printf("REV: %s\n", rev_addr);
//		printf("FWD: %s\n", fwd_addr);

		/* Dont alter the fwd_addr pointer */
		snprintf(tmp_addr, 1024, "%s", fwd_addr);
		
		/* Add the network mask */
		if( strlen(mask) > 0 )
		{
		    snprintf(tmp, 1024, "%s/%s", tmp_addr, mask);
		    snprintf(tmp_addr, 1024, "%s", tmp);
		}

		utf8 = g_locale_to_utf8(tmp_addr, strlen(tmp_addr), NULL, NULL, NULL);
		gtk_list_store_set(GTK_LIST_STORE(widgets->domain_store), &iter, 2, utf8, -1);

		free(fwd_addr);
	    }
	    free(rev_addr);


	    set_cursorpath = 1;

	    /* DNS Type */
	    while(fgets(line, file_size, fp)!=NULL)
	    if( strstr(line, "type") && strlen(domain_name) > 3 )
	    {
		sscanf(line, "%*s %s", type);
		snprintf(tmp, 8000, "%s", type);
		snprintf(type, strlen(tmp), "%s", tmp);
		utf8 = g_locale_to_utf8(type, strlen(type), NULL, NULL, NULL);
		gtk_list_store_set(GTK_LIST_STORE(widgets->domain_store), &iter, 3, utf8, -1);
		break;	    
	    }	    
	}
    }
    fclose(fp);
    g_free(named_conf);
    
    free(line);
    free(domain_name);
    free(rev_file);
    free(type);
    free(tmp);
    free(tmp_addr);
    free(mask);

    /* Forward zones populated, Select the first domain */
    path = gtk_tree_path_new_first();
    gtk_tree_view_set_cursor(GTK_TREE_VIEW(widgets->domain_treeview), path, NULL, edit);
    gtk_tree_path_free(path);
}


/* Returns the specified zones reverse network address or NULL if not found */
/* Valid addr_type's are "ipv4" or "ipv6" */
char * get_reverse_ip_for_zone(char *domain_name, char addr_type[128])
{
    FILE *fp;
    long file_size = 0;
    int i = 0, rev_zone_found = 0, fwd_zone_found = 0;
    char *line, *mask, *tmp;
    gchar *named_conf, *rev_file, *match_domain_name;
    char *rev_ip = NULL;

    /* For compat with gbindadmin */
    gchar *old_zone, *new_zone;
    FILE *move_fp;
    gchar *cmd;


    named_conf = g_strdup_printf("%s/etc/named.conf", CHROOT_PATH_BUF);
    if((fp=fopen(named_conf, "r"))==NULL)
    {
        printf("Couldnt find: %s\n", named_conf);
	g_free(named_conf);
        return rev_ip;
    }
    fseek(fp, 0, SEEK_END);
    file_size = ftell(fp);
    rewind(fp);

    line = allocate(file_size+1);
    rev_ip = allocate(MAX_CONF_LINE+1);

    if( strstr(addr_type, "ipv4") )
    {
        rev_file = g_strdup_printf("\"sites/%s/reverse.zone.ipv4\"", domain_name);

        /* For compability with gbindadmin we need to
           rename the old reverse.zone to reverse.zone.ipv4 */
        old_zone = g_strdup_printf("%s/etc/sites/%s/reverse.zone", CHROOT_PATH_BUF, domain_name);
        new_zone = g_strdup_printf("%s/etc/sites/%s/reverse.zone.ipv4", CHROOT_PATH_BUF, domain_name);
        if( file_exists(old_zone) && ! file_exists(new_zone) )
        {
    	    cmd = g_strdup_printf("mv %s %s", old_zone, new_zone);
	    if((move_fp=popen(cmd, "r"))==NULL)
	    {
    	    }
    	    else
    	      pclose(move_fp);
    	    
    	    g_free(cmd);
	}
	g_free(old_zone); g_free(new_zone);
	/* Compat end */

    }
    else
      rev_file = g_strdup_printf("\"sites/%s/reverse.zone.ipv6\"", domain_name);

    tmp = allocate(MAX_CONF_LINE);
    mask = allocate(MAX_CONF_LINE);

    match_domain_name = g_strdup_printf("\"%s\"", domain_name);
    
    
    if( file_size > 1 )
    while(fgets(line, file_size, fp)!=NULL)
    {
	if( strlen(line) > MAX_CONF_LINE-3 )
	  continue;

	/* Skip comments */
	if( strstr(line, "/*") && ! strstr(line, "*/") )
	{
	    while(fgets(line, file_size, fp)!=NULL)
	    {
		if( strstr(line, "*/") )
		  break;
	    }	
	}
	/* Skip comment */
	if( strstr(line, "//") || (strstr(line, "/*") && strstr(line, "*/")) )
	  continue;

	/* Ignore any options */
	if( ! strstr(line, "zone") && strstr(line, "{") && ! strstr(line, "}") )
	  while(fgets(line, file_size, fp)!=NULL)
	    if( strstr(line, "}") )
	      break;

	/* Scroll to the matching forward zone */
	if( ! strstr(line, "zone")
	&&  ! strstr(line, match_domain_name)
	&&  ! strstr(line, "{") )
	  continue;
	
	fwd_zone_found = 1;
	break;
    }


    if( ! fwd_zone_found )
    {
	printf("Forward zone not found: [%s]\n", domain_name);
	return rev_ip;
    }


    if( file_size > 1 )
    while(fgets(line, file_size, fp)!=NULL)
    {
	if( strlen(line) > MAX_CONF_LINE-3 )
	  continue;

	/* Skip comments */
	if( strstr(line, "/*") && ! strstr(line, "*/") )
	{
	    while(fgets(line, file_size, fp)!=NULL)
	    {
		if( strstr(line, "*/") )
		  break;
	    }	
	}
	/* Skip comment */
	if( strstr(line, "//") || (strstr(line, "/*") && strstr(line, "*/")) )
	  continue;

	/* Ignore any options */
	if( ! strstr(line, "zone") && strstr(line, "{") && ! strstr(line, "}") )
	  while(fgets(line, file_size, fp)!=NULL)
	    if( strstr(line, "}") )
	      break;


	/* Reverse zone begin */
	if( strstr(line, "zone ") && strstr(line, "{")
	&& ! strstr(line, "forward.zone") 
	&& ! strstr(line, "localhost") 
	&& ! strstr(line, "127.in-addr.arpa") 
	&& ! strstr(line, "hint")
	&& ( strstr(line, ".in-addr.arpa") || strstr(line, ".ip6.arpa") ) )
	{
	    /* Looking for ipv4 but got ipv6 */
	    if( strstr(addr_type, "ipv4") && strstr(line, ".ip6.arpa") )
	      continue;
	    /* Looking for ipv6 but got ipv4 */
	    if( strstr(addr_type, "ipv6") && strstr(line, ".in-addr.arpa") )
	      continue;

	    /* Get the reverse zones network address */
	    for(i=0; line[i]!='\0'; i++)
	      if( line[i]=='"' )
	        break;

	    snprintf(tmp, MAX_CONF_LINE-1, "%s", &line[i]);

	    /* IPv4 */
	    if( strstr(line, ".in-addr.arpa") )
	    {
	        snprintf(rev_ip, strlen(tmp)-17, "%s", &tmp[1]);
	    }
	    else /* IPv6 */
	    {
	        snprintf(rev_ip, strlen(tmp)-13, "%s", &tmp[1]);
	    }
	    
	    /* look for the file "...." line and see if its the correct domain name */
	    while(fgets(line, file_size, fp)!=NULL)
	    {	
		if( strlen(line) > MAX_CONF_LINE-3 )
		  continue;
		
		/* Skip options */
		if( strstr(line, "{") && strstr(line, "}") )
		  continue;

		/* Break at the next zone */		  
		if( strstr(line, "zone") && strstr(line, "{") )
		  break;
	    
		/* The reverse zones file name matched the zone name */
		if( strstr(line, rev_file) )
		{
		    rev_zone_found = 1;
		    break;
		}
	    }
	}

	if( rev_zone_found )
	  break;
    }
    fclose(fp);
    g_free(named_conf);    
    g_free(rev_file);    
    g_free(match_domain_name);    
    free(tmp);    
    free(mask);    

    if( rev_zone_found )
      return rev_ip;
    else
      return NULL;
}


void populate_domain_treeview(struct w *widgets)
{
    gtk_list_store_clear(widgets->domain_store);

    populate_zones(widgets);

    select_first_domain(widgets);
}
