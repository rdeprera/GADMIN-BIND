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
#include "domain_treeview_row_clicked.h"
#include "add_domain.h"
#include "delete_domain_from_btn.h"
#include "resource_treeview_row_clicked.h"
#include "add_resource.h"
#include "delete_resource_from_btn.h"
#include "resource_combo_changed.h"
#include "domain_settings_window.h"

/* Wether or not to let the domain section expand */
#define EXPAND_DOMAIN_SECTION FALSE

/* Wether or not to let the resource section expand */
#define EXPAND_RESOURCE_SECTION TRUE



void create_domain_tab(struct w *widgets)
{
    GtkCellRenderer *domain_cell_renderer, *resource_cell_renderer;
    GtkWidget *domain_treeview_hbox, *resource_treeview_hbox;
    GtkWidget *domain_scrolled_window, *resource_scrolled_window;

    GtkTooltips *tooltips;
    tooltips = gtk_tooltips_new();
    
    gchar *utf8=NULL;
    gchar *text;

    /* Create the domain treeview in a scrolled window */
    domain_treeview_hbox = gtk_hbox_new(TRUE, 0);
    gtk_box_pack_start(GTK_BOX(widgets->notebook_vbox1), domain_treeview_hbox, EXPAND_DOMAIN_SECTION, TRUE, 0);

    domain_scrolled_window = gtk_scrolled_window_new (NULL, NULL);
    gtk_box_pack_start(GTK_BOX(domain_treeview_hbox), domain_scrolled_window, TRUE, TRUE, 0);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(domain_scrolled_window), 
						    GTK_POLICY_AUTOMATIC, 
						    GTK_POLICY_ALWAYS);
    /* Must set a larger size or it wont scroll */
    gtk_widget_set_size_request(domain_scrolled_window, -1, 100);

    widgets->domain_store = gtk_list_store_new (4, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
// was 3

    widgets->domain_treeview = gtk_tree_view_new();
    gtk_tree_view_set_model(GTK_TREE_VIEW(widgets->domain_treeview), GTK_TREE_MODEL(widgets->domain_store));

    gtk_container_add(GTK_CONTAINER(domain_scrolled_window), widgets->domain_treeview);
    gtk_tree_view_set_rules_hint(GTK_TREE_VIEW(widgets->domain_treeview), TRUE);

    /* Set the column labels in the treeview */
    domain_cell_renderer = gtk_cell_renderer_text_new();

    text = g_strdup_printf(_("Domain name"));
    utf8 = g_locale_to_utf8(text, strlen(text), NULL, NULL, NULL);
    GtkTreeViewColumn *domain_name_col = gtk_tree_view_column_new_with_attributes(utf8, domain_cell_renderer, "text", 0, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(widgets->domain_treeview), GTK_TREE_VIEW_COLUMN(domain_name_col));
    g_free(text);
    
    text = g_strdup_printf(_("IPv4 Network address"));
    utf8 = g_locale_to_utf8(text, strlen(text), NULL, NULL, NULL);
    GtkTreeViewColumn *network_ipv4_address_col = gtk_tree_view_column_new_with_attributes(utf8, domain_cell_renderer, "text", 1, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(widgets->domain_treeview), GTK_TREE_VIEW_COLUMN(network_ipv4_address_col));
    g_free(text);

    text = g_strdup_printf(_("IPv6 Network address"));
    utf8 = g_locale_to_utf8(text, strlen(text), NULL, NULL, NULL);
    GtkTreeViewColumn *network_ipv6_address_col = gtk_tree_view_column_new_with_attributes(utf8, domain_cell_renderer, "text", 2, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(widgets->domain_treeview), GTK_TREE_VIEW_COLUMN(network_ipv6_address_col));
    g_free(text);

    text = g_strdup_printf(_("DNS type"));
    utf8 = g_locale_to_utf8(text, strlen(text), NULL, NULL, NULL);
    GtkTreeViewColumn *dns_type_col = gtk_tree_view_column_new_with_attributes(utf8, domain_cell_renderer, "text", 3, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(widgets->domain_treeview), GTK_TREE_VIEW_COLUMN(dns_type_col));
    g_free(text);

    g_signal_connect((gpointer)widgets->domain_treeview, "button_press_event",  
		      G_CALLBACK(domain_treeview_row_clicked), widgets);




    /* The add/delete domain entries and buttons */
    GtkWidget *domain_add_hbox1 = gtk_hbox_new(FALSE,FALSE);
    gtk_box_pack_start(GTK_BOX(widgets->notebook_vbox1), domain_add_hbox1, FALSE, FALSE, 0);

    GtkWidget *domain_add_hbox2 = gtk_hbox_new(FALSE,FALSE);
    gtk_box_pack_start(GTK_BOX(widgets->notebook_vbox1), domain_add_hbox2, FALSE, FALSE, 0);


    widgets->add_domain_name_entry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(domain_add_hbox2), widgets->add_domain_name_entry, TRUE, TRUE, 0);
    text = g_strdup_printf(_("The domain to add, IE: example.org"));
    utf8 = g_locale_to_utf8(text, strlen(text), NULL, NULL, NULL);
    gtk_tooltips_set_tip(tooltips, widgets->add_domain_name_entry, utf8, NULL);
    g_free(text);

    widgets->add_domain_ipv4_address_entry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(domain_add_hbox2), widgets->add_domain_ipv4_address_entry, TRUE, TRUE, 0);
    text = g_strconcat(_("An IPv4 network address for the domain.\nIE: 192.168.0 or 192.168.0.16-42\n"),
    _("If the last method is used another domain must handle this domains reverse lookups.\n"),
    _("In that case a second DNS (NS-record) should be added that points to your ISP's server."),
    NULL);
    utf8 = g_locale_to_utf8(text, strlen(text), NULL, NULL, NULL);
    gtk_tooltips_set_tip(tooltips, widgets->add_domain_ipv4_address_entry, utf8, NULL);
    g_free(text);

    widgets->add_domain_ipv6_address_entry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(domain_add_hbox2), widgets->add_domain_ipv6_address_entry, TRUE, TRUE, 0);
    text = g_strconcat("An IPv6 network address, IE: fe80::213:8fff:feea:9b79/64\n",
    _("Fill in this field and an existing (IPv4 only) domain name in the domain field to add an IPv6 range to it."),
    NULL);
    utf8 = g_locale_to_utf8(text, strlen(text), NULL, NULL, NULL);
    gtk_tooltips_set_tip(tooltips, widgets->add_domain_ipv6_address_entry, utf8, NULL);
    g_free(text);

    /* Domain type combo */
    widgets->add_domain_type_combo = gtk_combo_box_new_text();
    gtk_box_pack_start(GTK_BOX(domain_add_hbox2), widgets->add_domain_type_combo, TRUE, TRUE, 0);

    text = g_strdup_printf(_("master"));
    utf8 = g_locale_to_utf8(text, strlen(text), NULL, NULL, NULL);
    gtk_combo_box_append_text(GTK_COMBO_BOX(widgets->add_domain_type_combo), utf8);
    g_free(text);

    text = g_strdup_printf(_("slave"));
    utf8 = g_locale_to_utf8(text, strlen(text), NULL, NULL, NULL);
    gtk_combo_box_append_text(GTK_COMBO_BOX(widgets->add_domain_type_combo), utf8);
    /* Set this combo position as selected (Master) */
    gtk_combo_box_set_active(GTK_COMBO_BOX(widgets->add_domain_type_combo), 0);
    g_free(text);


    /* Add, del and change Buttonbox */
    GtkWidget *domain_hbuttonbox = gtk_hbutton_box_new();
    gtk_box_pack_start(GTK_BOX(widgets->notebook_vbox1), domain_hbuttonbox, FALSE, FALSE, 2);

    /* The delete domain button */
    GtkWidget *domain_del_button = gtk_button_new_from_stock(GTK_STOCK_DELETE);
    gtk_container_add(GTK_CONTAINER(domain_hbuttonbox), domain_del_button);
    g_signal_connect_swapped(G_OBJECT(domain_del_button), "clicked", 
                             G_CALLBACK(delete_domain_from_btn), widgets);
    text = g_strdup_printf(_("Delete the selected domain."));
    utf8 = g_locale_to_utf8(text, strlen(text), NULL, NULL, NULL);
    gtk_tooltips_set_tip(tooltips, domain_del_button, utf8, NULL);
    g_free(text);

    /* The add domain button */
    GtkWidget *domain_add_button = gtk_button_new_from_stock(GTK_STOCK_ADD);
    gtk_container_add(GTK_CONTAINER(domain_hbuttonbox), domain_add_button);
    g_signal_connect_swapped(G_OBJECT(domain_add_button), "clicked", 
                             G_CALLBACK(add_domain), widgets);


    /* The change domain button with custom text */
    GtkWidget *domain_change_button = gtk_button_new();
    gtk_container_add(GTK_CONTAINER(domain_hbuttonbox), domain_change_button);
    text = g_strdup_printf(_("Make changes to the selected domain."));
    utf8 = g_locale_to_utf8(text, strlen(text), NULL, NULL, NULL);
    gtk_tooltips_set_tip(tooltips, domain_change_button, utf8, NULL);
    g_free(text);
    GtkWidget *alignment60 = gtk_alignment_new(0.5, 0.5, 0, 0);
    gtk_container_add(GTK_CONTAINER(domain_change_button), alignment60);
    GtkWidget *hbox73 = gtk_hbox_new(FALSE, 2);
    gtk_container_add(GTK_CONTAINER(alignment60), hbox73);
    GtkWidget *image40 = gtk_image_new_from_stock("gtk-refresh", GTK_ICON_SIZE_BUTTON);
    gtk_box_pack_start(GTK_BOX(hbox73), image40, FALSE, FALSE, 0);
