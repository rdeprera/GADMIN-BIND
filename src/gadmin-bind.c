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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "gettext.h"
#include "widgets.h"
#include "show_info.h"
#include "locate_icons.h"
#include "commands.h"
#include "allocate.h"
#include "add_standard_user.h"
#include "create_main_window.h"
#include "create_domain_tab.h"
#include "create_root_servers_tab.h"
#include "create_resolvers_tab.h"
#include "create_log_tab.h"
#include "create_verify_tab.h"
#include "populate_domain_treeview.h"
#include "populate_root_servers_tab.h"
#include "populate_resolvers.h"
#include "populate_log_tab.h"
#include "populate_resource_treeview.h"
#include "dialog_if_files_missing.h"
#include "select_first_domain.h"
#include "select_first_resource.h"
#include "status_update.h"
#include "load_settings.h"
#include "gadmin-bind.h"

/* Used as externs */
char global_domain_name[1024]="";
char global_domain_ipv4_address[1024]="";
char global_domain_ipv6_address[1024]="";
char global_domain_type[1024]="";

char global_resource_type[1024]="";
char global_resource_hostname[1024]="";
char global_resource_address[1024]="";

int activated=0;
int info_window_exit_main=0;

int MAX_CONF_LINE=1024;
int MAX_READ_POPEN=8192;

char CHROOT_PATH_BUF[1024]="";
char SYSLOG_PATH_BUF[1024]="";
char RESOLV_PATH_BUF[1024]="";



int main(int argc, char *argv[])
{
    gchar *command, *info;
    int i=0;

#ifdef ENABLE_NLS
    bindtextdomain(GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
    bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
    textdomain(GETTEXT_PACKAGE);
#endif

    gtk_set_locale();
    gtk_init(&argc, &argv);

    wid *widgets = g_malloc (sizeof (wid));

    /* Non root usage */
    if( ! getuid() == 0 )
    {
	/* For setting a different exit method in the info window */
	info_window_exit_main = 1;

        info = g_strdup_printf("You must be root to run: %s\nThis window will close in 10 seconds.\n", PACKAGE);
        show_info(info);
        g_free(info);

	/* Users can close the info window earlier then the timeout */
	for(i=0; i<10; i++)
	{
    	    while(gtk_events_pending())
                  gtk_main_iteration();

	    /* Set when close info window is clicked */
	    if( info_window_exit_main == 2 )
	      break;

    	    usleep(100000*10);
    	}

	g_free(widgets);
	return 0;
    }



    /* Load the settings for paths etc */
    load_settings(widgets);


    /* Use compiletime settings if there where no settings to load */
    if( strlen(CHROOT_PATH_BUF) < 5 )
      snprintf(CHROOT_PATH_BUF, 1000, "%s", CHROOT_PATH);

    if( strlen(SYSLOG_PATH_BUF) < 5 )
      snprintf(SYSLOG_PATH_BUF, 1000, "%s", SYSLOG_PATH);

    if( strlen(RESOLV_PATH_BUF) < 5 )
      snprintf(RESOLV_PATH_BUF, 1000, "%s", RESOLV_PATH);



    /* SELINUX can disallow named to use its conf or zone files so we make sure its ok */
    if( file_exists("/sbin/restorecon") )
    {
	command = allocate(1024);
	snprintf(command, 1000, "/sbin/restorecon -R %s", CHROOT_PATH_BUF);
	if( ! run_command(command) )
	{
	    printf("Error running SELINUX command: %s\n", command);
	    printf("There could be file access troubles but we continue anyway and hope for the best.\n");
	}
	free(command);
    }

    /* Add the NOBODY user if it doesnt exist.
       Shows detailed error information in the function */
    if( ! add_standard_user() )
    {
        while(gtk_events_pending())
              gtk_main_iteration();

	sleep(10);
	return 0;
    }


    if( file_exists(CHROOT_PATH_BUF) )
    {
	/* Chown nobody:nobody on all files and dirs created so far but
    	   not directory "named" (to allow for ddns updates etc) */
	command = g_strdup_printf("chown -R %s:%s %s/*", NAMED_USER, NAMED_USER, CHROOT_PATH_BUF);
	if( ! run_command(command) )
	{
    	    info = g_strdup_printf(_("Error running command:\n%s\n"), command);
    	    show_info(info);
    	    g_free(info);
	}
	g_free(command);
                                                           
	/* Chown the CHROOT to root:NAMED_USER (in many cases bind rpms uses the "named" user)
           so make sure that our NAMED_USER has write permissions there */
	command = g_strdup_printf("chown root:%s %s", NAMED_USER, CHROOT_PATH_BUF);
	if( ! run_command(command) )
	{
	    info = g_strdup_printf(_("Error running command:\n%s\n"), command);
    	    show_info(info);
    	    g_free(info);
	}
	g_free(command);
    }
    

    /* Create and show the various parts of the gui */
    create_main_window(widgets);

    create_domain_tab(widgets);

    create_root_servers_tab(widgets);
    populate_root_servers_tab(widgets);

    create_resolvers_tab(widgets);
    populate_resolvers(widgets);

    create_log_tab(widgets);
    populate_log_tab(widgets);

    create_verify_tab(widgets);


    /* Populate the domain treeview */
    populate_domain_treeview(widgets);
    select_first_domain(widgets);

    /* Populate the resource treeview */
    populate_resource_treeview(widgets);
    select_first_resource(widgets);

    gtk_widget_show_all(widgets->main_window);

    /* Start the online/offline timer */
    gtk_timeout_add(1000, (GtkFunction) status_update, widgets);

    /* If we dont have all the required directories and files, show a dialog */
    dialog_if_files_missing(widgets);


    /* Window (x) close button */
    g_signal_connect(GTK_OBJECT(widgets->main_window), "destroy",
                         GTK_SIGNAL_FUNC(gtk_main_quit), NULL);

    /* Start the main loop */
    gtk_main();

    g_free(widgets);

    return 0;
}
