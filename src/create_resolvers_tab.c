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
#include "populate_resolvers.h"
#include "save_resolvers.h"


void create_resolvers_tab(struct w *widgets)
{
    /* Create the leases textview in a scrolled window */
    GtkWidget *resolvers_hbox, *resolvers_scrolled_window;
    GtkWidget *resolvers_hbuttonbox, *resolvers_tab_save_button;
    
    resolvers_hbox = gtk_hbox_new(TRUE, 0);
    gtk_box_pack_start(GTK_BOX(widgets->notebook_vbox3), resolvers_hbox, TRUE, TRUE, 0);

    resolvers_scrolled_window = gtk_scrolled_window_new (NULL, NULL);
    gtk_box_pack_start(GTK_BOX(resolvers_hbox), resolvers_scrolled_window, TRUE, TRUE, 0);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(resolvers_scrolled_window), 
							    GTK_POLICY_AUTOMATIC, 
							    GTK_POLICY_ALWAYS);
    /* Must set a larger size or it wont scroll */
    gtk_widget_set_size_request(resolvers_scrolled_window, -1, 140);

    widgets->resolvers_textview = gtk_text_view_new();
    gtk_container_add (GTK_CONTAINER (resolvers_scrolled_window), widgets->resolvers_textview);

    /* The resolvers save and update buttons */
    resolvers_hbuttonbox = gtk_hbutton_box_new();
    gtk_box_pack_start(GTK_BOX(widgets->notebook_vbox3), resolvers_hbuttonbox, FALSE, TRUE, 0);
    gtk_button_box_set_layout(GTK_BUTTON_BOX(resolvers_hbuttonbox), GTK_BUTTONBOX_SPREAD);

    resolvers_tab_save_button = gtk_button_new_from_stock(GTK_STOCK_SAVE);
    gtk_container_add(GTK_CONTAINER(resolvers_hbuttonbox), resolvers_tab_save_button);
    g_signal_connect_swapped(G_OBJECT(resolvers_tab_save_button), "clicked", 
                             G_CALLBACK(save_resolvers), widgets);

    populate_resolvers(widgets);
}
