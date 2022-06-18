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

#define _XOPEN_SOURCE /* glibc2 needs this */
#include <time.h>

#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "widgets.h"
#include "gettext.h"
#include "show_info.h"
#include "populate_domain_treeview.h"
#include "populate_resource_treeview.h"
#include "reread_conf.h"
#include "allocate.h"
#include "reverse_address.h"
#include "commands.h"

extern char CHROOT_PATH_BUF[1024];



int zone_exists(gchar *zone)
{
    FILE *fp;
    long file_size;
    int ret = 0;
    char *line;
    gchar *named_conf, *info;
    gchar *match_zone = g_strdup_printf("\"%s\"", zone);

    /* Open named.conf in this chroot and check if the domain exists */
    if(strlen(CHROOT_PATH_BUF) > 0){
        named_conf = g_strdup_printf("%s%s", CHROOT_PATH_BUF, BIND_CONF);
    }else{
        named_conf = g_strdup_printf("%s", BIND_CONF);
    }
    if((fp=fopen(named_conf, "r"))==NULL)
    {
        info = g_strdup_printf(_("Domain not added. Could not find named.conf here:\n%s\n"), named_conf);
	show_info(info);
	g_free(info);
	g_free(named_conf);
        return 1;
    }
    fseek(fp, 0, SEEK_END);
    file_size = ftell(fp);
    rewind(fp);
    line = allocate(file_size+1);

    if( file_size > 1 )
    while(fgets(line, file_size, fp)!=NULL)
    {
	if( strstr(line, "zone")
	&&  strstr(line, "{")
	&&  strstr(line, match_zone) )
	{
	    ret = 1;
	    break;
	}

    }    
    fclose(fp);
    free(line);
    g_free(named_conf);
    g_free(match_zone);

    return ret;
}


