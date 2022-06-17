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
#include "show_info.h"
#include "populate_resource_treeview.h"
#include "reread_conf.h"
#include "allocate.h"
#include "commands.h"


// Combine these 2
#include "reverse_address.h"
// When this is GPL 3
#include "ip_handling.h"


extern char CHROOT_PATH_BUF[1024];

extern char global_domain_name[1024];
extern char global_domain_ipv4_address[1024];
extern char global_domain_ipv6_address[1024];
extern char global_domain_type[1024];



int zone_data_exists(gchar *zone_file, gchar *match)
{
    FILE *fp;
    long file_size = 0;
    char *line;
    int retval = 0;
    gchar *match_line;
    
    /* Remove '\n' in the end and beginning */
    match_line = g_strdup_printf("%s", &match[1]);
    match_line[strlen(match_line)-1]='\0';

    if((fp=fopen(zone_file, "r"))==NULL)
    {
	g_free(match_line);
	return retval;
    }
    fseek(fp, 0, SEEK_END);
    file_size = ftell(fp);
    rewind(fp);
    line = allocate(file_size+1);

    if( file_size > 1 )
    while(fgets(line, file_size, fp)!=NULL)
    if( strstr(line, match_line) )
    {
	retval = 1;
    }
    fclose(fp);
    free(line);
    g_free(match_line);

    return retval;
}


