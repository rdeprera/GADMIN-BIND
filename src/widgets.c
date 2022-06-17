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

/* Used in the domain settings window */
#define NUM_DOMAIN_SETTINGS_ENTRIES 20
#define NUM_DOMAIN_SETTINGS_COMBOS  3

/* Used in the settings window */
#define NUM_SETTINGS_ENTRIES 10
#define NUM_SETTINGS_COMBOS  2

/* Not used much yet, same as above */
#define NUM_DOMAIN_SETTINGS 16


typedef struct w
{
  GtkWidget *main_window;
  GtkWidget *main_vbox;
  GtkWidget *notebook_vbox1;
  GtkWidget *notebook_vbox2;
  GtkWidget *notebook_vbox3;
  GtkWidget *notebook_vbox4;
  GtkWidget *notebook_vbox5;
  GtkWidget *status_label;
  GtkWidget *version_label;
  GtkWidget *domain_treeview;
  GtkListStore *domain_store;
  GtkTreePath *domain_treepath;

  /* Add domain entries */
  GtkWidget *add_domain_name_entry;
//  GtkWidget *add_domain_address_entry;
  GtkWidget *add_domain_ipv4_address_entry;
  GtkWidget *add_domain_ipv6_address_entry;
  GtkWidget *add_domain_type_combo;

  /* Domain resources (NS, A, AAAA, MX, PTR, CNAME) */
  GtkWidget *resource_treeview;
  GtkListStore *resource_store;

  /* Add resource entries */
  GtkWidget *add_resource_type_combo;
  GtkWidget *add_resource_hostname_entry;
  GtkWidget *add_resource_address_entry;

  /* Resource tooltips changes with the combo selections */
  GtkTooltips *resource_hostname_tooltip;
  GtkTooltips *resource_address_tooltip;

  /* Domain settings window */
  GtkWidget *domain_settings_window;
  GtkWidget *domain_settings_entry[NUM_DOMAIN_SETTINGS_ENTRIES];
  GtkWidget *domain_settings_combo[NUM_DOMAIN_SETTINGS_COMBOS];

  /* Settings window */
  GtkWidget *settings_window;
  GtkWidget *settings_entry[NUM_SETTINGS_ENTRIES];
  GtkWidget *settings_combo[NUM_SETTINGS_COMBOS];

  /* Add new configuration window */
  GtkWidget *add_configuration_window;

  /* Root servers tab (rs) */
  GtkWidget *rs_textview;  

  GtkWidget *credits_window;

  /* The resolvers tab */
  GtkWidget *resolvers_textview;

  /* Syslog tab */
  GtkWidget *log_textview;  

  /* Verify tab */
  GtkWidget *verify_textview;  
  GtkWidget *verify_run_cmd_entry;  

}wid;