void add_domain(struct w *widgets)
{
    FILE *fp, *date_fp, *new_zone_fp;
    int have_ipv6 = 0;
    int have_ipv4 = 0;
    G_CONST_RETURN gchar *domain_name, *ipv4_address, *ipv6_address;
    gchar *command, *named_conf, *zone_file;
    char *rev_ipv4_address=NULL, *rev_ipv6_address=NULL;

    gchar *zone_data;
    char *date_string;
    gint active_index;
    gchar *mask=NULL;
    int i = 0;
    gchar *info;


    domain_name = gtk_entry_get_text(GTK_ENTRY(widgets->add_domain_name_entry));
    ipv4_address = gtk_entry_get_text(GTK_ENTRY(widgets->add_domain_ipv4_address_entry));
    ipv6_address = gtk_entry_get_text(GTK_ENTRY(widgets->add_domain_ipv6_address_entry));

    /* Domain type combo */
    active_index = gtk_combo_box_get_active(GTK_COMBO_BOX(widgets->add_domain_type_combo));

    if( strlen(domain_name) < 3 )
    {
	info = g_strdup_printf(_("The domain name is too short, the domain was not added.\n"));
	show_info(info);
	g_free(info);
	return;
    }

    if(  strlen(ipv4_address) > 4    /* Shortest ipv4 network address: "1.0.0" */
    &&   strstr(ipv4_address, ".") 
    && ! strstr(ipv4_address, ":") )
    {
	have_ipv4 = 1;
    }
    if(  strlen(ipv6_address) > 6   /* Shortest network address: "::1/128" */
    &&   strstr(ipv6_address, ":")
    && ! strstr(ipv6_address, ".") )
    {
	have_ipv6 = 1;
    }
    
    if( ! have_ipv4 && ! have_ipv6 )
    {
	info = g_strdup_printf(_("Missing valid network address, the domain was not added.\n"));
	show_info(info);
	g_free(info);
	return;
    }
    
    /* IPv6 network address, get the mask if any */
    if( have_ipv6 )
    {
        for(i=0; ipv6_address[i]!='\0'; i++)
        if( ipv6_address[i]=='/' )
	{
    	    mask = g_strdup_printf("%s", &ipv6_address[i+1]);
//	    printf("Mask: %s\n", mask);
    	    break;
        }
    }

    /* Reverse the network address(es) from the gui and sanitycheck */
    if( have_ipv4 )
    {
        rev_ipv4_address = reverse_address((char *)ipv4_address);

	// FIXME, sanity check.
    }

    if( have_ipv6 )
    {
        rev_ipv6_address = reverse_address((char *)ipv6_address);
//	printf("Add_Domain: Rev IPv6: %s\n", rev_ipv6_address);
	if( strlen(rev_ipv6_address) < 10 )
	{
	    info = g_strdup_printf(_("The IPv6 network address is not valid, the domain was not added.\n"));
	    show_info(info);
	    g_free(info);
	
	    if( mask!=NULL )
	      g_free(mask);
	    if( rev_ipv4_address!=NULL )
	      free(rev_ipv4_address);
	    if( rev_ipv6_address!=NULL )
	      free(rev_ipv6_address);
	    return;	
	}
    }
    

    /* Write the forward and reverse zones to named.conf */
    named_conf = g_strdup_printf("%s%s", CHROOT_PATH_BUF, BIND_CONF);
    if((fp=fopen(named_conf, "a"))==NULL)
    {
        info = g_strdup_printf(_("Domain not added. Could not find named.conf here:\n%s\n"), named_conf);
	show_info(info);
	g_free(info);

	if( mask!=NULL )
	  g_free(mask);
	if( rev_ipv4_address!=NULL )
	  free(rev_ipv4_address);
	if( rev_ipv6_address!=NULL )
	  free(rev_ipv6_address);
        return;
    }

    rev_ipv4_address = reverse_address((char *)ipv4_address);

    /* Create the directory CHROOT/etc/sites/DomainName */
    command = g_strdup_printf("mkdir -p %s/etc/sites/%s", CHROOT_PATH_BUF, domain_name);
    if( ! run_command(command) )
    {
        info = g_strdup_printf(_("Error running command:\n%s\n"), command);
        show_info(info);
        g_free(info);
    }
    g_free(command);


    /* Get todays date or use an old one */
    date_string = allocate(20);
    if((date_fp=popen("date +%Y%m%d", "r"))==NULL)
      strcpy(date_string, "20100319");
    else
    {
	fgets(date_string, 9, date_fp);
	pclose(date_fp);
    }
    /* Add the incremental serial */
    strcat(date_string, "01");


    /* Add the forward zone */
    if( ! zone_exists((gchar *)domain_name) )
    {
	/* Add the named.conf zones */
	if( active_index <= 0 )
	{
	    zone_data = g_strconcat(
	    "zone \"", domain_name, "\" IN {\n",
	    "\ttype master;\n",
	    "\tfile \"sites/", domain_name, "/forward.zone\";\n",
	    "\tallow-transfer { 127.0.0.1; };\n",
	    "\tallow-update { none; };\n",
	    "\tallow-query { any; };\n",
	    "\tzone-statistics yes;\n",
	    "\tnotify no;\n",
	    "\talso-notify { };\n",
	    "};\n",
	    NULL);
	    fputs(zone_data, fp);
	    g_free(zone_data);
	}
	if( active_index == 1 )
	{
	    zone_data = g_strconcat(
	    "zone \"", domain_name, "\" IN {\n",
	    "\ttype slave;\n",
	    "\tfile \"sites/", domain_name, "/forward.zone\";\n",
	    "\tallow-transfer { 127.0.0.1; };\n",
	    "\tallow-query { any; };\n",
	    "\tmasters { 127.0.0.1; };\n",
	    "\tzone-statistics yes;\n",
	    "\tnotify no;\n",
	    "\talso-notify { };\n",
	    "\tallow-notify { };\n",
	    "};\n",
	    NULL);
	    fputs(zone_data, fp);
	    g_free(zone_data);
	}

	/* Add the forward zone file */
	zone_file = g_strdup_printf("%s/etc/sites/%s/forward.zone", CHROOT_PATH_BUF, domain_name);
	if((new_zone_fp=fopen(zone_file, "w+"))==NULL)
	{
    	    info = g_strdup_printf(_("Couldnt write the forward zone file here:\n%s\n"), zone_file);
	    show_info(info);
	    g_free(info);
	    g_free(zone_file);
	    free(date_string);

	    if( mask!=NULL )
	      g_free(mask);
	    if( rev_ipv4_address!=NULL )
	      free(rev_ipv4_address);
	    if( rev_ipv6_address!=NULL )
	      free(rev_ipv6_address);
    	    return;
	}
	zone_data = g_strconcat(
	"$TTL 3D; Forward zone DNS->IP\n",
	"@\tIN\tSOA\t", domain_name, ".\thostmaster.", domain_name, ". (\n"
	"\t\t\t", date_string, "\t;\n",
	"\t\t\t8H\t\t;\n",
	"\t\t\t2H\t\t;\n",
	"\t\t\t1W\t\t;\n",
	"\t\t\t1D )\t\t;\n\n",
	NULL);
	fputs(zone_data, new_zone_fp);
	g_free(zone_data);

	fclose(new_zone_fp);
	g_free(zone_file);
    }


    /* Add the reverse IPv4 zone */
    if( have_ipv4 && ! zone_exists(rev_ipv4_address) )
    {
	if( active_index <= 0 )
	{
	    // dumped mask.
	    zone_data = g_strconcat(
	    "\nzone \"", rev_ipv4_address, ".in-addr.arpa\" {\n",
	    "\ttype master;\n"
	    "\tfile \"sites/", domain_name, "/reverse.zone.ipv4\";\n",
	    "\tallow-transfer { 127.0.0.1; };\n",
	    "\tallow-update { none; };\n",
	    "\tallow-query { any; };\n",
	    "\tzone-statistics yes;\n",
	    "\tnotify no;\n",
	    "\talso-notify { };\n",
	    "};\n",
	    NULL);
	    fputs(zone_data, fp);
	    g_free(zone_data);
	}
	if( active_index == 1 )
	{
	    // dumped mask
	    zone_data = g_strconcat(
	    "\nzone \"", rev_ipv4_address, ".in-addr.arpa\" {\n",
	    "\ttype slave;\n"
	    "\tfile \"sites/", domain_name, "/reverse.zone.ipv4\";\n",
	    "\tallow-transfer { 127.0.0.1; };\n",
	    "\tallow-query { any; };\n",
	    "\tmasters { 127.0.0.1; };\n",
	    "\tzone-statistics yes;\n",
	    "\tnotify no;\n",
	    "\talso-notify { };\n",
	    "};\n",
	    NULL);
	    fputs(zone_data, fp);
	    g_free(zone_data);
	}	

	/* Add the reverse IPv4 zone file */
	zone_file = g_strdup_printf("%s/etc/sites/%s/reverse.zone.ipv4", CHROOT_PATH_BUF, domain_name);
	if((new_zone_fp=fopen(zone_file, "w+"))==NULL)
	{
    	    info = g_strdup_printf(_("Couldnt write the forward zone file here:\n%s\n"), zone_file);
	    show_info(info);
	    g_free(info);
	    g_free(zone_file);
	    free(date_string);

	    if( mask!=NULL )
	      g_free(mask);
	    if( rev_ipv4_address!=NULL )
	      free(rev_ipv4_address);
	    if( rev_ipv6_address!=NULL )
	      free(rev_ipv6_address);
    	    return;
	}
	zone_data = g_strconcat(
	"$TTL 3D; Reverse zone IPv4->DNS\n",
	"@\tIN\tSOA\t", domain_name, ".\thostmaster.", domain_name, ". (\n"
	"\t\t\t", date_string, "\t;\n",
	"\t\t\t8H\t\t;\n",
	"\t\t\t2H\t\t;\n",
	"\t\t\t1W\t\t;\n",
	"\t\t\t1D )\t\t;\n\n",
	NULL);
	fputs(zone_data, new_zone_fp);
	g_free(zone_data);
	
	fclose(new_zone_fp);
	g_free(zone_file);
    }

    /* Add the reverse IPv6 zone */
    if( have_ipv6 && ! zone_exists(rev_ipv6_address) )
    {
	if( active_index <= 0 )
	{
	    // dumped mask
	    zone_data = g_strconcat(
	    "\nzone \"", rev_ipv6_address, ".ip6.arpa\" {\n",
	    "\ttype master;\n"
	    "\tfile \"sites/", domain_name, "/reverse.zone.ipv6\";\n",
	    "\tallow-transfer { 127.0.0.1; };\n",
	    "\tallow-update { none; };\n",
	    "\tallow-query { any; };\n",
	    "\tzone-statistics yes;\n",
	    "\tnotify no;\n",
	    "\talso-notify { };\n",
	    "};\n",
	    NULL);
	    fputs(zone_data, fp);
	    g_free(zone_data);
	}
	if( active_index == 1 )
	{
	    // dumped mask
	    zone_data = g_strconcat(
	    "\nzone \"", rev_ipv6_address, ".ip6.arpa\" {\n",
	    "\ttype slave;\n"
	    "\tfile \"sites/", domain_name, "/reverse.zone.ipv6\";\n",
	    "\tallow-transfer { 127.0.0.1; };\n",
	    "\tallow-query { any; };\n",
	    "\tmasters { 127.0.0.1; };\n",
	    "\tzone-statistics yes;\n",
	    "\tnotify no;\n",
	    "\talso-notify { };\n",
	    "};\n",
	    NULL);
	    fputs(zone_data, fp);
	    g_free(zone_data);
	}	

	/* Add the reverse IPv6 zone file */
	zone_file = g_strdup_printf("%s/etc/sites/%s/reverse.zone.ipv6", CHROOT_PATH_BUF, domain_name);
	if((new_zone_fp=fopen(zone_file, "w+"))==NULL)
	{
    	    info = g_strdup_printf(_("Couldnt write the reverse IPv6 zone file here:\n%s\n"), zone_file);
	    show_info(info);
	    g_free(info);
	    g_free(zone_file);
	    free(date_string);

	    if( mask!=NULL )
	      g_free(mask);
	    if( rev_ipv4_address!=NULL )
	      free(rev_ipv4_address);
	    if( rev_ipv6_address!=NULL )
	      free(rev_ipv6_address);

    	    return;
	}
	zone_data = g_strconcat(
	"$TTL 3D; Reverse zone IPv6->DNS\n",
	"@\tIN\tSOA\t", domain_name, ".\thostmaster.", domain_name, ". (\n"
	"\t\t\t", date_string, "\t;\n",
	"\t\t\t8H\t\t;\n",
	"\t\t\t2H\t\t;\n",
	"\t\t\t1W\t\t;\n",
	"\t\t\t1D )\t\t;\n\n",
	NULL);
	fputs(zone_data, new_zone_fp);
	g_free(zone_data);

	fclose(new_zone_fp);
	g_free(zone_file);
    }
    fclose(fp);

    free(date_string);

    if( have_ipv4 )    
      free(rev_ipv4_address);
    if( have_ipv6 )
      free(rev_ipv6_address);

    if( mask!=NULL )
      g_free(mask);
//	if( rev_ipv4_address!=NULL )
//	  free(rev_ipv4_address);
//	if( rev_ipv6_address!=NULL )
//	  free(rev_ipv6_address);


    /* Chown -R NAMED_USER:NAMED_USER /var/named/etc/sites/ so that named can write a journal (.jnl) etc */
    command = g_strdup_printf("chown -R %s:%s %s/etc/sites/*", NAMED_USER, NAMED_USER, CHROOT_PATH_BUF);
    if( ! run_command(command) )
    {
        info = g_strdup_printf(_("Error running command:\n%s\n"), command);
        show_info(info);
        g_free(info);
    }
    g_free(command);


    /* Populate the relevant gui sections */
    populate_domain_treeview(widgets);

    populate_resource_treeview(widgets);

    /* FIXME set this as the selected domain, the first domain is selected as is. */
    // Select_last_domain();
}
