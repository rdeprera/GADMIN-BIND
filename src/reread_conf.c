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
#include <unistd.h>
#include <string.h>
#include "widgets.h"
#include "gettext.h"
#include "commands.h"
#include "chars_are_digits.h"
#include "clear_log_tab.h"
#include "populate_log_tab.h"

extern int activated;

extern char CHROOT_PATH_BUF[1024];



void reread_conf(struct w *widgets)
{
    /* It doesnt restart/reread with a HUP, no problem */
    gchar *command, *test;

    if( ! activated )
      return;

    command = g_strdup_printf("killall -15 %s", NAMED_BINARY);
    if( ! run_command(command) )
      printf("named shutdown failed, trying to start it\n");

    g_free(command);
    

    clear_log_tab(widgets);

    if (strlen(CHROOT_PATH_BUF) > 0){
      command = g_strdup_printf("%s -u %s -t %s -c %s", NAMED_BINARY, NAMED_USER, CHROOT_PATH_BUF, BIND_CONF);
    }else{
      command = g_strdup_printf("%s -u %s -c %s", NAMED_BINARY, NAMED_USER, BIND_CONF);
    }
    
    if( ! run_command(command) )
    {
        printf("Restarting named failed.\n");
        test = g_strdup_printf("%s -fg -u %s -t %s -c %s 2>&1", NAMED_BINARY, NAMED_USER, CHROOT_PATH_BUF, BIND_CONF);
	run_command_show_err(test);
	g_free(test);
    }
    g_free(command);


    if( ! chars_are_digits(CMD_SLEEP_LOG_SECS) )
    {
	printf("CMD_SLEEP_LOG_SECS should be in seconds, please reinstall the program\n");
	return;
    }
    if( strlen(CMD_SLEEP_LOG_SECS) > 2 ) /* 99 seconds max */
    {
	printf("CMD_SLEEP_LOG_SECS max length is 2 chars IE: 99 seconds, please reinstall the program\n");
	return;
    }
    sleep(atoi(CMD_SLEEP_LOG_SECS));

    populate_log_tab(widgets);
}
