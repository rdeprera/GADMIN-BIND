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
#include "show_info.h"
#include "gettext.h"
#include "resource_combo_changed.h"



void resource_combo_changed(struct w *widgets)
{
    gint active_index = 0;
    gchar *info, *utf8 = NULL;

    active_index = gtk_combo_box_get_active(GTK_COMBO_BOX(widgets->add_resource_type_combo));
    if( active_index <= 0 )
    {
	info = g_strdup_printf(_("A DNS Server: dns1.example.org"));
	utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);
	gtk_tooltips_set_tip(widgets->resource_hostname_tooltip, widgets->add_resource_hostname_entry, utf8, NULL);
	g_free(info);

	info = g_strdup_printf(_("An IPv4 or IPv6 address: 192.168.0.30 or fe80::213:8fff:feea:9b79/64"));
	utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);
	gtk_tooltips_set_tip(widgets->resource_address_tooltip, widgets->add_resource_address_entry, utf8, NULL);
	g_free(info);
    }
    if( active_index == 1 )
    {
	info = g_strdup_printf(_("A mail server: mail1.example.org"));
	utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);
	gtk_tooltips_set_tip(widgets->resource_hostname_tooltip, widgets->add_resource_hostname_entry, utf8, NULL);
	g_free(info);

	info = g_strdup_printf(_("The priority of this mail server: 10 or 20 etc, 10 beeing the highest priority of these."));
	utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);
	gtk_tooltips_set_tip(widgets->resource_address_tooltip, widgets->add_resource_address_entry, utf8, NULL);
	g_free(info);
    }
    if( active_index == 2 )
    {
	info = g_strdup_printf(_("The hostname of some host: router.example.org"));
	utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);
	gtk_tooltips_set_tip(widgets->resource_hostname_tooltip, widgets->add_resource_hostname_entry, utf8, NULL);
	g_free(info);

	info = g_strdup_printf(_("The IPv4 address of this host: 192.168.0.100"));
	utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);
	gtk_tooltips_set_tip(widgets->resource_address_tooltip, widgets->add_resource_address_entry, utf8, NULL);
	g_free(info);
    }
    if( active_index == 3 )
    {
	info = g_strdup_printf(_("The hostname of some host: router.example.org"));
	utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);
	gtk_tooltips_set_tip(widgets->resource_hostname_tooltip, widgets->add_resource_hostname_entry, utf8, NULL);
	g_free(info);

	info = g_strdup_printf(_("The IPv6 address of this host: fe80::213:8fff:feea:9b79/64"));
	utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);
	gtk_tooltips_set_tip(widgets->resource_address_tooltip, widgets->add_resource_address_entry, utf8, NULL);
	g_free(info);
    }
    if( active_index == 4 )
    {
	info = g_strdup_printf(_("The hostname of some host that already has an A record but not the name server: router.example.org"));
	utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);
	gtk_tooltips_set_tip(widgets->resource_hostname_tooltip, widgets->add_resource_hostname_entry, utf8, NULL);
	g_free(info);

	info = g_strdup_printf(_("The alias of the host: router.example.org"));
	utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);
	gtk_tooltips_set_tip(widgets->resource_address_tooltip, widgets->add_resource_address_entry, utf8, NULL);
	g_free(info);
    }
    
    if( utf8!=NULL )
        g_free(utf8);
}
