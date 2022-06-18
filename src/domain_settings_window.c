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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "allocate.h"
#include "gettext.h"
#include "widgets.h"
#include "show_info.h"
#include "commands.h"
#include "make_settings_entries.h"
#include "make_settings_combos.h"
#include "populate_domain_treeview.h"
#include "populate_resource_treeview.h"
#include "populate_log_tab.h"
#include "reread_conf.h"
#include "domain_settings_type_combo_changed.h"
#include "domain_settings_window.h"

#define MAX_CONF_LINE 1000

extern char SYSLOG_PATH_BUF[1024];
extern char CHROOT_PATH_BUF[1024];

extern char global_domain_name[1024];
extern char global_domain_ipv4_address[1024];
extern char global_domain_ipv6_address[1024];
extern char global_domain_type[1024];



gchar * make_soa_header(const gchar *a, gchar *b, gchar *c,
			gchar *d, gchar *e, gchar *f,
				    gchar *g, gchar *h)
{
    /* Constructs a SOA header like this... */
/*    
    $TTL 3D;
    @       IN      SOA     example.org.    hostmaster.example.org. (
                            2008021201      ; Serial
                            8H              ; Refresh
                            2H              ; Retry
                    	    1W              ; Expire
                            1D )            ; Min TTL
*/
    gchar *header = g_strconcat(
    "$TTL ", a, ";\n",
    "@\tIN\tSOA\t", b, ".\t", c, ". (\n",
    "\t\t\t", d, "\t; Serial\n",
    "\t\t\t", e, "\t\t; Refresh\n",
    "\t\t\t", f, "\t\t; Retry\n",
    "\t\t\t", g, "\t\t; Expire\n",
    "\t\t\t", h, " )\t\t; Min TTL\n",
    NULL);

    return header;
}


void change_soa_header_in_file(gchar *soa_header, gchar *zone_file)
{
    FILE *fp;
    long file_size;
    int have_soa_end = 0;
    char *line, *new_zone;
    gchar *info;

    if((fp=fopen(zone_file, "r"))==NULL)
    {
	info = g_strdup_printf(_("Error: Could not change SOA header in zone file:\n%s\n"), zone_file);
	show_info(info);
	g_free(info);
	return;
    }
    fseek(fp, 0, SEEK_END);
    file_size = ftell(fp);
    rewind(fp);

    line = allocate(file_size+1);

    if( file_size > 1 )
    while(fgets(line, file_size, fp)!=NULL)
    {    
	if( strstr(line, ")") )
	{
	    have_soa_end = 1;
	    break;	
	}
    }

    /* No valid soa header found, add it at the top of the file */
    if( ! have_soa_end )
    {
	rewind(fp);
    }

    /* This will contain our SOA header and
       the rest of the zone data */
    new_zone = allocate(file_size+1024);

    strcat(new_zone, soa_header);

    /* Collect all lines starting after the soa header */
    if( file_size > 1 )
    while(fgets(line, file_size, fp)!=NULL)
    {    
	strcat(new_zone, line);
    }

    fclose(fp);
    free(line);
    
    /* Write the new zone */ 
    if((fp=fopen(zone_file, "w+"))==NULL)
    {
	info = g_strdup_printf(_("Error: Could not change SOA header in zone file:\n%s\n"), zone_file);
	show_info(info);
	g_free(info);

	free(new_zone);
	return;
    }
    fputs(new_zone, fp);
    fclose(fp);
    free(new_zone);
}


