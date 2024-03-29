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
#include "gettext.h"
#include <stdio.h>
#include <string.h>
#include "locate_icons.h"


/* Pixmap locations */
static GList *pixmaps_directories = NULL;

static gchar* find_pixmap_file(const gchar *filename);


/* Use this function to set the directory containing installed pixmaps. */
void add_pixmap_directory(const gchar *directory)
{
    pixmaps_directories = g_list_prepend (pixmaps_directories,
                                          g_strdup (directory));
}


/* This is an internally used function to find pixmap files. */
static gchar* find_pixmap_file(const gchar *filename)
{
    GList *elem;

    /* We step through each of the pixmaps directory to find it. */
    elem = pixmaps_directories;
    while( elem )
    {
        gchar *pathname = g_strdup_printf ("%s%s%s", (gchar*)elem->data,
                                         G_DIR_SEPARATOR_S, filename);
        if( g_file_test(pathname, G_FILE_TEST_EXISTS) )
    	    return pathname;
        g_free (pathname);
        elem = elem->next;
    }
    return NULL;
}


/* This is an internally used function to create pixmaps. */
GtkWidget* create_pixmap(GtkWidget *widget, const gchar *filename)
{
    gchar *pathname = NULL;
    GtkWidget *pixmap;

    if (!filename || !filename[0])
        return gtk_image_new ();

    pathname = find_pixmap_file (filename);

    if( ! pathname )
    {
        printf("Couldnt find pixmap file: %s\n", filename);
        printf("In path: %s\n", pathname);
        return gtk_image_new ();
    }

    pixmap = gtk_image_new_from_file (pathname);
    g_free (pathname);

    return pixmap;
}


/* This is an internally used function to create pixmaps. */
GdkPixbuf* create_pixbuf(const gchar *filename)
{
    gchar *pathname = NULL;
    GdkPixbuf *pixbuf;
    GError *error = NULL;

    if( ! filename || ! filename[0] )
      return NULL;

    pathname = find_pixmap_file (filename);

    if( ! pathname )
    {
        printf("Couldn't find pixmap file: %s\n", filename);
        printf("In path: %s", pathname);
        return NULL;
    }

    pixbuf = gdk_pixbuf_new_from_file (pathname, &error);
    if( ! pixbuf )
    {
        fprintf(stderr, "Failed to load pixbuf file: %s: %s\n",
                                       pathname, error->message);
        g_error_free(error);
    }
    g_free(pathname);

    return pixbuf;
}
