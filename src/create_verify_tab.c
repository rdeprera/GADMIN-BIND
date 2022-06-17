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
#include "widgets.h"
#include "gettext.h"
#include "run_verify_cmd.h"



void create_verify_tab(struct w *widgets)
{
    /* Create the verify textview in a scrolled window */
    gchar *utf8=NULL;
    gchar *text;
    GtkWidget *verify_hbox, *verify_scrolled_window;
    GtkWidget *run_cmd_hbox, *run_cmd_button;
    GtkTooltips *tooltips;
    
    tooltips = gtk_tooltips_new();
    
    verify_hbox = gtk_hbox_new(TRUE, 0);
    gtk_box_pack_start(GTK_BOX(widgets->notebook_vbox5), verify_hbox, TRUE, TRUE, 0);

    verify_scrolled_window = gtk_scrolled_window_new (NULL, NULL);
    gtk_box_pack_start(GTK_BOX(verify_hbox), verify_scrolled_window, TRUE, TRUE, 0);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(verify_scrolled_window), 
							    GTK_POLICY_AUTOMATIC, 
							    GTK_POLICY_ALWAYS);
    /* Must set a larger size or it wont scroll */
    gtk_widget_set_size_request(verify_scrolled_window, -1, 140);

    widgets->verify_textview = gtk_text_view_new();
    gtk_container_add(GTK_CONTAINER(verify_scrolled_window), widgets->verify_textview);

    /* The run command entry */
    run_cmd_hbox = gtk_hbox_new(TRUE, 0);
    gtk_box_pack_start(GTK_BOX(widgets->notebook_vbox5), run_cmd_hbox, FALSE, TRUE, 0);
        
    widgets->verify_run_cmd_entry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(run_cmd_hbox), widgets->verify_run_cmd_entry, TRUE, TRUE, 0);

    /* Set default text in the entry */
    text = g_strdup_printf("dig axfr domain.org");
    utf8 = g_locale_to_utf8(text, strlen(text), NULL, NULL, NULL);
    gtk_entry_set_text(GTK_ENTRY(widgets->verify_run_cmd_entry), utf8);
    g_free(text);

    /* Show a tooltip */
    text = g_strdup_printf(_("Forward lookup: host dns1.site.org\nReverse lookup: host (IP of dns1.site.org)\nNetwork info: ifconfig\n"));
    utf8 = g_locale_to_utf8(text, strlen(text), NULL, NULL, NULL);
    gtk_tooltips_set_tip(tooltips, widgets->verify_run_cmd_entry, utf8, NULL);
    g_free(text);
    
    /* The run command button */
    run_cmd_button = gtk_button_new_from_stock(GTK_STOCK_APPLY);
    gtk_box_pack_start(GTK_BOX(run_cmd_hbox), run_cmd_button, FALSE, FALSE, 0);

    g_signal_connect_swapped(G_OBJECT(run_cmd_button), "clicked",
                             G_CALLBACK(run_verify_cmd), widgets);

    g_signal_connect_swapped(G_OBJECT(widgets->verify_run_cmd_entry), "activate",
                                 G_CALLBACK(run_verify_cmd), widgets);

    if( utf8!=NULL )
      g_free(utf8);
}