void show_domain_settings_window(struct w *widgets)
{
    FILE *fp;
    long file_size;
    GtkWidget *domain_settings_vbox;
    GtkWidget *scrolled_window;
    GtkWidget *frame;
    GtkWidget *table;
    GtkTooltips *tooltips;
    gchar *utf8=NULL;
    gchar *info, *fwd_zone_path, *domain_line, *named_conf;
    int a=0, b=1;
    char *line, *tmp;
    int i, domain_found = 0;
    int soa_found = 0;

    tooltips = gtk_tooltips_new();

    widgets->domain_settings_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_position(GTK_WINDOW (widgets->domain_settings_window), GTK_WIN_POS_CENTER);
    gtk_widget_set_size_request(widgets->domain_settings_window, -1, -1);

    /* Set window information */
    info = g_strdup_printf(_("GADMIN-BIND %s domain settings"), VERSION);
    utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);
    gtk_window_set_title(GTK_WINDOW(widgets->domain_settings_window), utf8);
    g_free(info);

    g_signal_connect(GTK_WINDOW(widgets->domain_settings_window), "delete_event", 
		     G_CALLBACK (gtk_widget_destroy), NULL);

    /* Add a vbox to the window */
    domain_settings_vbox = gtk_vbox_new(FALSE, 0);
    gtk_container_add(GTK_CONTAINER(widgets->domain_settings_window), domain_settings_vbox);

    /* Add a scrolled window to the vbox */
    scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_box_pack_start(GTK_BOX(domain_settings_vbox), scrolled_window, TRUE, TRUE, 0);

    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
                                            	       GTK_POLICY_NEVER,
                                                       GTK_POLICY_ALWAYS);
    /* Set a larger vertical size */
    gtk_widget_set_size_request(scrolled_window, -1, 400);

    /* Add a frame with a table to the scrolled window */
    info = g_strdup_printf(_("Domain settings:"));
    utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);
    g_free(info);
    frame = gtk_frame_new(utf8);

    /* A table with 19 settings and 2 columns */
    table = gtk_table_new(19, 2, FALSE);
    /* Add a table to the frame */
    gtk_container_add(GTK_CONTAINER(frame), table);
    gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scrolled_window), frame);



    /* Domain name */
    info = g_strdup_printf(_(" Domain name: "));
    utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);
    widgets->domain_settings_entry[0] = make_entry_with_label(GTK_TABLE(table), utf8,    0,1,a,b,350);
    g_free(info);
    utf8 = g_locale_to_utf8(global_domain_name, strlen(global_domain_name), NULL, NULL, NULL);
    gtk_entry_set_text(GTK_ENTRY(widgets->domain_settings_entry[0]), utf8);
    info = g_strdup_printf(_("If you change domain name and hit apply the entire domain will be transformed."));
    utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);
    gtk_tooltips_set_tip(tooltips, widgets->domain_settings_entry[0], utf8, NULL);
    g_free(info);
    a++; b++;

    /* IPv4 Network address */
    info = g_strdup_printf(_(" IPv4 network address: "));
    utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);
    widgets->domain_settings_entry[1] = make_entry_with_label(GTK_TABLE(table), utf8,    0,1,a,b,350);
    g_free(info);
    utf8 = g_locale_to_utf8(global_domain_ipv4_address, strlen(global_domain_ipv4_address), NULL, NULL, NULL);
    gtk_entry_set_text(GTK_ENTRY(widgets->domain_settings_entry[1]), utf8);
    info = g_strdup_printf(_("192.168.0 or 192.168.0.1-254"));
    utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);
    gtk_tooltips_set_tip(tooltips, widgets->domain_settings_entry[1], utf8, NULL);
    g_free(info);
    a++; b++;

    /* IPv6 Network address */
    info = g_strdup_printf(_(" IPv6 network address: "));
    utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);
    widgets->domain_settings_entry[2] = make_entry_with_label(GTK_TABLE(table), utf8,    0,1,a,b,350);
    g_free(info);
    utf8 = g_locale_to_utf8(global_domain_ipv6_address, strlen(global_domain_ipv6_address), NULL, NULL, NULL);
    gtk_entry_set_text(GTK_ENTRY(widgets->domain_settings_entry[2]), utf8);
    info = g_strdup_printf(_("fe80::250:daff:fe4d:e6c7/64"));
    utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);
    gtk_tooltips_set_tip(tooltips, widgets->domain_settings_entry[2], utf8, NULL);
    g_free(info);
    a++; b++;

    /* Domain type combo */
    info = g_strdup_printf(_(" Domain type: "));
    utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);
    widgets->domain_settings_combo[0] = make_combo_with_label(GTK_TABLE(table), utf8,    0,1,a,b,350);
    g_free(info);
    info = g_strdup_printf(_("master"));
    utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);
    gtk_combo_box_append_text(GTK_COMBO_BOX(widgets->domain_settings_combo[0]), utf8);
    g_free(info);
    info = g_strdup_printf(_("slave"));
    utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);
    gtk_combo_box_append_text(GTK_COMBO_BOX(widgets->domain_settings_combo[0]), utf8);
    g_free(info);
    a++; b++;

    if( strstr(global_domain_type, "master") )
      gtk_combo_box_set_active(GTK_COMBO_BOX(widgets->domain_settings_combo[0]), 0);

    if( strstr(global_domain_type, "slave") )
      gtk_combo_box_set_active(GTK_COMBO_BOX(widgets->domain_settings_combo[0]), 1);

    /* Different widgets are visible depending on whats selected in the type combo */
    g_signal_connect_swapped(G_OBJECT(widgets->domain_settings_combo[0]), "changed",
                             G_CALLBACK(domain_settings_type_combo_changed), widgets);    

    /* Administrator email */
    info = g_strdup_printf(_(" Administrator email: "));
    utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);
    widgets->domain_settings_entry[3] = make_entry_with_label(GTK_TABLE(table), utf8, 0,1,a,b,350);
    g_free(info);
    info = g_strdup_printf(_("hostmaster@example.org"));
    utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);
    gtk_tooltips_set_tip(tooltips, widgets->domain_settings_entry[3], utf8, NULL);
    g_free(info);
    a++; b++;

    /* Domain serial */
    info = g_strdup_printf(_(" Domain serial: "));
    utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);
    widgets->domain_settings_entry[4] = make_entry_with_label(GTK_TABLE(table), utf8,    0,1,a,b,350);
    g_free(info);
    info = g_strdup_printf(_("Year+Month+Day+2-digit serial, IE: 2008021204"));
    utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);
    gtk_tooltips_set_tip(tooltips, widgets->domain_settings_entry[4], utf8, NULL);
    g_free(info);
    a++; b++;

    /* Default TTL */
    info = g_strdup_printf(_(" Default TTL: "));
    utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);
    widgets->domain_settings_entry[5] = make_entry_with_label(GTK_TABLE(table), utf8,    0,1,a,b,350);
    g_free(info);
    info = g_strdup_printf(_("Default time to live, IE: 3D (for 3 days)"));
    utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);
    gtk_tooltips_set_tip(tooltips, widgets->domain_settings_entry[5], utf8, NULL);
    g_free(info);
    a++; b++;

    /* Refresh */
    info = g_strdup_printf(_(" Refresh: "));
    utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);
    widgets->domain_settings_entry[6] = make_entry_with_label(GTK_TABLE(table), utf8,    0,1,a,b,350);
    g_free(info);
    info = g_strdup_printf(_("Refresh interval, IE: 8H (for 8 hours)"));
    utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);
    gtk_tooltips_set_tip(tooltips, widgets->domain_settings_entry[6], utf8, NULL);
    g_free(info);
    a++; b++;

    /* Retry */
    info = g_strdup_printf(_(" Retry: "));
    utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);
    widgets->domain_settings_entry[7] = make_entry_with_label(GTK_TABLE(table), utf8,    0,1,a,b,350);
    g_free(info);
    info = g_strdup_printf(_("Retry interval, IE: 2H (for 2 hours)"));
    utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);
    gtk_tooltips_set_tip(tooltips, widgets->domain_settings_entry[7], utf8, NULL);
    g_free(info);
    a++; b++;

    /* Expire */
    info = g_strdup_printf(_(" Expire: "));
    utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);
    widgets->domain_settings_entry[8] = make_entry_with_label(GTK_TABLE(table), utf8,    0,1,a,b,350);
    g_free(info);
    info = g_strdup_printf(_("Expire, IE: 4W (for 4 weeks after not beeing able to update)"));
    utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);
    gtk_tooltips_set_tip(tooltips, widgets->domain_settings_entry[8], utf8, NULL);
    g_free(info);
    a++; b++;

    /* Min TTL */
    info = g_strdup_printf(_(" Minimum TTL: "));
    utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);
    widgets->domain_settings_entry[9] = make_entry_with_label(GTK_TABLE(table), utf8,    0,1,a,b,350);
    g_free(info);
    info = g_strdup_printf(_("Minimum TTL, IE: 1D (for 1 day)"));
    utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);
    gtk_tooltips_set_tip(tooltips, widgets->domain_settings_entry[9], utf8, NULL);
    g_free(info);
    a++; b++;

    /* allow-update */
    info = g_strdup_printf(_(" Allow update: "));
    utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);
    widgets->domain_settings_entry[10] = make_entry_with_label(GTK_TABLE(table), utf8, 0,1,a,b,350);
    g_free(info);
    info = g_strdup_printf(_("Hosts allowed to submit dynamic DNS updates (DDNS) delimited with a \";\" IE: 127.0.0.1; 192.168.0.0/24; any; or none;"));
    utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);
    gtk_tooltips_set_tip(tooltips, widgets->domain_settings_entry[10], utf8, NULL);
    g_free(info);
    a++; b++;

    /* allow-transfer */
    info = g_strdup_printf(_(" Allow transfer: "));
    utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);
    widgets->domain_settings_entry[11] = make_entry_with_label(GTK_TABLE(table), utf8, 0,1,a,b,350);
    g_free(info);
    info = g_strdup_printf(_("Hosts that are allowed to make zone transfers (copy the entire zone) delimited with a \";\" IE: 127.0.0.1; 192.168.0.0/24; any; or none;"));
    utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);
    gtk_tooltips_set_tip(tooltips, widgets->domain_settings_entry[11], utf8, NULL);
    g_free(info);
    a++; b++;

    /* allow-query */
    info = g_strdup_printf(_(" Allow query: "));
    utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);
    widgets->domain_settings_entry[12] = make_entry_with_label(GTK_TABLE(table), utf8, 0,1,a,b,350);
    g_free(info);
    info = g_strdup_printf(_("Hosts allowed to query this DNS, delimited with a \";\" IE: 127.0.0.1; 192.168.0.0/24; any; or none;"));
    utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);
    gtk_tooltips_set_tip(tooltips, widgets->domain_settings_entry[12], utf8, NULL);
    g_free(info);
    a++; b++;

    /* Notify combo */
    info = g_strdup_printf(_(" Send notifications: "));
    utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);
    widgets->domain_settings_combo[2] = make_combo_with_label(GTK_TABLE(table), utf8, 0,1,a,b,350);
    g_free(info);
    info = g_strdup_printf(_("yes"));
    utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);
    gtk_combo_box_append_text(GTK_COMBO_BOX(widgets->domain_settings_combo[2]), utf8);
    g_free(info);
    info = g_strdup_printf(_("no"));
    utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);
    gtk_combo_box_append_text(GTK_COMBO_BOX(widgets->domain_settings_combo[2]), utf8);
    g_free(info);
    info = g_strdup_printf(_("explicit"));
    utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);
    gtk_combo_box_append_text(GTK_COMBO_BOX(widgets->domain_settings_combo[2]), utf8);
    g_free(info);
    a++; b++;

    /* Also-notify (for masters only) */
    info = g_strdup_printf(_(" Also notify: "));
    utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);
    widgets->domain_settings_entry[13] = make_entry_with_label(GTK_TABLE(table), utf8, 0,1,a,b,350);
    g_free(info);
    info = g_strdup_printf(_("Servers that should recieve zone update notifications delimited with a \";\" IE: 127.0.0.1; 192.168.0.100;"));
    utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);
    gtk_tooltips_set_tip(tooltips, widgets->domain_settings_entry[13], utf8, NULL);
    g_free(info);
    a++; b++;

    /* Allow-notify (for slaves only) */
    info = g_strdup_printf(_(" Allow notify: "));
    utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);
    widgets->domain_settings_entry[14] = make_entry_with_label(GTK_TABLE(table), utf8, 0,1,a,b,350);
    g_free(info);
    info = g_strdup_printf(_("Servers allowed to notify this server for zone updates delimited with a \";\" IE: 127.0.0.1; 192.168.0.100; any; or none;"));
    utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);
    gtk_tooltips_set_tip(tooltips, widgets->domain_settings_entry[14], utf8, NULL);
    g_free(info);
    a++; b++;

    /* Statistics combo */
    info = g_strdup_printf(_(" Domain statistics: "));
    utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);
    widgets->domain_settings_combo[1] = make_combo_with_label(GTK_TABLE(table), utf8, 0,1,a,b,350);
    g_free(info);
    info = g_strdup_printf(_("yes"));
    utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);
    gtk_combo_box_append_text(GTK_COMBO_BOX(widgets->domain_settings_combo[1]), utf8);
    g_free(info);
    info = g_strdup_printf(_("no"));
    utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);
    gtk_combo_box_append_text(GTK_COMBO_BOX(widgets->domain_settings_combo[1]), utf8);
    g_free(info);
    a++; b++;

    /* Masters (Only for slave domains) */
    info = g_strdup_printf(_(" Master domains: "));
    utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);
    widgets->domain_settings_entry[15] = make_entry_with_label(GTK_TABLE(table), utf8, 0,1,a,b,350);
    g_free(info);
    info = g_strdup_printf(_("IP-addresses of the master domains delimited with a \";\" IE: 127.0.0.1; 192.168.0.100; none;"));
    utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);
    gtk_tooltips_set_tip(tooltips, widgets->domain_settings_entry[15], utf8, NULL);
    g_free(info);
    a++; b++;

    if( utf8!=NULL )
      g_free(utf8);



    /* Populations begin... */


    /* Insert SOA data from the selected domains forward zone */
    fwd_zone_path = g_strdup_printf("%s/etc/sites/%s/forward.zone", CHROOT_PATH_BUF, global_domain_name);
    if((fp=fopen(fwd_zone_path, "r"))==NULL)
    {
	info = g_strdup_printf(_("Error: could not find the domains forward zone here:\n%s\n"), fwd_zone_path);
	show_info(info);
	g_free(info);

	g_free(fwd_zone_path);
	return;
    }
    fseek(fp, 0, SEEK_END);
    file_size = ftell(fp);
    rewind(fp);

    line = allocate(file_size+1);
    tmp = allocate(1024);

    if( file_size > 1 )
    while(fgets(line, file_size, fp)!=NULL)
    {    
	if( strstr(line, "#") || strlen(line) > 1000 )
	  continue;

	/* Get default TTL */
	if( strstr(line, "$TTL") )
	{
	    sscanf(line, "%*s %s", tmp); /* Compensate for semicolon */
	    if( strstr(tmp, ";") )
	      utf8 = g_locale_to_utf8(tmp, strlen(tmp)-1, NULL, NULL, NULL);
	    else
	      utf8 = g_locale_to_utf8(tmp, strlen(tmp), NULL, NULL, NULL);
	    
	    gtk_entry_set_text(GTK_ENTRY(widgets->domain_settings_entry[5]), utf8);
	}

	/* Administrator email */
	if( strstr(line, "SOA") )
	{
	    soa_found = 1;    
	
	    /* Admin Email */
	    sscanf(line, "%*s %*s %*s %*s %s", tmp); /* -1 for the ending dot */
	    utf8 = g_locale_to_utf8(tmp, strlen(tmp)-1, NULL, NULL, NULL);
	    gtk_entry_set_text(GTK_ENTRY(widgets->domain_settings_entry[3]), utf8);

	    /* Domain Serial */
	    while(fgets(line, file_size, fp)!=NULL)
	      if( strlen(line) < 1000 && strlen(line) > 1 )
	        break;
	    sscanf(line, "%s", tmp);
	    utf8 = g_locale_to_utf8(tmp, strlen(tmp), NULL, NULL, NULL);
	    gtk_entry_set_text(GTK_ENTRY(widgets->domain_settings_entry[4]), utf8);

	    /* Refresh */
	    while(fgets(line, file_size, fp)!=NULL)
	      if( strlen(line) < 1000 && strlen(line) > 1 )
	        break;
	    sscanf(line, "%s", tmp);
	    utf8 = g_locale_to_utf8(tmp, strlen(tmp), NULL, NULL, NULL);
	    gtk_entry_set_text(GTK_ENTRY(widgets->domain_settings_entry[6]), utf8);

	    /* Retry */
	    while(fgets(line, file_size, fp)!=NULL)
	      if( strlen(line) < 1000 && strlen(line) > 1 )
	        break;
	    sscanf(line, "%s", tmp);
	    utf8 = g_locale_to_utf8(tmp, strlen(tmp), NULL, NULL, NULL);
	    gtk_entry_set_text(GTK_ENTRY(widgets->domain_settings_entry[7]), utf8);

	    /* Expire */
	    while(fgets(line, file_size, fp)!=NULL)
	      if( strlen(line) < 1000 && strlen(line) > 1 )
	        break;
	    sscanf(line, "%s", tmp);
	    utf8 = g_locale_to_utf8(tmp, strlen(tmp), NULL, NULL, NULL);
	    gtk_entry_set_text(GTK_ENTRY(widgets->domain_settings_entry[8]), utf8);

	    /* Min TTL */
	    while(fgets(line, file_size, fp)!=NULL)
	      if( strlen(line) < 1000 && strlen(line) > 1 )
	        break;
	    sscanf(line, "%s", tmp);
	    utf8 = g_locale_to_utf8(tmp, strlen(tmp), NULL, NULL, NULL);
	    gtk_entry_set_text(GTK_ENTRY(widgets->domain_settings_entry[9]), utf8);
	}
	if( soa_found )
	  break;
    }
    free(line);
    free(tmp);
    fclose(fp);


    /* Insert all other values from this chroots named.conf */
    if(strlen(CHROOT_PATH_BUF) > 0){
        named_conf = g_strdup_printf("%s%s", CHROOT_PATH_BUF, BIND_CONF);
    }else{
        named_conf = g_strdup_printf("%s", BIND_CONF);
    }
    if((fp=fopen(named_conf, "r"))==NULL)
    {
	info = g_strdup_printf(_("Error: could not open named.conf here:\n%s\n"), named_conf);
	show_info(info);
	g_free(info);

	g_free(named_conf);
	return;
    }
    fseek(fp, 0, SEEK_END);
    file_size = ftell(fp);
    rewind(fp);

    line = allocate(file_size+1);
    tmp = allocate(1024);


    domain_line = g_strdup_printf("zone \"%s\" IN", global_domain_name);

    if( file_size > 1 )
    while(fgets(line, file_size, fp)!=NULL)
    {    
	if( strstr(line, "#") )
	  continue;

	if( strstr(line, domain_line) )
	{
	    domain_found = 1;

	    while(fgets(line, file_size, fp)!=NULL)
	    {    
		if( strstr(line, "#") )
		  continue;

		/* Stop when the correct zone has been read */
		if( strstr(line, "zone \"") && domain_found )
	          break;

		if( strstr(line, "allow-update") && strstr(line, "{") )
		{
		    for(i=0; i<1000; i++)
		      if( line[i]=='{' )
		        break;

		    snprintf(tmp, 1000, "%s", &line[i+2]);
		    tmp[strlen(tmp)-4]='\0';
		    utf8 = g_locale_to_utf8(tmp, strlen(tmp), NULL, NULL, NULL);
		    gtk_entry_set_text(GTK_ENTRY(widgets->domain_settings_entry[10]), utf8);
		}

		if( strstr(line, "allow-transfer") && strstr(line, "{") )
		{
		    for(i=0; i<1000; i++)
		      if( line[i]=='{' )
		        break;
		    
		    snprintf(tmp, 1000, "%s", &line[i+2]);
		    tmp[strlen(tmp)-4]='\0';
		    utf8 = g_locale_to_utf8(tmp, strlen(tmp), NULL, NULL, NULL);
		    gtk_entry_set_text(GTK_ENTRY(widgets->domain_settings_entry[11]), utf8);
		}

		if( strstr(line, "allow-query") && strstr(line, "{") )
		{
		    for(i=0; i<1000; i++)
		      if( line[i]=='{' )
		        break;
		    
		    snprintf(tmp, 1000, "%s", &line[i+2]);
		    tmp[strlen(tmp)-4]='\0';
		    utf8 = g_locale_to_utf8(tmp, strlen(tmp), NULL, NULL, NULL);
		    gtk_entry_set_text(GTK_ENTRY(widgets->domain_settings_entry[12]), utf8);
		}

		if( strstr(line, "also-notify") && strstr(line, "{") )
		{
		    for(i=0; i<1000; i++)
		      if( line[i]=='{' )
		        break;
		    
		    snprintf(tmp, 1000, "%s", &line[i+2]);
		    tmp[strlen(tmp)-4]='\0';
		    if( strlen(tmp) > 5 )
		    {
			utf8 = g_locale_to_utf8(tmp, strlen(tmp), NULL, NULL, NULL);
			gtk_entry_set_text(GTK_ENTRY(widgets->domain_settings_entry[13]), utf8);
		    }
		}

		if( strstr(line, "allow-notify") && strstr(line, "{") )
		{
		    for(i=0; i<1000; i++)
		      if( line[i]=='{' )
		        break;
		    
		    snprintf(tmp, 1000, "%s", &line[i+2]);
		    tmp[strlen(tmp)-4]='\0';

		    if( strlen(tmp) > 3 )
		    {
			utf8 = g_locale_to_utf8(tmp, strlen(tmp), NULL, NULL, NULL);
			gtk_entry_set_text(GTK_ENTRY(widgets->domain_settings_entry[14]), utf8);
		    }
		}

		if( strstr(line, "masters") && strstr(line, "{") )
		{
		    for(i=0; i<1000; i++)
		      if( line[i]=='{' )
		        break;
		    
		    snprintf(tmp, 1000, "%s", &line[i+2]);
		    tmp[strlen(tmp)-4]='\0';
		    utf8 = g_locale_to_utf8(tmp, strlen(tmp), NULL, NULL, NULL);
		    gtk_entry_set_text(GTK_ENTRY(widgets->domain_settings_entry[15]), utf8);
		}

		if( strstr(line, "zone-statistics") && strstr(line, ";") )
		{
		    snprintf(tmp, 1000, "%s", &line[16]);
		    tmp[strlen(tmp)-2]='\0';

		    if( strstr(tmp, "yes") )
    		      gtk_combo_box_set_active(GTK_COMBO_BOX(widgets->domain_settings_combo[1]), 0);
		    else
		    if( strstr(tmp, "no") )
    		      gtk_combo_box_set_active(GTK_COMBO_BOX(widgets->domain_settings_combo[1]), 1);
		}

		if( strstr(line, "notify") && ! strstr(line, "also-notify") 
		&& ! strstr(line, "allow-notify") && strstr(line, ";") )
		{
		    snprintf(tmp, 1000, "%s", &line[8]);
		    tmp[strlen(tmp)-2]='\0';

		    if( strstr(tmp, "yes") )
    		      gtk_combo_box_set_active(GTK_COMBO_BOX(widgets->domain_settings_combo[2]), 0);
		    else
		    if( strstr(tmp, "no") )
    		      gtk_combo_box_set_active(GTK_COMBO_BOX(widgets->domain_settings_combo[2]), 1);
		    else
		    if( strstr(tmp, "explicit") )
    		      gtk_combo_box_set_active(GTK_COMBO_BOX(widgets->domain_settings_combo[2]), 2);
		}

	    }
	    if( domain_found )
	      break;
	}
    }
    fclose(fp);
    free(line);
    free(tmp);
    g_free(named_conf);

    if( utf8!=NULL )
      g_free(utf8);

    GtkWidget *domain_settings_hbuttonbox = gtk_hbutton_box_new();
    gtk_box_pack_start(GTK_BOX(domain_settings_vbox), domain_settings_hbuttonbox, FALSE, FALSE, 0);
    gtk_button_box_set_layout(GTK_BUTTON_BOX(domain_settings_hbuttonbox), GTK_BUTTONBOX_SPREAD);

    GtkWidget *apply_domain_settings_button = gtk_button_new_from_stock(GTK_STOCK_APPLY);
    gtk_container_add(GTK_CONTAINER(domain_settings_hbuttonbox), apply_domain_settings_button);
    g_signal_connect_swapped(G_OBJECT(apply_domain_settings_button), "clicked", 
                             G_CALLBACK(apply_domain_settings_button_clicked), widgets);
    
    GtkWidget *cancel_domain_settings_button = gtk_button_new_from_stock(GTK_STOCK_CANCEL);
    gtk_container_add(GTK_CONTAINER(domain_settings_hbuttonbox), cancel_domain_settings_button);
    g_signal_connect_swapped(G_OBJECT(cancel_domain_settings_button), "clicked", 
                             G_CALLBACK(cancel_domain_settings_button_clicked), widgets);

    gtk_widget_show_all(widgets->domain_settings_window);

    /* Domain type determines what entries are visible */
    domain_settings_type_combo_changed(widgets);
}


