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

extern char global_domain_name[1024];
extern char global_domain_ipv4_address[1024];
extern char global_domain_ipv6_address[1024];
extern char global_domain_type[1024];



void select_first_domain(struct w *widgets)
{
    GtkTreePath *path;
    GtkTreeIter iter;
    GtkTreeModel *list_store;
    gchar *domain=NULL, *ipv4_address=NULL, *ipv6_address=NULL, *type=NULL;

    path = gtk_tree_path_new_first();

    list_store = gtk_tree_view_get_model(GTK_TREE_VIEW(widgets->domain_treeview));

    if( ! gtk_tree_model_get_iter(list_store, &iter, path) )
    {
	if( path!=NULL )
          gtk_tree_path_free(path);
       return;
    }
    
    gtk_tree_model_get(list_store, &iter, 0, &domain, -1);
    gtk_tree_model_get(list_store, &iter, 1, &ipv4_address, -1);
    gtk_tree_model_get(list_store, &iter, 2, &ipv6_address, -1);
    gtk_tree_model_get(list_store, &iter, 3, &type, -1);

    if( domain==NULL || (ipv4_address==NULL && ipv6_address==NULL) || type==NULL )
    {
	if( path!=NULL )
          gtk_tree_path_free(path);
        return;
    }

    sprintf(global_domain_name, "%s", (gchar *)domain?domain:"none");
    sprintf(global_domain_ipv4_address, "%s", (gchar *)ipv4_address?ipv4_address:"none");
    sprintf(global_domain_ipv6_address, "%s", (gchar *)ipv6_address?ipv6_address:"none");
    sprintf(global_domain_type, "%s", (gchar *)type?type:"master");

    if( domain!=NULL )
      g_free(domain);

    if( ipv4_address!=NULL )
      g_free(ipv4_address);

    if( ipv6_address!=NULL )
      g_free(ipv6_address);

    if( type!=NULL )
      g_free(type);

    if( path!=NULL )
      gtk_tree_path_free(path);
}
