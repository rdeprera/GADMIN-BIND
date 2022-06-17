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



void select_first_resource(struct w *widgets)
{
    GtkTreePath *path;
    GtkTreeIter iter;
    GtkTreeModel *list_store;
    gchar *type=NULL, *hostname=NULL, *address=NULL;

    path = gtk_tree_path_new_first();

    list_store = gtk_tree_view_get_model(GTK_TREE_VIEW(widgets->resource_treeview));

    if( ! gtk_tree_model_get_iter(list_store, &iter, path) )
    {
	if( path!=NULL )
          gtk_tree_path_free(path);
        return;
    }
    
    gtk_tree_model_get(list_store, &iter, 0, &type, -1);
    gtk_tree_model_get(list_store, &iter, 1, &hostname, -1);
    gtk_tree_model_get(list_store, &iter, 2, &address, -1);

    if( type==NULL || (hostname==NULL && address==NULL) )
    {
	if( path!=NULL )
          gtk_tree_path_free(path);
        return;
    }

    sprintf(global_resource_type, "%s", (gchar *)type?type:"none");
    sprintf(global_resource_hostname, "%s", (gchar *)hostname?hostname:"none");
    sprintf(global_resource_address, "%s", (gchar *)address?address:"none");

    if( type!=NULL )
      g_free(type);
    if( hostname!=NULL )
      g_free(hostname);
    if( address!=NULL )
      g_free(address);

    if( path!=NULL )
      gtk_tree_path_free(path);
}