void cancel_domain_settings_button_clicked(struct w *widgets)
{
    gtk_widget_destroy(widgets->domain_settings_window);
}


/* FIXME: Uses sed... */
void apply_domain_settings_button_clicked(struct w *widgets)
{
    /* The apply button in the domain settings window */
    FILE *fp;
    long file_size, type_pos=0;
    int new_domain_name=0, domain_type_slave=0;
    char *line, *new_conf, *tmp;
    gint active_index = 0;
    gchar *cmd, *info, *named_conf, *utf8=NULL;
    char *forward, *reverse_file;
    int found_rew_zone = 0;
    int found_fwd_zone = 0;

    G_CONST_RETURN gchar *domain_name = gtk_entry_get_text(GTK_ENTRY(widgets->domain_settings_entry[0]));

    /* FIXME: All matching A, AAAA and PTR records must be changed (if used) */
//    G_CONST_RETURN gchar *ipv4_network_address = gtk_entry_get_text(GTK_ENTRY(widgets->domain_settings_entry[1]));
//    G_CONST_RETURN gchar *ipv6_network_address = gtk_entry_get_text(GTK_ENTRY(widgets->domain_settings_entry[2]));

    G_CONST_RETURN gchar *admin_email = gtk_entry_get_text(GTK_ENTRY(widgets->domain_settings_entry[3]));
    G_CONST_RETURN gchar *domain_serial = gtk_entry_get_text(GTK_ENTRY(widgets->domain_settings_entry[4]));

    G_CONST_RETURN gchar *def_ttl = gtk_entry_get_text(GTK_ENTRY(widgets->domain_settings_entry[5]));
    G_CONST_RETURN gchar *refresh = gtk_entry_get_text(GTK_ENTRY(widgets->domain_settings_entry[6]));
    G_CONST_RETURN gchar *retry   = gtk_entry_get_text(GTK_ENTRY(widgets->domain_settings_entry[7]));
    G_CONST_RETURN gchar *expire  = gtk_entry_get_text(GTK_ENTRY(widgets->domain_settings_entry[8]));
    G_CONST_RETURN gchar *min_ttl = gtk_entry_get_text(GTK_ENTRY(widgets->domain_settings_entry[9]));

    G_CONST_RETURN gchar *allow_update = gtk_entry_get_text(GTK_ENTRY(widgets->domain_settings_entry[10]));
    G_CONST_RETURN gchar *allow_transfer = gtk_entry_get_text(GTK_ENTRY(widgets->domain_settings_entry[11]));
    G_CONST_RETURN gchar *allow_query = gtk_entry_get_text(GTK_ENTRY(widgets->domain_settings_entry[12]));
    G_CONST_RETURN gchar *also_notify = gtk_entry_get_text(GTK_ENTRY(widgets->domain_settings_entry[13]));

    G_CONST_RETURN gchar *allow_notify = gtk_entry_get_text(GTK_ENTRY(widgets->domain_settings_entry[14]));
    G_CONST_RETURN gchar *masters = gtk_entry_get_text(GTK_ENTRY(widgets->domain_settings_entry[15]));


    /* Setup the zone paths */
    gchar *fwd_zone_path = g_strdup_printf("%s/etc/sites/%s/forward.zone", CHROOT_PATH_BUF, global_domain_name);
    gchar *rev_ipv4_zone_path = g_strdup_printf("%s/etc/sites/%s/reverse.zone.ipv4", CHROOT_PATH_BUF, global_domain_name);
    gchar *rev_ipv6_zone_path = g_strdup_printf("%s/etc/sites/%s/reverse.zone.ipv6", CHROOT_PATH_BUF, global_domain_name);


    /* Update SOA headers in all zone files for the selected domain */
    gchar *soa_header = make_soa_header((gchar *)def_ttl, (gchar *)domain_name, (gchar *)admin_email,
					    (gchar *)domain_serial, (gchar *)refresh, (gchar *)retry,
								    (gchar *)expire, (gchar *)min_ttl);
    change_soa_header_in_file(soa_header, fwd_zone_path);

    if( file_exists(rev_ipv4_zone_path) )
      change_soa_header_in_file(soa_header, rev_ipv4_zone_path);

    if( file_exists(rev_ipv6_zone_path) )
      change_soa_header_in_file(soa_header, rev_ipv6_zone_path);

    g_free(soa_header);


    named_conf = g_strdup_printf("%s%s", CHROOT_PATH_BUF, BIND_CONF);

    /* Check that we have a complete domain with all required files */
    if( ! file_exists(named_conf) )
    {
	info = g_strdup_printf(_("Error: named.conf does not exist here:\n%s\n"), named_conf);
	utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);
	show_info(info);
	g_free(info);
	if( utf8!=NULL )
    	  g_free(utf8);
	g_free(fwd_zone_path);
	g_free(rev_ipv4_zone_path);
	g_free(rev_ipv6_zone_path);
	g_free(named_conf);
	return;
    }

    if( ! file_exists(fwd_zone_path) )
    {
	info = g_strdup_printf(_("Error: Forward zone does not exist here:\n%s\n"), fwd_zone_path);
	utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);
	show_info(info);
	g_free(info);
	if( utf8!=NULL )
    	  g_free(utf8);
	g_free(fwd_zone_path);
	g_free(rev_ipv4_zone_path);
	g_free(rev_ipv6_zone_path);
	g_free(named_conf);
	return;
    }


    /* Atleast one of the 2 reverse zones (IPv4/IPv6) are required */
    if( ! file_exists(rev_ipv4_zone_path)
    &&  ! file_exists(rev_ipv6_zone_path) )
    {
	info = g_strdup_printf(_("Error: No reverse zones not found.\n"));
	utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);
	show_info(info);
	g_free(info);
	if( utf8!=NULL )
    	  g_free(utf8);
	g_free(fwd_zone_path);
	g_free(rev_ipv4_zone_path);
	g_free(rev_ipv6_zone_path);
	g_free(named_conf);
	return;
    }


    /* DNS type (combo) */
    gchar *domain_type = NULL;

    /* DNS statistics (combo) */
    gchar *domain_statistics = NULL;

    /* DNS Notify (combo) */
    gchar *domain_notify = NULL;

    /* DNS Type */    
    active_index = gtk_combo_box_get_active(GTK_COMBO_BOX(widgets->domain_settings_combo[0]));
    if( active_index <= 0 )
	domain_type = g_strdup_printf("master");
    else
    if( active_index == 1 )
	domain_type = g_strdup_printf("slave");
    else
      {
	 info = g_strdup_printf(_("DNS Type not set, no changes made.\n"));
	 utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);
	 show_info(info);
	 g_free(info);
	 if( utf8!=NULL )
    	   g_free(utf8);
	 g_free(fwd_zone_path);
	 g_free(rev_ipv4_zone_path);
	 g_free(rev_ipv6_zone_path);
	 g_free(named_conf);
	 return;
      }


    /* DNS Statistics */    
    active_index = gtk_combo_box_get_active(GTK_COMBO_BOX(widgets->domain_settings_combo[1]));
    if( active_index <= 0 )
	domain_statistics = g_strdup_printf("yes");
    else
    if( active_index == 1 )
	domain_statistics = g_strdup_printf("no");
    else
      {
	 domain_statistics = g_strdup_printf("yes");
	 info = g_strdup_printf(_("DNS statistics not set, setting it to yes\n"));
	 utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);
	 show_info(info);
	 g_free(info);
	 if( utf8!=NULL )
    	   g_free(utf8);
	 /* Dont return. */
      }


    /* DNS Notify */    
    active_index = gtk_combo_box_get_active(GTK_COMBO_BOX(widgets->domain_settings_combo[2]));
    if( active_index <= 0 )
	domain_notify = g_strdup_printf("yes");
    else
    if( active_index == 1 )
	domain_notify = g_strdup_printf("no");
    else
    if( active_index == 2 )
	domain_notify = g_strdup_printf("explicit");
    else
      {
	 domain_notify = g_strdup_printf("no");
	 info = g_strdup_printf(_("DNS notification not set, setting it to no.\n"));
	 utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);
	 show_info(info);
	 g_free(info);
	 if( utf8!=NULL )
    	   g_free(utf8);
	 /* Dont return. */
      }


    /* Some configuration values cant be used in slave mode */
    if( strstr(domain_type, "slave") )
      domain_type_slave = 1;
    else
      domain_type_slave = 0;

    /* If slave domain is selected it must have atleast one master domain server */
    if( domain_type_slave && strlen(masters) < 3 )
    {
	info = g_strdup_printf(_("Error: Slave domains requires atleast one master domain server.\n"));
	utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);
	show_info(info);
	g_free(info);
	if( utf8!=NULL )
    	  g_free(utf8);
	g_free(fwd_zone_path);
	g_free(rev_ipv4_zone_path);
	g_free(rev_ipv6_zone_path);
	g_free(domain_type);
	g_free(domain_statistics);
	g_free(named_conf);
	return;
    }

    