//    text = g_strdup_printf(_("Change domain settings"));
    text = g_strdup_printf(_("Change"));
    utf8 = g_locale_to_utf8(text, strlen(text), NULL, NULL, NULL);
    GtkWidget *label115 = gtk_label_new_with_mnemonic(utf8);
    g_free(text);
    gtk_box_pack_start(GTK_BOX(hbox73), label115, FALSE, FALSE, 0);
    gtk_label_set_justify(GTK_LABEL(label115), GTK_JUSTIFY_LEFT);
    g_signal_connect_swapped(G_OBJECT(domain_change_button), "clicked", 
                             G_CALLBACK(show_domain_settings_window), widgets);


    /* Some space between the 2 treeviews */
    GtkWidget *views_spacer_hbox = gtk_hbox_new(TRUE, 0);
    gtk_box_pack_start(GTK_BOX(widgets->notebook_vbox1), views_spacer_hbox, FALSE, FALSE, 0);





    /* Domain resources */

//    text = g_strdup_printf(_("Domain resources:"));
//    utf8 = g_locale_to_utf8(text, strlen(text), NULL, NULL, NULL);
//    GtkWidget *resource_label = gtk_label_new(utf8);
//    gtk_box_pack_start(GTK_BOX(widgets->notebook_vbox1), resource_label, FALSE, FALSE, 10);
//    g_free(text);

    /* Create the resource treeview in a scrolled window */
    resource_treeview_hbox = gtk_hbox_new(TRUE, 0);
    gtk_box_pack_start(GTK_BOX(widgets->notebook_vbox1), resource_treeview_hbox, EXPAND_RESOURCE_SECTION, TRUE, 0);

    resource_scrolled_window = gtk_scrolled_window_new (NULL, NULL);
    gtk_box_pack_start(GTK_BOX(resource_treeview_hbox), resource_scrolled_window, TRUE, TRUE, 0);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(resource_scrolled_window), 
						    GTK_POLICY_AUTOMATIC, 
						    GTK_POLICY_ALWAYS);
    /* Must set a larger size or it wont scroll */
    gtk_widget_set_size_request(resource_scrolled_window, -1, 100);

    widgets->resource_store = gtk_list_store_new (3, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);

    widgets->resource_treeview = gtk_tree_view_new();
    gtk_tree_view_set_model(GTK_TREE_VIEW(widgets->resource_treeview), GTK_TREE_MODEL(widgets->resource_store));

    gtk_container_add(GTK_CONTAINER(resource_scrolled_window), widgets->resource_treeview);
    gtk_tree_view_set_rules_hint(GTK_TREE_VIEW(widgets->resource_treeview), TRUE);

    /* Set the column labels in the treeview */
    resource_cell_renderer = gtk_cell_renderer_text_new();

    text = g_strdup_printf(_("Resource type"));
    utf8 = g_locale_to_utf8(text, strlen(text), NULL, NULL, NULL);
    GtkTreeViewColumn *resource_name_col = gtk_tree_view_column_new_with_attributes(utf8, resource_cell_renderer, "text", 0, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(widgets->resource_treeview), GTK_TREE_VIEW_COLUMN(resource_name_col));
    g_free(text);

    text = g_strdup_printf(_("Full Hostname"));
    utf8 = g_locale_to_utf8(text, strlen(text), NULL, NULL, NULL);
    GtkTreeViewColumn *resource_hostname_col = gtk_tree_view_column_new_with_attributes(utf8, resource_cell_renderer, "text", 1, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(widgets->resource_treeview), GTK_TREE_VIEW_COLUMN(resource_hostname_col));
    g_free(text);

    text = g_strdup_printf(_("IP-address, mail server priority or CNAME host alias"));
    utf8 = g_locale_to_utf8(text, strlen(text), NULL, NULL, NULL);
    GtkTreeViewColumn *resource_ipaddress_col = gtk_tree_view_column_new_with_attributes(utf8, resource_cell_renderer, "text", 2, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(widgets->resource_treeview), GTK_TREE_VIEW_COLUMN(resource_ipaddress_col));
    g_free(text);

    g_signal_connect((gpointer)widgets->resource_treeview, "button_press_event",  
		      G_CALLBACK(resource_treeview_row_clicked), widgets);


    /* The add/delete resource entries */
    GtkWidget *resource_add_hbox1 = gtk_hbox_new(FALSE,FALSE);
    gtk_box_pack_start(GTK_BOX(widgets->notebook_vbox1), resource_add_hbox1, FALSE, FALSE, 0);

    GtkWidget *resource_add_hbox2 = gtk_hbox_new(FALSE,FALSE);
    gtk_box_pack_start(GTK_BOX(widgets->notebook_vbox1), resource_add_hbox2, FALSE, FALSE, 0);

    GtkWidget *resource_del_hbox = gtk_hbox_new(FALSE,FALSE);
    gtk_box_pack_start(GTK_BOX(widgets->notebook_vbox1), resource_del_hbox, FALSE, FALSE, 0);

    /* The resource combo box and selections */
    widgets->add_resource_type_combo = gtk_combo_box_new_text();
    gtk_box_pack_start(GTK_BOX(resource_add_hbox2), widgets->add_resource_type_combo, FALSE, FALSE, 0);

    text = g_strdup_printf(_("Nameserver (NS-Record)"));
    utf8 = g_locale_to_utf8(text, strlen(text), NULL, NULL, NULL);
    gtk_combo_box_append_text(GTK_COMBO_BOX(widgets->add_resource_type_combo), utf8);
    g_free(text);

    text = g_strdup_printf(_("Mailserver (MX)"));
    utf8 = g_locale_to_utf8(text, strlen(text), NULL, NULL, NULL);
    gtk_combo_box_append_text(GTK_COMBO_BOX(widgets->add_resource_type_combo), utf8);
    g_free(text);

    text = g_strdup_printf(_("Authoritive (IPv4 A-Record)"));
    utf8 = g_locale_to_utf8(text, strlen(text), NULL, NULL, NULL);
    gtk_combo_box_append_text(GTK_COMBO_BOX(widgets->add_resource_type_combo), utf8);
    g_free(text);

    text = g_strdup_printf(_("Authoritive (IPv6 AAAA-Record)"));
    utf8 = g_locale_to_utf8(text, strlen(text), NULL, NULL, NULL);
    gtk_combo_box_append_text(GTK_COMBO_BOX(widgets->add_resource_type_combo), utf8);
    g_free(text);

    text = g_strdup_printf(_("Machine alias (CNAME)"));
    utf8 = g_locale_to_utf8(text, strlen(text), NULL, NULL, NULL);
    gtk_combo_box_append_text(GTK_COMBO_BOX(widgets->add_resource_type_combo), utf8);
    g_free(text);

    /* Set this combo position as selected */
    gtk_combo_box_set_active(GTK_COMBO_BOX(widgets->add_resource_type_combo), 0);


    /* Set different tooltips for different selections */
    g_signal_connect_swapped(G_OBJECT(widgets->add_resource_type_combo), "changed", 
                             G_CALLBACK(resource_combo_changed), widgets);


    /* Create the tooltips for the resource combo */
    widgets->resource_hostname_tooltip = gtk_tooltips_new();
    widgets->resource_address_tooltip  = gtk_tooltips_new();

    widgets->add_resource_hostname_entry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(resource_add_hbox2), widgets->add_resource_hostname_entry, TRUE, TRUE, 0);
    text = g_strdup_printf(_("A DNS server: dns1.example.org"));
    utf8 = g_locale_to_utf8(text, strlen(text), NULL, NULL, NULL);
    gtk_tooltips_set_tip(widgets->resource_hostname_tooltip, widgets->add_resource_hostname_entry, utf8, NULL);
    g_free(text);

    widgets->add_resource_address_entry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(resource_add_hbox2), widgets->add_resource_address_entry, TRUE, TRUE, 0);
    text = g_strdup_printf(_("192.168.0.1 or fe80::213:8fff:feea:9b79/64"));
    utf8 = g_locale_to_utf8(text, strlen(text), NULL, NULL, NULL);
    gtk_tooltips_set_tip(widgets->resource_address_tooltip, widgets->add_resource_address_entry, utf8, NULL);
    g_free(text);



    /* Add, del and change resource Buttonbox */
    GtkWidget *resource_hbuttonbox = gtk_hbutton_box_new();
    gtk_box_pack_start(GTK_BOX(widgets->notebook_vbox1), resource_hbuttonbox, FALSE, FALSE, 2);

    /* The delete resource button */
    GtkWidget *resource_del_button = gtk_button_new_from_stock(GTK_STOCK_DELETE);
    gtk_container_add(GTK_CONTAINER(resource_hbuttonbox), resource_del_button);
    text = g_strdup_printf(_("Delete the selected resource."));
    utf8 = g_locale_to_utf8(text, strlen(text), NULL, NULL, NULL);
    gtk_tooltips_set_tip(tooltips, resource_del_button, utf8, NULL);
    g_free(text);
    g_signal_connect_swapped(G_OBJECT(resource_del_button), "clicked", 
                             G_CALLBACK(delete_resource_from_btn), widgets);

    /* The add resource button */
    GtkWidget *resource_add_button = gtk_button_new_from_stock(GTK_STOCK_ADD);
    gtk_container_add(GTK_CONTAINER(resource_hbuttonbox), resource_add_button);
    g_signal_connect_swapped(G_OBJECT(resource_add_button), "clicked", 
                             G_CALLBACK(add_resource), widgets);

    /* The change resource button with custom text */
    GtkWidget *resource_change_button = gtk_button_new();
    gtk_container_add(GTK_CONTAINER(resource_hbuttonbox), resource_change_button);
    text = g_strdup_printf(_("Make changes to the selected resource."));
    utf8 = g_locale_to_utf8(text, strlen(text), NULL, NULL, NULL);
    gtk_tooltips_set_tip(tooltips, resource_change_button, utf8, NULL);
    g_free(text);
    GtkWidget *alignment80 = gtk_alignment_new(0.5, 0.5, 0, 0);
    gtk_container_add(GTK_CONTAINER(resource_change_button), alignment80);
    GtkWidget *hbox74 = gtk_hbox_new(FALSE, 2);
    gtk_container_add(GTK_CONTAINER(alignment80), hbox74);
    GtkWidget *image42 = gtk_image_new_from_stock("gtk-refresh", GTK_ICON_SIZE_BUTTON);
    gtk_box_pack_start(GTK_BOX(hbox74), image42, FALSE, FALSE, 0);
//    text = g_strdup_printf(_("Change resource settings"));
    text = g_strdup_printf(_("Change"));
    utf8 = g_locale_to_utf8(text, strlen(text), NULL, NULL, NULL);
    GtkWidget *label124 = gtk_label_new_with_mnemonic(utf8);
    g_free(text);
    gtk_box_pack_start(GTK_BOX(hbox74), label124, FALSE, FALSE, 0);
    gtk_label_set_justify(GTK_LABEL(label124), GTK_JUSTIFY_LEFT);
//    g_signal_connect_swapped(G_OBJECT(resource_change_button), "clicked", 
//                             G_CALLBACK(show_resource_settings_window), widgets);
// FIXME ^

    if( utf8 != NULL )
      g_free(utf8);
}
