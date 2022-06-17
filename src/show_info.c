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

extern int info_window_exit_main;


void info_set_exit(gchar *content)
{
    /* 2 means break the non-root wait loop in main() */
    info_window_exit_main = 2;
}



void show_info(gchar *content)
{
    gchar *utf8=NULL;
    GtkTextBuffer *text_buffer;
    gchar *info;
    GtkWidget *info_window, *info_vbox, *info_textview;
    GtkWidget *info_viewport, *scrolled_info_window;

    if( content == NULL )
    {
       printf("show_info recieved null content\n");
       return;
    }
    
    if( strlen(content) < 5 )
    {
       printf("show_info recieved < 5 chars of content\n");
       return;
    }

    info_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_widget_set_size_request(info_window, 630, 350);
    gtk_window_set_position(GTK_WINDOW (info_window), GTK_WIN_POS_CENTER);
    
    /* Set window information */
    info = g_strdup_printf(_("GAdmin-BIND %s information"), VERSION);
    gtk_window_set_title(GTK_WINDOW(info_window), info);
    g_free(info);

    info_vbox = gtk_vbox_new(FALSE, 0);
    gtk_container_add (GTK_CONTAINER (info_window), info_vbox);

    /* Put a scrolled window in the vbox */
    scrolled_info_window = gtk_scrolled_window_new (NULL, NULL);
    gtk_box_pack_start(GTK_BOX(info_vbox), scrolled_info_window, TRUE, TRUE, 0);

    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_info_window), 
				      GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);

    info_viewport = gtk_viewport_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scrolled_info_window), info_viewport);

    info_textview = gtk_text_view_new();
    gtk_container_add(GTK_CONTAINER(info_viewport), info_textview);

    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(info_textview), GTK_WRAP_WORD);


    GtkWidget *close_info_button = gtk_button_new_from_stock(GTK_STOCK_CLOSE);
    gtk_box_pack_start(GTK_BOX(info_vbox), close_info_button, FALSE, TRUE, 0);


    /* Used with main() and closing the non-root timer */
    if( info_window_exit_main )
    {
	g_signal_connect_swapped((gpointer) close_info_button, "clicked",
                         G_CALLBACK(info_set_exit), info_window);

	g_signal_connect(GTK_WINDOW(info_window), "delete_event", 
                         GTK_SIGNAL_FUNC(info_set_exit), info_window);
    }

    /* Regular close info window hookups */
    if( ! info_window_exit_main )
    {
	g_signal_connect_swapped((gpointer) close_info_button, "clicked",
                              G_CALLBACK(gtk_widget_destroy), GTK_OBJECT(info_window));

	g_signal_connect(GTK_WINDOW(info_window), "delete_event", 
		     G_CALLBACK(gtk_widget_destroy), NULL);
    }


    text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(info_textview));
    utf8 = g_locale_to_utf8(content, strlen(content), NULL, NULL, NULL);
    if( utf8!=NULL )
    {
        gtk_text_buffer_set_text(text_buffer, utf8, strlen(utf8));
        g_free(utf8);
    }
    gtk_widget_show_all(info_window);
}