/* Sanity check in teh pirkinning */


    /* Domain names differ, set this domain name as the selected domain */
    if( strcmp(global_domain_name, domain_name) )
    {
	new_domain_name = 1;
    }


    /* Use sed to change the domain name in the forward zone */
    if( new_domain_name && file_exists(fwd_zone_path) )
    {
	/* sed -e s/drum.se/bbb.se/g CHROOT/etc/sites/forward.zone > CHROOT/etc/sites/forward.zone.tmp */
	cmd = g_strdup_printf("%s -e s/%s/%s/g %s > %s.tmp", 
              SED_BINARY, global_domain_name, domain_name, fwd_zone_path, fwd_zone_path);
	if( ! run_command(cmd) )
	{
	    printf("Error running command:\n%s\n", cmd);
	}
	g_free(cmd);

	/* mv CHROOT/etc/sites/SITENAME/forward.zone.tmp CHROOT/etc/sites/SITENAME/forward.zone */
	cmd = g_strdup_printf("mv %s.tmp %s", fwd_zone_path, fwd_zone_path);
	if( ! run_command(cmd) )
	{
	    printf("Error running command:\n%s\n", cmd);
	}
	g_free(cmd);
    }

    /* Use sed to change the domain name in the reverse IPv4 zone */
    if( new_domain_name && file_exists(rev_ipv4_zone_path) )
    {
	/* sed -e s/drum.se/bbb.se/g filepath > filepath.tmp */
	cmd = g_strdup_printf("%s -e s/%s/%s/g %s > %s.tmp", 
              SED_BINARY, global_domain_name, domain_name, rev_ipv4_zone_path, rev_ipv4_zone_path);
	if( ! run_command(cmd) )
	{
	    printf("Error running command:\n%s\n", cmd);
	}
	g_free(cmd);

	/* mv /path/to/zonefile.tmp path/to/zonefile */
	cmd = g_strdup_printf("mv %s.tmp %s", rev_ipv4_zone_path, rev_ipv4_zone_path);
	if( ! run_command(cmd) )
	{
	    printf("Error running command:\n%s\n", cmd);
	}
	g_free(cmd);
    }

    /* Use sed to change the domain name in the reverse IPv6 zone */
    if( new_domain_name && file_exists(rev_ipv6_zone_path) )
    {
	/* sed -e s/drum.se/bbb.se/g filepath > filepath.tmp */
	cmd = g_strdup_printf("%s -e s/%s/%s/g %s > %s.tmp", 
              SED_BINARY, global_domain_name, domain_name, rev_ipv6_zone_path, rev_ipv6_zone_path);
	if( ! run_command(cmd) )
	{
	    printf("Error running command:\n%s\n", cmd);
	}
	g_free(cmd);

	/* mv /path/to/zonefile.tmp path/to/zonefile */
	cmd = g_strdup_printf("mv %s.tmp %s", rev_ipv6_zone_path, rev_ipv6_zone_path);
	if( ! run_command(cmd) )
	{
	    printf("Error running command:\n%s\n", cmd);
	}
	g_free(cmd);
    }

    g_free(fwd_zone_path);
    g_free(rev_ipv4_zone_path);
    g_free(rev_ipv6_zone_path);


    /* Rename this domains directory */
    /* mv CHROOT/etc/sites/Domain.org CHROOT/etc/sites/NewDomain.org */
    if( new_domain_name )
    {
	cmd = g_strdup_printf("mv %s/etc/sites/%s %s/etc/sites/%s", 
	    CHROOT_PATH_BUF, global_domain_name, CHROOT_PATH_BUF, domain_name);
	if( ! run_command(cmd) )
	{
	    printf("Error running command:\n%s\n", cmd);
	    if( utf8!=NULL )
    	      g_free(utf8);
	    g_free(domain_statistics);
	    g_free(domain_type);
	    g_free(named_conf);
	    return;    
	}
	g_free(cmd);
    }


    /* Change this domains options in named.conf */
    if((fp=fopen(named_conf, "r"))==NULL)
    {
        /* Sanity checked in teh pirkinning */
        printf("Couldnt find:\n%s\n", named_conf);
	if( utf8!=NULL )
    	  g_free(utf8);
	g_free(domain_statistics);
	g_free(domain_type);
	g_free(named_conf);
        return;
    }
    fseek(fp, 0, SEEK_END);
    file_size = ftell(fp);
    rewind(fp);

    line = allocate(file_size+1);
    forward = allocate(MAX_CONF_LINE+1);
    reverse_file = allocate(MAX_CONF_LINE+1);
    tmp = allocate(MAX_CONF_LINE+1);

    new_conf = allocate(file_size+16384);

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

	/* Skip comments */
	if( strstr(line, "//") )
	  continue;

	/* Forward zone begin */
	if( strstr(line, "zone \"")
	&& strstr(line, global_domain_name) 
	&& ! found_fwd_zone )
	{
	    found_fwd_zone = 1;
	    strcat(new_conf, line);

	    /* For matching against the reverse zone file */
	    sscanf(line, "%*s %s", forward);
	    snprintf(tmp, MAX_CONF_LINE-1, "%s", &forward[1]);
	    snprintf(forward, strlen(tmp), "%s", tmp);
	    tmp[strlen(tmp)-1]='\0';
	    snprintf(reverse_file, strlen(tmp)+3, "/%s/", tmp);


	    /* Handle the options */
	    while(fgets(line, file_size, fp)!=NULL)
	    {
		/* Remove allow-update line */
		if( strstr(line, "allow-update") && strstr(line, "{") )
		  continue;
		    
		/* Remove masters line */
		if( strstr(line, "masters") && strstr(line, "{") )
		  continue;

		/* Remove allow-notify line */
		if( strstr(line, "allow-notify") && strstr(line, "{") )
		  continue;


		    
		/* The allow-transfer line must exist, also handles other options */
		if( strstr(line, "type ") && strstr(line, ";") )
		{
		    snprintf(tmp, 1000, "\ttype %s;\n", domain_type);
		    strcat(new_conf, tmp);
		}
		else
		if( strstr(line, "allow-transfer") && strstr(line, "{") )
		{
		    snprintf(tmp, 1000, "\tallow-transfer { %s };\n", allow_transfer);
		    strcat(new_conf, tmp);

		    /* If the domain type is master, we can have allow-update */
		    if( ! domain_type_slave )
		    {
			snprintf(tmp, 1000, "\tallow-update { %s };\n", allow_update);
		    	strcat(new_conf, tmp);
		    }

		    if( domain_type_slave )
		    {
			snprintf(tmp, 1000, "\tmasters { %s };\n", masters);
			strcat(new_conf, tmp);

			if( strlen(allow_notify) > 3 )
			{
			    snprintf(tmp, 1000, "\tallow-notify { %s };\n", allow_notify);
			    strcat(new_conf, tmp);
			}
		    }
		}
		else
		if( strstr(line, "allow-query") && strstr(line, "{") )
		{
		    snprintf(tmp, 1000, "\tallow-query { %s };\n", allow_query);
		    strcat(new_conf, tmp);
		}
		else
		if( strstr(line, "zone-statistics") && strstr(line, ";") )
		{
		    snprintf(tmp, 1000, "\tzone-statistics %s;\n", domain_statistics);
		    strcat(new_conf, tmp);
		}
		else
		if( strstr(line, "notify") && ! strstr(line, "also-notify")
		&& ! strstr(line, "allow-notify") && strstr(line, ";") )
		{
		    snprintf(tmp, 1000, "\tnotify %s;\n", domain_notify);
		    strcat(new_conf, tmp);
		}
		else
		if( strstr(line, "also-notify") && strstr(line, ";") )
		{
		    snprintf(tmp, 1000, "\talso-notify { %s };\n", also_notify);
		    strcat(new_conf, tmp);
		}
		else
		  strcat(new_conf, line);
		    
		if( strstr(line, "zone \"") && ! strstr(line, "forward.zone") )
		  break;
	    }
	}
	else
	  strcat(new_conf, line);


	/* Reverse zone begin (the first zone after the fwd one) */
	if( strstr(line, "zone ") && found_fwd_zone && ! found_rew_zone 
	&& strstr(line, "in-addr.arpa") )
	{
	    /* Dont handle root.hints or the local zone */
	    if( ! strcmp(forward, ".") || ! strcmp(forward, "0.0.127") )
	    {
		strcat(new_conf, line);
	        continue;
	    }
	    
	    while(fgets(line, file_size, fp)!=NULL)
	    if( strstr(line, "type ") && strstr(line, ";") )
	    {
		/* Store position of the "type" line */
		type_pos = ftell(fp);
	    }
	    else  /* The correct reverse zone has been found */
	    if( strstr(line, reverse_file) && strlen(line) < MAX_CONF_LINE-3 )
	    {
		/* We have found the correct reverse zone's "file" line */
	    	found_rew_zone = 1;

		/* Scroll back to the stored "type" position */
		fseek(fp, type_pos, SEEK_SET);
		snprintf(tmp, 1000, "\ttype %s;\n", domain_type);
		strcat(new_conf, tmp);

		/* Skip comments */
		if( strstr(line, "/*") )
		{
		    while(fgets(line, file_size, fp)!=NULL)
		    {
			strcat(new_conf, line);
			if( strstr(line, "*/") )
			  break;
		    }	
		}

		/* Skip comments */
		if( strstr(line, "//") )
		  continue;


		/* Handle the options */
		while(fgets(line, file_size, fp)!=NULL)
		{
		    /* Remove any allow-update lines */
		    if( strstr(line, "allow-update") && strstr(line, "{") )
		      continue;
		    
		    /* Remove any masters lines */
		    if( strstr(line, "masters") && strstr(line, "{") )
		      continue;

		    /* Remove any allow-update lines */
		    if( strstr(line, "allow-notify") && strstr(line, "{") )
		      continue;


		    
		    /* The allow-transfer line must exist, also handles other options */
		    if( strstr(line, "allow-transfer") && strstr(line, "{") )
		    {
		        snprintf(tmp, 1000, "\tallow-transfer { %s };\n", allow_transfer);
		        strcat(new_conf, tmp);

			/* If the domain type is master, we can have allow-update */
			if( ! domain_type_slave )
			{
		    	     snprintf(tmp, 1000, "\tallow-update { %s };\n", allow_update);
		    	     strcat(new_conf, tmp);
			}

			if( domain_type_slave )
			{
		    	    snprintf(tmp, 1000, "\tmasters { %s };\n", masters);
			    strcat(new_conf, tmp);

			    if( strlen(allow_notify) > 3 )
			    {
				snprintf(tmp, 1000, "\tallow-notify { %s };\n", allow_notify);
				strcat(new_conf, tmp);
			    }
			}
		    }
		    else
		    if( strstr(line, "allow-query") && strstr(line, "{") )
		    {
		        snprintf(tmp, 1000, "\tallow-query { %s };\n", allow_query);
		        strcat(new_conf, tmp);
		    }
		    else
		    if( strstr(line, "zone-statistics") && strstr(line, ";") )
		    {
		        snprintf(tmp, 1000, "\tzone-statistics %s;\n", domain_statistics);
		        strcat(new_conf, tmp);
		    }
		    else
		    if( strstr(line, "notify") && ! strstr(line, "also-notify")
		    && ! strstr(line, "allow-notify") && strstr(line, ";") )
		    {
			snprintf(tmp, 1000, "\tnotify %s;\n", domain_notify);
			strcat(new_conf, tmp);
		    }
		    else
		    if( strstr(line, "also-notify") && strstr(line, ";") )
		    {
			snprintf(tmp, 1000, "\talso-notify { %s };\n", also_notify);
			strcat(new_conf, tmp);
		    }
		    else
		      strcat(new_conf, line);
		    
		    if( strstr(line, "zone \"") && ! strstr(line, "zone-statistics") )
		      break;
		}
		break;
	    }
	    else
	      strcat(new_conf, line);
	}
    }
    fclose(fp);

    free(line);
    free(forward);
    free(reverse_file);
    free(tmp);
