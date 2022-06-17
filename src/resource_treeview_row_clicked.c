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

extern char global_resource_type[1024];
extern char global_resource_hostname[1024];
extern char global_resource_address[1024];



void resource_treeview_row_clicked(GtkTreeView *treeview, GdkEventButton *event,
								gpointer data)
{
    gchar *type, *hostname, *address;
    GtkTreeIter iter;
    GtkTreePath *path;
    GtkTreeModel *list_store;
    GtkTreeSelection *selection;

    /* The left button is pressed */
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
	    gtk_tree_model_get(list_store, &iter, 0, &type, -1);
	    gtk_tree_model_get(list_store, &iter, 1, &hostname, -1);
	    gtk_tree_model_get(list_store, &iter, 2, &address, -1);

	    sprintf(global_resource_type, "%s", type?type:"none");
	    sprintf(global_resource_hostname, "%s", hostname?hostname:"none");
	    sprintf(global_resource_address, "%s", address?address:"none");

	    g_free(type);
	    g_free(hostname);
	    g_free(address);
            gtk_tree_path_free(path);
    	 }
    }
}