void add_resource(struct w *widgets)
{
    FILE *fwd_fp, *rev_ipv4_fp=NULL, *rev_ipv6_fp=NULL;
    G_CONST_RETURN gchar *hostname, *forward_address;
    gchar *fwd_address=NULL;
    char *last_octets=NULL;
    gint active_index;
    gchar *record=NULL;
    gchar *forward_zone_file;
    gchar *reverse_ipv4_zone_file, *reverse_ipv6_zone_file;
    gchar *info;
    int i=0, added_ns_ptr = 0;
    int is_ipv4 = 0;
    int is_ipv6 = 0;

    active_index    = gtk_combo_box_get_active(GTK_COMBO_BOX(widgets->add_resource_type_combo));
    hostname        = gtk_entry_get_text(GTK_ENTRY(widgets->add_resource_hostname_entry));
    forward_address = gtk_entry_get_text(GTK_ENTRY(widgets->add_resource_address_entry));
    fwd_address = g_strdup_printf("%s", forward_address);

    if( strlen(hostname) < 3 )
    {
	info = g_strdup_printf(_("The hostname is too short, the resource was not added.\n"));
	show_info(info);
	g_free(info);
	if( fwd_address!=NULL )
	  g_free(fwd_address);
	return;
    }
    
    /* Sanitycheck MX record additions. (MX prio (address) is only 2 chars. */
    if( active_index == 1 &&
      ( strlen(fwd_address) < 2 || strlen(fwd_address) > 3 ) )
    {
	info = g_strdup_printf(_("The mailservers priority is not correct, the resource was not added.\n"));
	show_info(info);
	g_free(info);
	if( fwd_address!=NULL )
	  g_free(fwd_address);
	return;
    }
    /* If its not an MX or CNAME record... */
    if( ! active_index == 1 && ! active_index == 4 )
    {
	/* We require a valid (forward) IPv4 or IPv6 address */
	if( (! strstr(fwd_address, "." ) && ! strstr(fwd_address, ":"))
	|| strlen(fwd_address) < 3 ) /* Can be "::1" */
	{
	    // Fixme: Not translated...
	    info = g_strdup_printf(_("The IP-address is too short, the resource was not added.\n"));
	    show_info(info);
	    g_free(info);
	    if( fwd_address!=NULL )
	      g_free(fwd_address);
	    return;
	}
    }

    if( strstr(fwd_address, ":") )
      is_ipv6 = 1;
    else
      is_ipv4 = 1;

    forward_zone_file = g_strdup_printf("%s/etc/sites/%s/forward.zone", CHROOT_PATH_BUF, global_domain_name);
    reverse_ipv4_zone_file = g_strdup_printf("%s/etc/sites/%s/reverse.zone.ipv4", CHROOT_PATH_BUF, global_domain_name);
    reverse_ipv6_zone_file = g_strdup_printf("%s/etc/sites/%s/reverse.zone.ipv6", CHROOT_PATH_BUF, global_domain_name);

    /* Open the forward zone for this site */
    if((fwd_fp=fopen(forward_zone_file, "a"))==NULL)
    {
        info = g_strdup_printf(_("Resource not added. Could not find the forward zone here: %s\n"), forward_zone_file);
	show_info(info);
	g_free(info);

        g_free(forward_zone_file);
        g_free(reverse_ipv4_zone_file);
	if( fwd_address!=NULL )
	  g_free(fwd_address);
        return;
    }
    
    if( is_ipv4 )
    {
	/* Open the reverse IPv4 zone file for this domain */
	if((rev_ipv4_fp=fopen(reverse_ipv4_zone_file, "a"))==NULL)
	{
    	    info = g_strdup_printf(_("Resource not added. Could not find the reverse zone here: %s\n"), reverse_ipv4_zone_file);
	    show_info(info);
	    g_free(info);
	
    	    g_free(forward_zone_file);
	    g_free(reverse_ipv4_zone_file);
	    g_free(reverse_ipv6_zone_file);
	    fclose(fwd_fp);
	    if( fwd_address!=NULL )
	      g_free(fwd_address);
	    return;
	}
    }

    if( is_ipv6 )
    {
	/* Open the reverse IPv6 zone file for this domain */
	if((rev_ipv6_fp=fopen(reverse_ipv6_zone_file, "a"))==NULL)
	{
    	    info = g_strdup_printf(_("Resource not added. Could not create the reverse ipv6 zone here: %s\n"), reverse_ipv6_zone_file);
    	    show_info(info);
    	    g_free(info);

    	    g_free(forward_zone_file);
    	    g_free(reverse_ipv4_zone_file);
    	    g_free(reverse_ipv6_zone_file);
    	    fclose(fwd_fp);
	    if( fwd_address!=NULL )
	      g_free(fwd_address);
    	    return;
	}
    }

    /* Get the host part (last octets) for an address or NULL */
    last_octets = get_ptr_addr_from_forward_address((gchar *)fwd_address);
//    printf("add resource: last octets: %s\n", last_octets);

    /* Sanitycheck the address (Not for MX or CNAME) */
    if( ( ! active_index == 1 && ! active_index == 4 && last_octets == NULL )
    &&  ( strstr(last_octets, ".ip6.arpa") || strlen(last_octets) < 1 ) )
    {
	/* Forward address had no mask or was malformed */
        info = g_strdup_printf(_("Resource not added. The IPv6 address is not valid.\n"));
        show_info(info);
        g_free(info);

        g_free(forward_zone_file);
        g_free(reverse_ipv4_zone_file);
        g_free(reverse_ipv6_zone_file);
        fclose(fwd_fp);
        
        if( is_ipv4 )
          fclose(rev_ipv4_fp);
        if( is_ipv6 )
          fclose(rev_ipv6_fp);

	if( fwd_address!=NULL )
	  g_free(fwd_address);
	return;
    }


    /* Snip the forward address at the first '/' */
    if( fwd_address!=NULL && strstr(fwd_address, "/") )
    {
	for(i=0; fwd_address[i]!='\0'; i++)
	   if( fwd_address[i]=='/' )
	     fwd_address[i]='\0';
    }


    /* Add the zone records as specified by the record type combo box */
    /* Automatically add PTR records where possible */
    if( active_index <= 0 )
    {
	/* Add an NS record and an A or AAAA record to
	   the forward zone and a PTR record in the reverse zone */
	record = g_strdup_printf("\n\tIN\tNS\t%s.\n", hostname);
	if( ! zone_data_exists(forward_zone_file, record) )
	  fputs(record, fwd_fp);
	
	if( is_ipv4 )
	{
	    record = g_strdup_printf("\n%s.\tIN\tA\t%s\n", hostname, fwd_address);
	    if( ! zone_data_exists(forward_zone_file, record) )
	      fputs(record, fwd_fp);
	}
	else
	{
	    record = g_strdup_printf("\n%s.\tIN\tAAAA\t%s\n", hostname, fwd_address);
	    if( ! zone_data_exists(forward_zone_file, record) )
	      fputs(record, fwd_fp);
	}

	if( is_ipv4 )
	{
	    record = g_strdup_printf("\n\tIN\tNS\t%s.\n", hostname);
	    if( ! zone_data_exists(reverse_ipv4_zone_file, record) )
	      fputs(record, rev_ipv4_fp);
	}
	else
	{
	    record = g_strdup_printf("\n\tIN\tNS\t%s.\n", hostname);
	    if( ! zone_data_exists(reverse_ipv6_zone_file, record) )
	      fputs(record, rev_ipv6_fp);
	}
	
	if( is_ipv4 )
	{
	    record = g_strdup_printf("\n%s\tIN\tPTR\t%s.\n", last_octets, hostname);
	    if( ! zone_data_exists(reverse_ipv4_zone_file, record) )
	    {	        
	        fputs(record, rev_ipv4_fp);
	        added_ns_ptr = 1;
	    }
	}
	else
	{
	    record = g_strdup_printf("\n%s\tIN\tPTR\t%s.\n", last_octets, hostname);
	    if( ! zone_data_exists(reverse_ipv6_zone_file, record) )
	    {
	        fputs(record, rev_ipv6_fp);
	        added_ns_ptr = 1;
	    }
	}

	if( added_ns_ptr )
	{
	    info = g_strconcat(
	    _("An A or AAAA record was also added to the forward zone.\n"),
	    _("A PTR record for this resource was added to the reverse zone.\n"),
	    NULL);
	    show_info(info);
	    g_free(info);
	}
    }
    else
    if( active_index == 1 )
    {
	/* Add an MX record in the forward zone (No PTR) */
	record = g_strdup_printf("\n\tIN\tMX\t%s\t%s.\n", fwd_address, hostname);
	if( ! zone_data_exists(forward_zone_file, record) )
	  fputs(record, fwd_fp);

	info = g_strdup_printf(_("This MX record also requires an A or AAAA record.\n"));
	show_info(info);
	g_free(info);
    }
    else
    if( active_index == 2 )
    {
	/* Add an IPv4 A record to the forward zone and a PTR in the reverse zone */
	record = g_strdup_printf("\n%s.\tIN\tA\t%s\n", hostname, fwd_address);
	if( ! zone_data_exists(forward_zone_file, record) )
	  fputs(record, fwd_fp);

	record = g_strdup_printf("\n%s\tIN\tPTR\t%s.\n", last_octets, hostname);
	if( ! zone_data_exists(reverse_ipv4_zone_file, record) )
	  fputs(record, rev_ipv4_fp);

//	info = g_strdup_printf(_("A PTR record for this resource was also added to the reverse zone\n"));
//	show_info(info);
//	g_free(info);
    }
    else
    if( active_index == 3 )
    {
	/* Add an IPv6 AAAA record to the forward zone and a PTR in the reverse zone */
	record = g_strdup_printf("\n%s.\tIN\tAAAA\t%s\n", hostname, fwd_address);
	if( ! zone_data_exists(forward_zone_file, record) )
	  fputs(record, fwd_fp);

	record = g_strdup_printf("\n%s\tIN\tPTR\t%s.\n", last_octets, hostname);
	if( ! zone_data_exists(reverse_ipv6_zone_file, record) )
	  fputs(record, rev_ipv6_fp);

//	info = g_strdup_printf(_("A PTR record for this resource was also added to the reverse zone\n"));
//	show_info(info);
//	g_free(info);
    }
    else
    if( active_index == 4 )
    {
	/* Add a CNAME record to the forward zone */
	record = g_strdup_printf("\n%s.\tIN\tCNAME\t%s.\n", fwd_address, hostname);
	if( ! zone_data_exists(forward_zone_file, record) )
	  fputs(record, fwd_fp);
    }

    if( last_octets!=NULL )
      free(last_octets);

    if( fwd_address!=NULL )
      g_free(fwd_address);

    g_free(record);
    g_free(forward_zone_file);
    g_free(reverse_ipv4_zone_file);
    g_free(reverse_ipv6_zone_file);

    fclose(fwd_fp);

    if( is_ipv4 )
      fclose(rev_ipv4_fp);

    if( is_ipv6 )
      fclose(rev_ipv6_fp);

    populate_resource_treeview(widgets);
}
