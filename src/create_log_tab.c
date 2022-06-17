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
#include "populate_log_tab.h"
#include "clear_log_tab.h"


void create_log_tab(struct w *widgets)
{
    /* Create the leases textview in a scrolled window */
    GtkWidget *log_hbox, *log_scrolled_window;
    GtkWidget *log_hbuttonbox, *log_tab_clear_button, *log_tab_update_button;
    
    log_hbox = gtk_hbox_new(TRUE, 0);
    gtk_box_pack_start(GTK_BOX(widgets->notebook_vbox4), log_hbox, TRUE, TRUE, 0);

    log_scrolled_window = gtk_scrolled_window_new (NULL, NULL);
    gtk_box_pack_start(GTK_BOX(log_hbox), log_scrolled_window, TRUE, TRUE, 0);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(log_scrolled_window), 
							    GTK_POLICY_AUTOMATIC, 
							    GTK_POLICY_ALWAYS);
    /* Must set a larger size or it wont scroll */
    gtk_widget_set_size_request(log_scrolled_window, -1, 140);

    widgets->log_textview = gtk_text_view_new();
    gtk_container_add (GTK_CONTAINER (log_scrolled_window), widgets->log_textview);

    /* The leases clear and update buttons */
    log_hbuttonbox = gtk_hbutton_box_new();
    gtk_box_pack_start(GTK_BOX(widgets->notebook_vbox4), log_hbuttonbox, FALSE, TRUE, 0);
    gtk_button_box_set_layout(GTK_BUTTON_BOX(log_hbuttonbox), GTK_BUTTONBOX_SPREAD);

    log_tab_clear_button = gtk_button_new_from_stock(GTK_STOCK_DELETE);
    gtk_container_add(GTK_CONTAINER(log_hbuttonbox), log_tab_clear_button);
    g_signal_connect_swapped(G_OBJECT(log_tab_clear_button), "clicked", 
                             G_CALLBACK(clear_log_tab), widgets);
    
    log_tab_update_button = gtk_button_new_from_stock(GTK_STOCK_REFRESH);
    gtk_container_add(GTK_CONTAINER(log_hbuttonbox), log_tab_update_button);
    g_signal_connect_swapped(G_OBJECT(log_tab_update_button), "clicked", 
                             G_CALLBACK(populate_log_tab), widgets);
}
