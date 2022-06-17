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
#include <stdlib.h>
#include "widgets.h"
#include "gettext.h"
#include "allocate.h"
#include "show_info.h"
#include "commands.h"
#include "new_rndc_key.h"

#define MAX_CONF_LINE 1000

extern int MAX_READ_POPEN;

extern char CHROOT_PATH_BUF[1024];


void new_rndc_key(struct w *widgets)
{
    FILE *fp;
    long file_size=0;
    char *line, *new_conf, *confgen_output, *key_line, *key;
    int i=0, key_found=0, key_changed=0;
    gchar *command, *info, *named_conf, *utf8=NULL;

    /* We use /dev/urandom because it contains alot of random data */
    if( ! file_exists("/dev/urandom") )
    {
        info = g_strdup_printf(_("The randomized file does not exist here: /dev/urandom\n"));
        show_info(info);
        g_free(info);
	return;
    }

    /* Read input from the rndc-confgen program, get the secret key */
    command = g_strdup_printf("%s -b 512 -r /dev/urandom 2>&1", RNDC_CONFGEN_BINARY);
    if((fp=popen(command, "r"))==NULL)
    {
        info = g_strdup_printf(_("Updating rndc key with rndc-confgen failed with command:\n%s\n"), command);
        show_info(info);
        g_free(info);
        g_free(command);
        return;
    }
    g_free(command);    

    line = allocate(MAX_READ_POPEN+1);
    confgen_output = allocate(MAX_READ_POPEN+1);
    key = allocate(1024);
    
    while(fgets(line, MAX_READ_POPEN, fp)!=NULL)
    {
	/* The output is saved and used later if theres no "secret" line in /etc/rndc.conf */
	strcat(confgen_output, line);

        if( strstr(line, "secret \"") && strlen(line) < 1000 && ! key_found )
        {
	    /* Scroll to the first '"' */
	    for(i=0; line[i]!='\0'; i++)
	       if( line[i]=='"' )
	         break;
	    
	    snprintf(key, 999, "%s", &line[i+1]);
	    if( key!=NULL && strlen(key) > 5 )
	      key[strlen(key)-3]='\0';

	    key_found = 1;
        }
    }
    pclose(fp);
    free(line);

    if( ! key_found )
    {
        info = g_strdup_printf(_("Error: New key not generated, rndc-confgen failed.\n"));
        show_info(info);
        g_free(info);
	free(confgen_output);
	free(key);
        return;
    }

    /* Insert the new key into named.conf */
    named_conf = g_strdup_printf("%s/etc/named.conf", CHROOT_PATH_BUF);
    if((fp=fopen(named_conf, "r"))==NULL)
    {
        info = g_strdup_printf(_("Error: cannot open file:\n%s\n"), named_conf);
        show_info(info);
        g_free(info);
	g_free(named_conf);
	free(confgen_output);
	free(key);
        return;
    }
    fseek(fp, 0, SEEK_END);
    file_size = ftell(fp);
    rewind(fp);

    line = allocate(file_size+1);
    new_conf = allocate(file_size+16384);
    key_line = allocate(1024);

    snprintf(key_line, 1024, "\tsecret \"%s\";\n", key);

    if( file_size > 1 )    
    while(fgets(line, file_size, fp)!=NULL)
    {
       if( strstr(line, "secret") && strstr(line, "\"") && strstr(line, ";") 
       && strlen(line) < 1000 && ! key_changed )
       {
	    strcat(new_conf, key_line);
	    key_changed = 1;
       }
       else
         strcat(new_conf, line);
    }
    fclose(fp);
    free(line);


    /* Insert the new key into the key settings entry */
    utf8 = g_locale_to_utf8(key, strlen(key), NULL, NULL, NULL);    
    gtk_entry_set_text(GTK_ENTRY(widgets->settings_entry[3]), utf8);
    if( utf8!=NULL )
      g_free(utf8);

    free(key);

    if( ! key_changed )
    {
        info = g_strdup_printf(_("Error: could not find the secret key line in:\n%s\n"), named_conf);
        show_info(info);
        g_free(info);
	free(new_conf);
	free(key_line);
	free(confgen_output);
	g_free(named_conf);
        return;
    }


    /* Write the new named.conf */
    if((fp=fopen(named_conf, "w+"))==NULL)
    {
        info = g_strdup_printf(_("Error: cannot write named.conf here:\n%s\n"), named_conf);
        show_info(info);
        g_free(info);
	free(new_conf);
	free(confgen_output);
	g_free(named_conf);
        return;
    }
    fputs(new_conf, fp);
    fclose(fp);
    free(new_conf);

    g_free(named_conf);


    key_changed = 0;


    /* Add RNDC_CONF if it doesnt exist and chmod it to 640 */
    if( ! file_exists(RNDC_CONF) )
    {
	command = g_strdup_printf("echo \"#Created by gadmin-bind\" > %s && chmod 640 %s", RNDC_CONF, RNDC_CONF);
	if((fp=popen(command, "r"))==NULL)
	{
    	    info = g_strdup_printf(_("Error running command: %s\n"), command);
    	    show_info(info);
    	    g_free(info);
    	    g_free(command);
	    free(confgen_output);
	    free(key_line);
    	    return;
	}
	pclose(fp);
	g_free(command);    
    }
    

    /* Change the "secret" in /etc/rndc.conf if any */
    if((fp=fopen(RNDC_CONF, "r"))==NULL)
    {
	info = g_strdup_printf(_("Error: cannot open file: %s\n"), RNDC_CONF);
    	show_info(info);
    	g_free(info);
	free(confgen_output);
	free(key_line);
    	return;
    }
    fseek(fp, 0, SEEK_END);
    file_size = ftell(fp);
    rewind(fp);

    line = allocate(file_size+1);
    new_conf = allocate(file_size+16384);

    if( file_size > 1 )    
    while(fgets(line, file_size, fp)!=NULL)
    {
    	if( strstr(line, "secret") && strstr(line, "\"") && strstr(line, ";") 
    	&& strlen(line) < 1000 && ! key_changed  )
    	{
	    strcat(new_conf, key_line);
	    key_changed = 1;
    	}
    	else
          strcat(new_conf, line);
    }
    fclose(fp);
    free(line);
    free(key_line);



    /* If the secret key hasnt changed because there was 
       no "secret" line in /etc/rndc.conf we write a new /etc/rndc.conf */
    if( ! key_changed )
      snprintf(new_conf, 1000, "%s", confgen_output);


    free(confgen_output);


    /* Write the new rndc.conf */
    if((fp=fopen(RNDC_CONF, "w+"))==NULL)
    {
        info = g_strdup_printf(_("Error: cannot write rndc.conf here: %s\n"), RNDC_CONF);
        show_info(info);
        g_free(info);
	free(new_conf);
        return;
    }
    fputs(new_conf, fp);
    fclose(fp);

    free(new_conf);

    /* The server needs to be restarted so that rndc can connect to it */
    info = g_strdup_printf(_("\nKey generation and configuration updates successful.\nThe server needs to be restarted now so it updates its internal key.\nThis must be done so that rndc is allowed to connect and zones can be reloaded.\n"));
    show_info(info);
    g_free(info);
}
