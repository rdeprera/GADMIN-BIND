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
#include "commands.h"
#include "dialog_if_files_missing.h"
#include "add_configuration_dialog.h"

extern char CHROOT_PATH_BUF[1024];



void dialog_if_files_missing(struct w *widgets)
{
    gchar *file;

    /* Show an add configuration dialog if one of the files is missing */
    if( ! file_exists(CHROOT_PATH_BUF) )
    {
        add_configuration_dialog(widgets);           
	return;
    }

    file = g_strdup_printf("%s/etc/named.conf", CHROOT_PATH_BUF);
    if( ! file_exists(file) )
    {
        add_configuration_dialog(widgets);           
	g_free(file);
	return;
    }
    g_free(file);

    file = g_strdup_printf("%s/etc/sites", CHROOT_PATH_BUF);
    if( ! file_exists(file) )
    {
        add_configuration_dialog(widgets);           
	g_free(file);
	return;
    }
    g_free(file);

    file = g_strdup_printf("%s/var/run", CHROOT_PATH_BUF);
    if( ! file_exists(file) )
    {
    	add_configuration_dialog(widgets);
	g_free(file);
	return;
    }
    g_free(file);

    file = g_strdup_printf("%s/etc/127.0.0", CHROOT_PATH_BUF);
    if( ! file_exists(file) )
    {
	add_configuration_dialog(widgets);
	g_free(file);
	return;
    }
    g_free(file);

    file = g_strdup_printf("%s/etc/localhost", CHROOT_PATH_BUF);
    if( ! file_exists(file) )
    {
	add_configuration_dialog(widgets);           
	g_free(file);
	return;
    }
    g_free(file);

    file = g_strdup_printf("%s/etc/localtime", CHROOT_PATH_BUF);
    if( ! file_exists(file) )
    {
	add_configuration_dialog(widgets);           
	g_free(file);
	return;
    }
    g_free(file);

    file = g_strdup_printf("%s/etc/root.hints", CHROOT_PATH_BUF);
    if( ! file_exists(file) )
    {
	add_configuration_dialog(widgets);           
	g_free(file);
	return;
    }
    g_free(file);

    file = g_strdup_printf("%s/dev/null", CHROOT_PATH_BUF);
    if( ! file_exists(file) )
    {
	add_configuration_dialog(widgets);           
	g_free(file);
	return;
    }
    g_free(file);

    file = g_strdup_printf("%s/dev/random", CHROOT_PATH_BUF);
    if( ! file_exists(file) )
    {
        add_configuration_dialog(widgets);
	g_free(file);
	return;
    }
    g_free(file);
}
