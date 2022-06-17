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
#include "show_info.h"



void show_help(struct w *widgets)
{
    gchar *text;
    
    text = g_strconcat(_("\n\n1. Adding master domains:\n"),
    _("   To add a master domain you will need a static IP-address range.\n"),
    _("   Supply the 2 entries with domain specific information.\n"),
    _("   Select master in the third box.\n"),
    _("   Click the add button to add this domain.\n"),
    _("   Now click on the domain and then add a name server in the resource manager.\n"),
    _("\n"),
    _("   When the domain and its resources is set up and functional you call the ISP.\n"),
    _("   The ISP will then delegate control over your domain name to you.\n"),
    _("   This may or may not be true depending on how your ISP does things.\n"),
    _("   Make sure its sending notifies for all zones in the system log tab.\n"),
    _("\n"),


    
    _("\n"),
    _("2. Adding slave domains:\n"),
    _("   To add a slave domain you will need a static IP-address.\n"),
    _("   Supply the 2 entries with domain specific information.\n"),
    _("   Select slave in the third box.\n"),
    _("   Click the add button to add this domain.\n"),
    _("   Now click on the domain and then add a name server in the resource manager.\n"),
    _("   Add more resources as you like and when youre satisfied you can activate the server.\n"),
    _("\n"),


    _("\n"),
    _("3. Adding forward caching domains: (Not completed)\n"),
    _("\n"),


    _("\n"),
    _("4. Adding stub domains: (Not completed)\n"),
    _("\n"),



    _("\n"),
    _("5. Altering domains:\n"),
    _("   Altering a domain is done by selecting a domain in the domain tab.\n"),
    _("   Now click the change button and a domain settings window will pop up.\n"),
    _("   In this window you can select how the server will function.\n"),
    _("\n"),
    _("   If the domain name is changed then all references to \"new.domain.se\" will\n"),
    _("   also be altered using sed so first make sure this is what you want to do.\n"),
    _("   IE: dont do this if you want to change a domain called hello.se and another domain\n"),
    _("   is called hello.se.happy.se this site will then be called new.domain.se.happy.se.\n"),
    _("\n"),
    _("   If the domain type is changed the server will automatically transform to this type.\n"),
    _("\n"),
    _("   Any changes must be confirmed by clicking the apply button and requires a reload.\n"),
    _("\n"),



    _("\n"),
    _("6. Zone reloads and key generation:\n"),
    _("   When using GAdmin-BIND for the first time you should go to the settings and generate a new key.\n"),
    _("   The key is used by rndc to connect to the server and configuring it.\n"),
    _("   If the current rndc.conf key and the chrooted named.conf's key differs then\n"),
    _("   rndc wont be able to connect to the server and you wont be able to reload the zones.\n"),
    _("\n"),


    "\n", NULL);


    show_info(text);

    g_free(text);
}
