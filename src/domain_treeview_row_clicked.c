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
#include "populate_resource_treeview.h"

extern char global_domain_name[1024];
extern char global_domain_ipv4_address[1024];
extern char global_domain_ipv6_address[1024];
extern char global_domain_type[1024];


void domain_treeview_row_clicked(GtkTreeView *treeview, GdkEventButton *event,
								gpointer data)
{
    gchar *domain, *ipv4_address, *ipv6_address, *type;
    GtkTreeIter iter;
    GtkTreePath *path;
    GtkTreeModel *list_store;
    GtkTreeSelection *selection;

    /* Only handle button press event's and left button clicks */
    if( ! event->type == GDK_BUTTON_PRESS || ! event->button == 1 )
      return;

    selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));

    if( gtk_tree_selection_count_selected_rows(selection) <= 1 )
    {
	/* Get the treepath for the row that was clicked */
         if( gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(treeview), event->x, event->y,
				                                &path, NULL, NULL, NULL))
         {
            gtk_tree_selection_unselect_all(selection);
    	    gtk_tree_selection_select_path(selection, path);

	    list_store = gtk_tree_view_get_model(GTK_TREE_VIEW(treeview));
	    gtk_tree_model_get_iter(list_store, &iter, path);
	    gtk_tree_model_get(list_store, &iter, 0, &domain, -1);
	    gtk_tree_model_get(list_store, &iter, 1, &ipv4_address, -1);
	    gtk_tree_model_get(list_store, &iter, 2, &ipv6_address, -1);
	    gtk_tree_model_get(list_store, &iter, 3, &type, -1);

	    sprintf(global_domain_name, "%s", domain?domain:"none");
	    sprintf(global_domain_ipv4_address, "%s", ipv4_address?ipv4_address:"none");
	    sprintf(global_domain_ipv6_address, "%s", ipv6_address?ipv6_address:"none");
	    sprintf(global_domain_type, "%s", type?type:"master");


	    g_free(domain);
	    g_free(ipv4_address);
	    g_free(ipv6_address);
	    g_free(type);
            gtk_tree_path_free(path);

	    populate_resource_treeview(data);
    	 }
    }
}
