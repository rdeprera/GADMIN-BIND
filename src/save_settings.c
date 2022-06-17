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
#include <stdlib.h>
#include <string.h>
#include "widgets.h"
#include "gettext.h"
#include "show_info.h"
#include "commands.h"
#include "allocate.h"
#include "populate_domain_treeview.h"
#include "populate_resource_treeview.h"
#include "populate_resolvers.h"
#include "populate_root_servers_tab.h"
#include "populate_log_tab.h"
#include "dialog_if_files_missing.h"
#include "save_settings.h"


extern char CHROOT_PATH_BUF[1024];
extern char SYSLOG_PATH_BUF[1024];
extern char RESOLV_PATH_BUF[1024];



void save_settings(struct w *widgets)
{
    FILE *fp;
    gchar *settings_file, *cmd, *info, *setting;
    G_CONST_RETURN gchar *set_chroot;
    G_CONST_RETURN gchar *set_resolv;
    G_CONST_RETURN gchar *set_syslog;

    /* Make the settings directory if it doesnt exist */
    cmd = g_strdup_printf("mkdir -p %s", SETTINGS_DIR);
    if( ! file_exists(SETTINGS_DIR) )
      run_command(cmd);
    g_free(cmd);

    if( ! file_exists(SETTINGS_DIR) )
    {
	info = g_strdup_printf(_("The settings directory could not be created, run gadmin-bind as root\n"));
	show_info(info);
	g_free(info);    
    }

    settings_file = g_strdup_printf("%s/settings.conf", SETTINGS_DIR);

    set_chroot = gtk_entry_get_text(GTK_ENTRY(widgets->settings_entry[0]));
    set_resolv = gtk_entry_get_text(GTK_ENTRY(widgets->settings_entry[1]));
    set_syslog = gtk_entry_get_text(GTK_ENTRY(widgets->settings_entry[2]));

    /* Update the global paths */
    snprintf(CHROOT_PATH_BUF, 1000, "%s", set_chroot);
    snprintf(RESOLV_PATH_BUF, 1000, "%s", set_resolv);
    snprintf(SYSLOG_PATH_BUF, 1000, "%s", set_syslog);

    /* Write the new settings */
    if((fp=fopen(settings_file, "w+"))==NULL)
    {
        printf("Couldnt open the settings file here:\n%s\n", settings_file);
	g_free(settings_file);
        return;
    }

    if( strlen(set_chroot) > 5 )
    {
	setting = g_strdup_printf("\nchroot_directory: %s\n", set_chroot);
	fputs(setting, fp);
	g_free(setting);
    }

    if( strlen(set_chroot) > 5 )
    {
	setting = g_strdup_printf("syslog_file_path: %s\n", set_syslog);
	fputs(setting, fp);
	g_free(setting);
    }
    
    if( strlen(set_chroot) > 5 )
    {
	setting = g_strdup_printf("resolv_conf_path: %s\n", set_resolv);
        fputs(setting, fp);
	g_free(setting);
    }
    
    fclose(fp);
    g_free(settings_file);

    /* Reread the entire gui */
    populate_domain_treeview(widgets);

    populate_resource_treeview(widgets);

    populate_resolvers(widgets);
    
    populate_root_servers_tab(widgets);
    
    populate_log_tab(widgets);

    /* If theres no valid chroot in the new 
       location we ask to create one */
    dialog_if_files_missing(widgets);
}