// FIXME: change named.conf reverse IPv6 zone options too
    /* Write the new named.conf */
    if((fp=fopen(named_conf, "w+"))==NULL)
    {
	info = g_strdup_printf(_("Error: Cannot write named.conf here:\n%s\n"), named_conf);
	utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);
	show_info(info);
	g_free(info);
	if( utf8!=NULL )
    	  g_free(utf8);
	g_free(domain_statistics);
	g_free(domain_type);
	g_free(named_conf);
	return;
    }
    fputs(new_conf, fp);
    fclose(fp);
    free(new_conf);



    /* Domain name changed. Use sed to change domain name in named.conf */
    if( new_domain_name )
    {
	/* sed -e s/drum.se/bbb.se/g filepath > filepath.tmp */
	cmd = g_strdup_printf("%s -e s/%s/%s/g %s > %s.tmp", 
              SED_BINARY, global_domain_name, domain_name, named_conf, named_conf);
	if( ! run_command(cmd) )
	{
	    printf("Error running command:\n%s\n", cmd);
	    if( utf8!=NULL )
    	      g_free(utf8);
	    g_free(domain_statistics);
	    g_free(domain_type);
	    g_free(named_conf);
	    return;    
	}
	g_free(cmd);

	/* mv /path/to/named.conf.tmp path/to/named.conf */
	cmd = g_strdup_printf("mv %s.tmp %s", named_conf, named_conf);
	if( ! run_command(cmd) )
	{
	    printf("Error running command:\n%s\n", cmd);
	    if( utf8!=NULL )
    	      g_free(utf8);
	    g_free(domain_statistics);
	    g_free(domain_type);
	    g_free(named_conf);
	    return;    
	}
	g_free(cmd);
    }

    /* Show some info */
    if( strcmp(global_domain_type, domain_type) )
    {
	if( strstr(domain_type, "master") )
    	  printf("Domain type changed to master.\n");
    	if( strstr(domain_type, "slave") )
          printf("Domain type changed to slave.\n");

	if( domain_type_slave )
	  info = g_strdup_printf(_("Domain type changed to slave.\n"));
	else
	  info = g_strdup_printf(_("Domain type changed to master.\n"));

	utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);
	show_info(info);
	g_free(info);
    }

    if( utf8!=NULL )
      g_free(utf8);
    g_free(domain_statistics);
    g_free(domain_type);
    g_free(named_conf);


    /* Repopulate the gui */
    populate_domain_treeview(widgets);

    populate_resource_treeview(widgets);

    populate_log_tab(widgets);
    
    gtk_widget_destroy(widgets->domain_settings_window);
}
