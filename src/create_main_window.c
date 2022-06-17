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
#include "locate_icons.h"
#include "activate_button_clicked.h"
#include "deactivate_button_clicked.h"
#include "reread_button_clicked.h"
#include "settings_window.h"
#include "show_help.h"
#include "credits_window.h"
#include "set_version.h"
#include "rndc_reload.h"



void create_main_window(struct w *widgets)
{
    gchar *info, *pixmap_directory;
    GtkCellRenderer *pixbuf_cell_renderer;
    GdkPixbuf *pixbuf;
    gchar *utf8=NULL;

    /* Create the main window */    
    widgets->main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_position(GTK_WINDOW (widgets->main_window), GTK_WIN_POS_CENTER);

    /* Only set the vertical size */
    gtk_widget_set_size_request(widgets->main_window, -1, 500);

    /* Set window information */
    info = g_strdup_printf("GADMIN-BIND %s", VERSION);
    utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);
    gtk_window_set_title(GTK_WINDOW(widgets->main_window), utf8);
    g_free(info);

    /* Set the main window icon */    
    pixmap_directory = g_strdup_printf("%s/pixmaps/gadmin-bind", PACKAGE_DATA_DIR);
    add_pixmap_directory(pixmap_directory);
    g_free(pixmap_directory);

    pixbuf_cell_renderer = gtk_cell_renderer_pixbuf_new();    
    pixbuf = create_pixbuf("gadmin-bind.png");
    g_object_set(pixbuf_cell_renderer, "pixbuf", pixbuf, NULL); 
    gtk_window_set_icon(GTK_WINDOW(widgets->main_window), pixbuf);
    gdk_pixbuf_unref(pixbuf);

    widgets->main_vbox = gtk_vbox_new(FALSE, 0);
    gtk_container_add (GTK_CONTAINER (widgets->main_window), widgets->main_vbox);

    /* Hboxes (down) */
    GtkWidget *toolbar_hbox = gtk_hbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(widgets->main_vbox), toolbar_hbox, FALSE, FALSE, 0);

    GtkWidget *status_hbox = gtk_hbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(widgets->main_vbox), status_hbox, FALSE, FALSE, 0);

    GtkWidget *status_hsep_hbox = gtk_hbox_new(TRUE, 0);
    gtk_box_pack_start(GTK_BOX(widgets->main_vbox), status_hsep_hbox, FALSE, TRUE, 0);

    GtkWidget *notebook_hbox = gtk_hbox_new(TRUE, 0);
    gtk_box_pack_start(GTK_BOX(widgets->main_vbox), notebook_hbox, TRUE, TRUE, 0);

    /* Create the main toolbar */
    GtkWidget *main_toolbar = gtk_toolbar_new();
    gtk_box_pack_start(GTK_BOX(toolbar_hbox), main_toolbar, TRUE, TRUE, 0);

    gtk_toolbar_set_style(GTK_TOOLBAR(main_toolbar), GTK_TOOLBAR_BOTH);

    /* Activate button */
    info = g_strdup_printf(_("Activate"));
    utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);
    GtkWidget *toolbar_icon_yes = gtk_image_new_from_stock("gtk-yes", 
                                  gtk_toolbar_get_icon_size(GTK_TOOLBAR(main_toolbar)));
    GtkWidget *activate_button = gtk_toolbar_append_element(GTK_TOOLBAR(main_toolbar),
                                                          GTK_TOOLBAR_CHILD_BUTTON,
							  NULL, utf8, 
							  NULL, NULL,
							  toolbar_icon_yes, NULL, NULL);
    g_free(info);
    g_signal_connect_swapped(G_OBJECT(activate_button), "clicked", 
                             G_CALLBACK(activate_button_clicked), widgets);

    /* Deactivate button */
    info = g_strdup_printf(_("Deactivate"));
    utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);
    GtkWidget *toolbar_icon_no = gtk_image_new_from_stock("gtk-no", 
                                 gtk_toolbar_get_icon_size(GTK_TOOLBAR(main_toolbar)));
    GtkWidget *deactivate_button = gtk_toolbar_append_element(GTK_TOOLBAR(main_toolbar),
                                                          GTK_TOOLBAR_CHILD_BUTTON,
							  NULL, utf8,
							  NULL, NULL,
							  toolbar_icon_no, NULL, NULL);
    g_free(info);
    g_signal_connect_swapped(G_OBJECT(deactivate_button), "clicked", 
                             G_CALLBACK(deactivate_button_clicked), widgets);
    /* Restart button */
    info = g_strdup_printf(_("Restart"));
    utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);
    GtkWidget *toolbar_icon_refresh = gtk_image_new_from_stock("gtk-refresh", 
                                      gtk_toolbar_get_icon_size(GTK_TOOLBAR(main_toolbar)));
    GtkWidget *reread_button = gtk_toolbar_append_element(GTK_TOOLBAR(main_toolbar),
                                                          GTK_TOOLBAR_CHILD_BUTTON,
							  NULL, info,
							  NULL, NULL,
							  toolbar_icon_refresh, NULL, NULL);
    g_free(info);
    g_signal_connect_swapped(G_OBJECT(reread_button), "clicked", 
                             G_CALLBACK(reread_button_clicked), widgets);

    /*  gtk_toolbar_append_space(GTK_TOOLBAR(main_toolbar)); */

    /* Settings button */
    info = g_strdup_printf(_("Settings"));
    utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);
    GtkWidget *toolbar_icon_settings = gtk_image_new_from_stock("gtk-index", 
                                       gtk_toolbar_get_icon_size(GTK_TOOLBAR(main_toolbar)));
    GtkWidget *settings_button = gtk_toolbar_append_element(GTK_TOOLBAR(main_toolbar),
                                                          GTK_TOOLBAR_CHILD_BUTTON,
							  NULL, utf8,
							  NULL, NULL,
							  toolbar_icon_settings, NULL, NULL);
    g_free(info);
    g_signal_connect_swapped(G_OBJECT(settings_button), "clicked", 
                             G_CALLBACK(show_settings_window), widgets);

    /* Reload zones button */
    info = g_strdup_printf(_("Reload zones"));
    utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);
    GtkWidget *toolbar_icon_reload = gtk_image_new_from_stock("gtk-refresh", 
                                     gtk_toolbar_get_icon_size(GTK_TOOLBAR(main_toolbar)));
    GtkWidget *reload_button = gtk_toolbar_append_element(GTK_TOOLBAR(main_toolbar),
                                                          GTK_TOOLBAR_CHILD_BUTTON,
							  NULL, utf8,
							  NULL, NULL,
							  toolbar_icon_reload, NULL, NULL);
    g_free(info);
    g_signal_connect_swapped(G_OBJECT(reload_button), "clicked", 
                             G_CALLBACK(rndc_reload), widgets);

    /* Help button */
    info = g_strdup_printf(_("Help"));
    utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);
    GtkWidget *toolbar_icon_help = gtk_image_new_from_stock("gtk-help", 
                                   gtk_toolbar_get_icon_size(GTK_TOOLBAR(main_toolbar)));
    GtkWidget *help_button = gtk_toolbar_append_element(GTK_TOOLBAR(main_toolbar),
                                                          GTK_TOOLBAR_CHILD_BUTTON,
							  NULL, utf8,
							  NULL, NULL,
							  toolbar_icon_help, NULL, NULL);
    g_free(info);
    g_signal_connect_swapped(G_OBJECT(help_button), "clicked", 
                             G_CALLBACK(show_help), widgets);


    /* Credits button */
    info = g_strdup_printf(_("Credits"));
    utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);
    GtkWidget *toolbar_icon_credits = gtk_image_new_from_stock("gtk-justify-center", 
                                      gtk_toolbar_get_icon_size(GTK_TOOLBAR(main_toolbar)));
    GtkWidget *credits_button = gtk_toolbar_append_element(GTK_TOOLBAR(main_toolbar),
                                                          GTK_TOOLBAR_CHILD_BUTTON,
							  NULL, utf8,
							  NULL, NULL,
							  toolbar_icon_credits, NULL, NULL);
    g_free(info);
    g_signal_connect_swapped(G_OBJECT(credits_button), "clicked", 
                             G_CALLBACK(show_credits), widgets);


    /* Quit button */
    info = g_strdup_printf(_("Quit"));
    utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);
    GtkWidget *toolbar_icon_quit = gtk_image_new_from_stock("gtk-quit", 
                                   gtk_toolbar_get_icon_size(GTK_TOOLBAR(main_toolbar)));
    GtkWidget *quit_button = gtk_toolbar_append_element(GTK_TOOLBAR(main_toolbar),
                                                          GTK_TOOLBAR_CHILD_BUTTON,
							  NULL, utf8,
							  NULL, NULL,
							  toolbar_icon_quit, NULL, NULL);
    g_free(info);
    g_signal_connect_swapped(G_OBJECT(quit_button), "clicked", 
                             G_CALLBACK(gtk_main_quit), NULL);

    /* The notebook */
    GtkWidget *notebook_vbox = gtk_vbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(notebook_hbox), notebook_vbox, TRUE, TRUE, 0);

    widgets->notebook_vbox1 = gtk_vbox_new(FALSE, 0);
    widgets->notebook_vbox2 = gtk_vbox_new(FALSE, 0);
    widgets->notebook_vbox3 = gtk_vbox_new(FALSE, 0);
    widgets->notebook_vbox4 = gtk_vbox_new(FALSE, 0);
    widgets->notebook_vbox5 = gtk_vbox_new(FALSE, 0);

    GtkWidget *main_notebook = gtk_notebook_new();
    gtk_box_pack_start(GTK_BOX(notebook_vbox), main_notebook, TRUE, TRUE, 0);
    gtk_notebook_set_show_border(GTK_NOTEBOOK(main_notebook), TRUE);
    gtk_widget_show(main_notebook);

    info = g_strdup_printf(_("Domains"));
    utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);
    GtkWidget *domain_label = gtk_label_new(utf8);
    g_free(info);
    
    info = g_strdup_printf(_("Root servers"));
    utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);
    GtkWidget *root_servers_label = gtk_label_new(utf8);
    g_free(info);

    info = g_strdup_printf(_("Resolvers"));
    utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);
    GtkWidget *resolvers_label = gtk_label_new(utf8);
    g_free(info);

    info = g_strdup_printf(_("System log"));
    utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);
    GtkWidget *system_log_label = gtk_label_new(utf8);
    g_free(info);

    info = g_strdup_printf(_("Verify domains"));
    utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);
    GtkWidget *verify_domain_label = gtk_label_new(utf8);
    g_free(info);

    gtk_notebook_insert_page(GTK_NOTEBOOK(main_notebook), widgets->notebook_vbox1, domain_label, 0);
    gtk_notebook_insert_page(GTK_NOTEBOOK(main_notebook), widgets->notebook_vbox2, root_servers_label, 1);
    gtk_notebook_insert_page(GTK_NOTEBOOK(main_notebook), widgets->notebook_vbox3, resolvers_label, 2);
    gtk_notebook_insert_page(GTK_NOTEBOOK(main_notebook), widgets->notebook_vbox4, system_log_label, 3);
    gtk_notebook_insert_page(GTK_NOTEBOOK(main_notebook), widgets->notebook_vbox5, verify_domain_label, 4);

    /* Set version and status labels */
    info = g_strdup_printf(_("Information: cant read version"));
    utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);
    widgets->version_label = gtk_label_new(utf8);
    gtk_box_pack_start(GTK_BOX(status_hbox), widgets->version_label, FALSE, FALSE, 0);
    gtk_misc_set_alignment(GTK_MISC(widgets->version_label), 0, 0);
    g_free(info);

    /* Set dhcpd version */
    set_version(widgets);

    GtkWidget *status_spacer_label = gtk_label_new("");
    gtk_box_pack_start(GTK_BOX(status_hbox), status_spacer_label, TRUE, TRUE, 0);
    gtk_misc_set_alignment (GTK_MISC (status_spacer_label), 0, 0);

    info = g_strdup_printf(_("Status: unknown"));
    utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);
    widgets->status_label = gtk_label_new(utf8);
    gtk_box_pack_start(GTK_BOX(status_hbox), widgets->status_label, FALSE, FALSE, 0);
    gtk_misc_set_alignment (GTK_MISC (widgets->status_label), 0, 0);
    g_free(info);

    GtkWidget *status_hseparator = gtk_hseparator_new();
    gtk_box_pack_start(GTK_BOX(status_hsep_hbox), status_hseparator, TRUE, TRUE, 0);
    gtk_widget_set_size_request(status_hseparator, 10, 10);

    if( utf8!=NULL )
      g_free(utf8);
}
