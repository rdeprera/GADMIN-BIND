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
#include "add_configuration.h"
#include "add_configuration_canceled.h"
#include "populate_root_servers_tab.h"
#include "commands.h"
#include "show_info.h"

extern char CHROOT_PATH_BUF[1024];



void add_configuration(struct w *widgets)
{
    FILE *fp;
    gchar *command=NULL, *info=NULL;
    gchar *dir, *file, *conf;

    /* This code creates as much of the named chroot as possible.
       In order to do so it does not return on any failures. 
       Popup windows describing any problem(s) are shown */

    gtk_widget_destroy(widgets->add_configuration_window);

    
    /* Make the directories if they dont exist */
    dir = g_strdup_printf("%s/dev", CHROOT_PATH_BUF);
    if( ! file_exists(dir) )
    {
	command = g_strdup_printf("mkdir -p %s/dev", CHROOT_PATH_BUF);
	if( ! run_command(command) )
	{
	    info = g_strdup_printf(_("Error running command:\n%s\n"), command);
	    show_info(info);
	    g_free(info);
	}
        g_free(command);
    }
    g_free(dir);


    dir = g_strdup_printf("%s/etc/sites", CHROOT_PATH_BUF);
    if( ! file_exists(dir) )
    {
	command = g_strdup_printf("mkdir -p %s/etc/sites", CHROOT_PATH_BUF);
	if( ! run_command(command) )
	{
	    info = g_strdup_printf(_("Error running command:\n%s\n"), command);
	    show_info(info);
	    g_free(info);
	}
        g_free(command);
    }
    g_free(dir);


    dir = g_strdup_printf("%s/var/run", CHROOT_PATH_BUF);
    if( ! file_exists(dir) )
    {
	command = g_strdup_printf("mkdir -p %s/var/run", CHROOT_PATH_BUF);
	if( ! run_command(command) )
	{
	    info = g_strdup_printf(_("Error running command:\n%s\n"), command);
	    show_info(info);
	    g_free(info);
	}
        g_free(command);
    }
    g_free(dir);




    /* Add /var/named/etc/named.conf if it doesnt exist */
    file = g_strdup_printf("%s%s", CHROOT_PATH_BUF,BIND_CONF);
    if( ! file_exists(file) )
    {
	if((fp=fopen(file, "w+"))==NULL)
	{
	    info = g_strdup_printf(_("Error: could not write named.conf here:\n%s\n"), file);
	    show_info(info);
	    g_free(info);

	    g_free(file);
	    return;
	}

	conf = g_strconcat("options {\n",
	"    directory \"/etc\";\n",
	"    pid-file \"/var/run/named.pid\";\n",
	"    statistics-file \"/var/run/named.stats\";\n",
	"    version \"Surely you must be joking\";\n",
	"    listen-on port 53 { any; };\n",
	"    listen-on-v6 port 53 { any; };\n",
	"};\n\n",

	"controls {\n",
	"    inet 127.0.0.1 allow { localhost; } keys { rndc_key; };\n",
	"};\n\n",
		    
	"key \"rndc_key\" {\n",
	"    algorithm hmac-md5;\n",
	"    secret \"\";\n",
	"};\n\n",
			    

	"logging {\n",
	"    category default { default_syslog; default_debug; };\n",
	"    category unmatched { null; };\n\n",

	"    channel default_syslog {\n",
	"	syslog daemon;\n",
	"	severity info;\n",
	"    };\n\n",
								    
	"    channel default_debug {\n",
	"	file \"named.run\";\n",
	"	severity dynamic;\n",
	"    };\n\n",

	"    channel default_stderr {\n",
	"	stderr;\n",
	"	severity info;\n",
	"    };\n\n",

	"    channel null {\n",
	"	null;\n",
	"    };\n",
	"};\n\n"

	"zone \".\" {\n",
	"    type hint;\n",
	"    file \"/etc/root.hints\";\n",
	"};\n\n",

	"zone \"localhost\" {\n",
	"    type master;\n",
	"    file \"/etc/localhost\";\n",
	"};\n\n",
				    
	"zone \"0.0.127.in-addr.arpa\" {\n",
	"    type master;\n",
	"    file \"/etc/127.0.0\";\n",
	"};\n\n",
	"", NULL);

	fputs(conf, fp);
	fclose(fp);
	g_free(conf);
    }
    g_free(file);


    /*	Add /var/named/etc/root.hints if it doesnt exist */
    file = g_strdup_printf("%s/etc/root.hints", CHROOT_PATH_BUF);
    if( ! file_exists(file) )
    {
	if((fp=fopen(file, "w+"))==NULL)
	{
	    info = g_strdup_printf(_("Error: couldnt write file root.hints here:\n%s\n"), file);
	    show_info(info);
	    g_free(info);

	    g_free(file);
	    return;
	}

	conf = g_strconcat(_("; This configuration is old. A working DNS resolver is required to update.\n\n"),
	NULL);

	fputs(conf, fp);
	fclose(fp);
	g_free(conf);
    }
    g_free(file);


    /*	Add /var/named/etc/localhost if it doesnt exist */
    file = g_strdup_printf("%s/etc/localhost", CHROOT_PATH_BUF);
    if( ! file_exists(file) )
    {
	if((fp=fopen(file, "w+"))==NULL)
	{
	    info = g_strdup_printf(_("Error: couldnt write file localhost here:\n%s\n"), file);
	    show_info(info);
	    g_free(info);

	    g_free(file);
	    return;
	}

	conf = g_strconcat("$TTL 3D\n",
	"@       IN      SOA     @       root (\n",
	"                        1       ; serial\n",
	"			3600    ; refresh\n",
	"			1800    ; retry\n",
	"			604800  ; expiration\n",
	"                	3600 )  ; minimum\n\n",

	"        IN      NS      @\n",
	"        IN      A       127.0.0.1\n",
	"        IN      AAAA    ::1\n\n"
	"", NULL);

	fputs(conf, fp);
	fclose(fp);
	g_free(conf);
    }
    g_free(file);



    /*	Add /var/named/etc/127.0.0 if it doesnt exist */
    file = g_strdup_printf("%s/etc/127.0.0", CHROOT_PATH_BUF);
    if( ! file_exists(file) )
    {
	if((fp=fopen(file, "w+"))==NULL)
	{
	    info = g_strdup_printf(_("Error: couldnt write file 127.0.0 here:\n%s\n"), file);
	    show_info(info);
	    g_free(info);

	    g_free(file);
	    return;
	}

	conf = g_strconcat("$TTL 3D\n",
	"@       IN      SOA     localhost. root.localhost. (\n",
        "    		 01      ; Serial\n",
        "    		 8H      ; Refresh\n",
        "    		 2H      ; Retry\n",
        "    		 4W      ; Expire\n",
        "    		 3D)     ; Minimum TTL\n\n",

    	"        IN      NS      localhost.\n",
	"1       IN      PTR     localhost.\n",
	"", NULL);

	fputs(conf, fp);
	fclose(fp);
	g_free(conf);
    }
    g_free(file);


    /*	Copy /etc/localtime from /etc to /var/named/etc/localtime if it doesnt exist */
    file = g_strdup_printf("%s/etc/localtime", CHROOT_PATH_BUF);
    if( ! file_exists(file) )
    {
	g_free(file);

	file = g_strdup_printf("/etc/localtime");
	if( ! file_exists(file) )
	{
	    info = g_strdup_printf(_("Error copying /etc/localtime, the file doesnt exist\n"));
	    show_info(info);
	    g_free(info);
	}
	else
	  {
		command = g_strdup_printf("cp /etc/localtime %s/etc/", CHROOT_PATH_BUF);
		if( ! run_command(command) )
    		{
		    info = g_strdup_printf(_("Error running command:\n%s\n"), command);
		    show_info(info);
		    g_free(info);
		}
		g_free(command);
	  }
	g_free(file);
    }
    else
      g_free(file);


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
       so make sure that the NAMED_USER IE: "nobody" has write permissions there */
    command = g_strdup_printf("chown root:%s %s", NAMED_USER, CHROOT_PATH_BUF);
    if( ! run_command(command) )
    {
	info = g_strdup_printf(_("Error running command:\n%s\n"), command);
	show_info(info);
	g_free(info);
    }
    g_free(command);

    /* Make dev/null and random if they dont exist */
    file = g_strdup_printf("%s/dev/null", CHROOT_PATH_BUF);
    if( ! file_exists(file) )
    {
	command = g_strdup_printf("mknod %s/dev/null c 1 3", CHROOT_PATH_BUF);
	if( ! run_command(command) )
	{
	    info = g_strdup_printf(_("Error running command:\n%s\n"), command);
	    show_info(info);
	    g_free(info);
	}
        g_free(command);
    }

    file = g_strdup_printf("%s/dev/random", CHROOT_PATH_BUF);
    if( ! file_exists(file) )
    {
	command = g_strdup_printf("mknod %s/dev/random c 1 8", CHROOT_PATH_BUF);
	if( ! run_command(command) )
	{
	    info = g_strdup_printf(_("Error running command:\n%s\n"), command);
	    show_info(info);
	    g_free(info);
	}
        g_free(command);
    }

    /* Chmod the nodes */
    command = g_strdup_printf("chmod 666 %s/dev/null", CHROOT_PATH_BUF);
    if( ! run_command(command) )
    {
	info = g_strdup_printf(_("Error running command:\n%s\n"), command);
	show_info(info);
	g_free(info);
    }
    g_free(command);
    
    command = g_strdup_printf("chmod 666 %s/dev/random", CHROOT_PATH_BUF);
    if( ! run_command(command) )
    {
	info = g_strdup_printf(_("Error running command:\n%s\n"), command);
	show_info(info);
	g_free(info);
    }
    g_free(command);

    /* Populate the root servers tab. No other tabs need to be repopulated. */
    populate_root_servers_tab(widgets);
}
