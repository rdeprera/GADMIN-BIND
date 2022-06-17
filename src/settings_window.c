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
#include "commands.h"
#include "make_settings_entries.h"
#include "make_settings_combos.h"
#include "make_settings_buttons.h"
#include "populate_domain_treeview.h"
#include "populate_resource_treeview.h"
#include "populate_log_tab.h"
#include "reread_conf.h"
#include "new_rndc_key.h"
#include "commented.h"
#include "save_settings.h"
#include "settings_window.h"

#define MAX_CONF_LINE 1000

extern char global_domain_name[1024];
extern char global_domain_ipv4_address[1024];
extern char global_domain_ipv6_address[1024];
extern char global_domain_type[1024];

extern char SYSLOG_PATH_BUF[1024];
extern char RESOLV_PATH_BUF[1024];
extern char CHROOT_PATH_BUF[1024];

gchar *get_secret_key();



/* Get the secret key */
gchar *get_secret_key()
{
    FILE *fp;
    long file_size;
    char *line, *key_line, *tmp;
    int x, z, found_key = 0;
    gchar *key=NULL;

    if((fp=fopen(RNDC_CONF, "r"))==NULL)
    {
        key = g_strdup_printf(_("Secret key not found."));
        return key;
    }
    fseek(fp, 0, SEEK_END);
    file_size = ftell(fp);
    rewind(fp);
    
    line = allocate(file_size+1);
    key_line = allocate(1024);
    tmp = allocate(1024);
                                                                                
    while(fgets(line, file_size, fp)!=NULL)
    {
        if( strstr(line, "secret") && strstr(line, "\"") && strstr(line, ";") 
        && ! commented(line) && strlen(line) < 1000 )
	{
	    found_key = 1;

	    /* Get the beginning of the key */
	    for(x=0; x<1000; x++)
	      if( line[x]=='"' )
	        break;

            snprintf(key_line, 1000, "%s", &line[x+1]);
	    
	    /* Get the end of the key */
	    for(z=strlen(key_line)-2; z>0; z--)
	      if( key_line[z]=='"' )
	        break;

            key_line[z]='\0';
	    key = g_strdup_printf("%s", key_line);
	    
	    break;
	}
    }
    fclose(fp);
    free(line);
    free(key_line);
    free(tmp);
                                                                                                                                                                        
    if( ! found_key )
      key = g_strdup_printf(_("Secret key not found."));
    
    return key;
}



