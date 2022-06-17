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



#include "../config.h"
#include <gtk/gtk.h>
#include <stdio.h>
#include <string.h>
#include "widgets.h"
#include "gettext.h"
#include "domain_settings_type_combo_changed.h"



void domain_settings_type_combo_changed(struct w *widgets)
{
    gint active_index = 0;

    active_index = gtk_combo_box_get_active(GTK_COMBO_BOX(widgets->domain_settings_combo[0]));

    /* Domain type master selected */
    if( active_index <= 0 )
    {
	/* Allow-update widget is visible */
	gtk_widget_show(widgets->domain_settings_entry[10]);

	/* Allow-notify widget is not visible */
	gtk_widget_hide(widgets->domain_settings_entry[14]);

	/* Master servers widget is not visible */
	gtk_widget_hide(widgets->domain_settings_entry[15]);
    }

    /* Domain type slave selected */
    if( active_index == 1 )
    {
	/* Allow-update widget is not visible */
	gtk_widget_hide(widgets->domain_settings_entry[10]);

	/* Allow-notify widget is visible */
	gtk_widget_show(widgets->domain_settings_entry[14]);

	/* Master servers widget is visible */
	gtk_widget_show(widgets->domain_settings_entry[15]);
    }
}