void show_settings_window(struct w *widgets)
{
    GtkWidget *frame;
    GtkWidget *table;
    GtkWidget *settings_vbox;
    GtkTooltips *tooltips;
    gchar *secret_key, *info, *utf8=NULL;

    widgets->settings_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_position(GTK_WINDOW (widgets->settings_window), GTK_WIN_POS_CENTER);
    gtk_widget_set_size_request(widgets->settings_window, -1, -1);

    /* Set window information */
    info = g_strdup_printf(_("GAdmin-BIND %s settings"), VERSION);
    utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);
    gtk_window_set_title(GTK_WINDOW(widgets->settings_window), utf8);
    g_free(info);

    g_signal_connect(GTK_WINDOW(widgets->settings_window), "delete_event", 
		     G_CALLBACK (gtk_widget_destroy), NULL);

    settings_vbox = gtk_vbox_new(FALSE, 0);
    gtk_container_add (GTK_CONTAINER (widgets->settings_window), settings_vbox);


    tooltips = gtk_tooltips_new();

    info = g_strdup_printf(_("Settings:"));
    utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);
    frame = gtk_frame_new(utf8);
    g_free(info);

    /* A table with 6 settings and 2 columns */
    table = gtk_table_new(6, 2, FALSE);

    gtk_box_pack_start(GTK_BOX(settings_vbox), frame, TRUE, TRUE, 1);
    gtk_container_add(GTK_CONTAINER(frame), table);


    /* Max length and input 350 chars */

    /* Path to the chroot */
    info = g_strdup_printf(_(" Path to the chroot directory: "));
    utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);
    widgets->settings_entry[0] = make_entry_with_label(GTK_TABLE(table), utf8, 0,1,2,3,350);
    g_free(info);

    utf8 = g_locale_to_utf8(CHROOT_PATH_BUF, strlen(CHROOT_PATH_BUF), NULL, NULL, NULL);
    gtk_entry_set_text(GTK_ENTRY(widgets->settings_entry[0]), utf8);

    /* Path to resolv.conf */
    info = g_strdup_printf(_(" Path to resolv.conf: "));
    utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);
    widgets->settings_entry[1] = make_entry_with_label(GTK_TABLE(table), utf8, 0,1,3,4,350);
    g_free(info);

    utf8 = g_locale_to_utf8(RESOLV_PATH_BUF, strlen(RESOLV_PATH_BUF), NULL, NULL, NULL);
    gtk_entry_set_text(GTK_ENTRY(widgets->settings_entry[1]), utf8);

    /* Path to syslog */
    info = g_strdup_printf(_(" Path to the system log: "));
    utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);
    widgets->settings_entry[2] = make_entry_with_label(GTK_TABLE(table), utf8, 0,1,4,5,350);
    g_free(info);

    utf8 = g_locale_to_utf8(SYSLOG_PATH_BUF, strlen(SYSLOG_PATH_BUF), NULL, NULL, NULL);
    gtk_entry_set_text(GTK_ENTRY(widgets->settings_entry[2]), utf8);

    /* Button and entry for generating the rndc key */
    widgets->settings_entry[3] = gtk_entry_new();

    /* Insert secret key if any, otherwise insert: key not found */
    secret_key = get_secret_key();        
    utf8 = g_locale_to_utf8(secret_key, strlen(secret_key), NULL, NULL, NULL);
    gtk_entry_set_text(GTK_ENTRY(widgets->settings_entry[3]), utf8);
    g_free(secret_key);

    GtkWidget *gen_key_button = make_gen_key_button_with_entry(GTK_TABLE(table), widgets->settings_entry[3], 0,1,5,6,350);
    g_signal_connect_swapped(G_OBJECT(gen_key_button), "clicked",
                             G_CALLBACK(new_rndc_key), widgets);



    GtkWidget *settings_hbuttonbox = gtk_hbutton_box_new();
    gtk_box_pack_start(GTK_BOX(settings_vbox), settings_hbuttonbox, FALSE, FALSE, 0);
    gtk_button_box_set_layout(GTK_BUTTON_BOX(settings_hbuttonbox), GTK_BUTTONBOX_SPREAD);

    GtkWidget *apply_settings_button = gtk_button_new_from_stock(GTK_STOCK_APPLY);
    gtk_container_add(GTK_CONTAINER(settings_hbuttonbox), apply_settings_button);
    g_signal_connect_swapped(G_OBJECT(apply_settings_button), "clicked", 
                             G_CALLBACK(apply_settings_button_clicked), widgets);
    
    GtkWidget *cancel_settings_button = gtk_button_new_from_stock(GTK_STOCK_CANCEL);
    gtk_container_add(GTK_CONTAINER(settings_hbuttonbox), cancel_settings_button);
    g_signal_connect_swapped(G_OBJECT(cancel_settings_button), "clicked", 
                             G_CALLBACK(cancel_settings_button_clicked), widgets);

    if( utf8!=NULL )
      g_free(utf8);

    gtk_widget_show_all(widgets->settings_window);
}



void cancel_settings_button_clicked(struct w *widgets)
{
    gtk_widget_destroy(widgets->settings_window);
}



void apply_settings_button_clicked(struct w *widgets)
{
    /* The apply button in the settings window */

    save_settings(widgets);

    /* Repopulate the gui */
    populate_domain_treeview(widgets);

    populate_resource_treeview(widgets);

    populate_log_tab(widgets);
    
    gtk_widget_destroy(widgets->settings_window);
}
