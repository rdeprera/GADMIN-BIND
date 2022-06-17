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
#include <stdlib.h>
#include "widgets.h"
#include "gettext.h"
#include "allocate.h"
#include "show_info.h"
#include "reread_conf.h"
#include "commands.h"
#include "reverse_address.h"
#include "populate_resource_treeview.h"

extern char global_domain_type[1024];
extern char global_domain_name[1024];
extern char global_domain_ipv4_address[1024];
extern char global_domain_ipv6_address[1024];

extern char global_resource_type[1024];
extern char global_resource_hostname[1024];
extern char global_resource_address[1024];

extern char CHROOT_PATH_BUF[1024];
extern int MAX_CONF_LINE;



/* Deletes the selected resource in the forward and reverse zones */
void delete_resource(struct w *widgets)
{
    FILE *fp;
    long file_size=0;
    char *new_conf;
    char *line;
    gchar *info;
    char *tmp1, *tmp2, *tmp3, *tmp4;
    gchar *forward_zone_file;
    gchar *reverse_zone_file;
    char *last_octets;

    if( strlen(global_domain_type)==0 )
    {
        info = g_strdup_printf(_("Missing domain type, resource not deleted\n"));
        show_info(info);
        g_free(info);
        return;
    }
    if( strlen(global_domain_name)==0 )
    {
        info = g_strdup_printf(_("Missing domain name, resource not deleted\n"));
        show_info(info);
        g_free(info);
        return;
    }
    if( strlen(global_domain_ipv4_address)==0
    &&  strlen(global_domain_ipv6_address)==0 )
    {
        info = g_strdup_printf(_("Missing forward domain address, resource not deleted\n"));
        show_info(info);
        g_free(info);
        return;
    }
    if( strlen(global_resource_type)==0 )
    {
        info = g_strdup_printf(_("Missing resource type, resource not deleted\n"));
        show_info(info);
        g_free(info);
        return;
    }
    if( strlen(global_resource_hostname)==0 )
    {
    	info = g_strdup_printf(_("Missing resource hostname, resource not deleted\n"));
        show_info(info);
        g_free(info);
        return;
    }
    /* NS-Records without forward addresses should be deletable */
    if( strlen(global_resource_address)==0
    && ! strstr(global_resource_type, "NS-Record") )
    {
        info = g_strdup_printf(_("Missing forward resource address, resource not deleted\n"));
        show_info(info);
        g_free(info);
        return;
    }


    /* Delete the selected resource from the forward zone */
    forward_zone_file = g_strdup_printf("%s/etc/sites/%s/forward.zone", CHROOT_PATH_BUF, global_domain_name);
    if((fp=fopen(forward_zone_file, "r"))==NULL)
    {
        info = g_strdup_printf(_("Could not open the forward zone file here:\n%s\n"), forward_zone_file);
        show_info(info);
        g_free(info);
	g_free(forward_zone_file);
        return;
    }
    fseek(fp, 0, SEEK_END);
    file_size = ftell(fp);
    rewind(fp);

    new_conf = allocate(file_size+1);
    line = allocate(file_size+1);

    tmp1 = allocate(MAX_CONF_LINE);
    tmp2 = allocate(MAX_CONF_LINE);
    tmp3 = allocate(MAX_CONF_LINE);
    tmp4 = allocate(MAX_CONF_LINE);

    if( file_size > 1 )
    while(fgets(line, file_size, fp)!=NULL)
    {
	if( strlen(line) > MAX_CONF_LINE-3 )
	  continue;

	if( strlen(line) < 2 )
	  continue;
	
	tmp1[0]='\0', tmp2[0]='\0', tmp3[0]='\0', tmp4[0]='\0';
	sscanf(line, "%s %s %s %s", tmp1, tmp2, tmp3, tmp4);

	/* Skip entries that should be deleted */
	if(  !strcmp(global_resource_type, "NS-Record")
	&& ( !strcmp(tmp2, global_resource_hostname) || !strcmp(tmp3, global_resource_hostname) )
	&& ( !strcmp(tmp1, "NS") || !strcmp(tmp2, "NS") ) )
	{
	    printf("Deleting Forward NS: %s\n", line);
	}
	else
	if(  !strcmp(global_resource_type, "MX-Record")
	&& ( !strcmp(tmp3, global_resource_hostname) || !strcmp(tmp4, global_resource_hostname) )
	&& ( !strcmp(tmp1, "MX") || !strcmp(tmp2, "MX") ) )
	{
	    printf("Deleting Forward MX: %s\n", line);
	}
	else
	if(  !strcmp(global_resource_type, "A-Record") 
	&&   !strcmp(tmp1, global_resource_hostname) 
	&& ( !strcmp(tmp3, global_resource_address) || !strcmp(tmp4, global_resource_address) )
	&& ( !strcmp(tmp2, "A") || !strcmp(tmp3, "A") ) )
	{
	    printf("Deleting Forward A: %s\n", line);    
	}
	else
	if(  !strcmp(global_resource_type, "AAAA-Record") 
	&&   !strcmp(tmp1, global_resource_hostname) 
	&& ( !strcmp(tmp3, global_resource_address) || !strcmp(tmp4, global_resource_address) )
	&& ( !strcmp(tmp2, "AAAA") || !strcmp(tmp3, "AAAA") ) )
	{
	    printf("Deleting Forward AAAA: %s\n", line);    
	}
	else
	if(  !strcmp(global_resource_type, "CNAME-Record")
	&&   !strcmp(tmp1, global_resource_address)
	&& ( !strcmp(tmp3, global_resource_hostname) || !strcmp(tmp4, global_resource_hostname) )
	&& ( !strcmp(tmp2, "CNAME") || !strcmp(tmp3, "CNAME") ) )
	{
	    printf("Deleting Forward CNAME: %s\n", line);    
	}
	else
	  strcat(new_conf, line);
    }
    fclose(fp);


    /* Write the new forward zone */
    if((fp=fopen(forward_zone_file, "w+"))==NULL)
    {
        info = g_strdup_printf(_("Could not write the forward zone file here:\n%s\n"), forward_zone_file);
        show_info(info);
        g_free(info);
        
	free(new_conf);
	free(line);
	free(tmp1); free(tmp2); free(tmp3); free(tmp4);
	g_free(forward_zone_file);
        return;
    }
    fputs(new_conf, fp);

    free(new_conf);
    free(line);
    free(tmp1); free(tmp2); free(tmp3); free(tmp4);
    g_free(forward_zone_file);
    fclose(fp);



    /* Reverse IPv4 zone resource deletion Begin! */

    /* Return if there should be no PTR for this record type */
    if( strcmp(global_resource_type, "NS-Record") 
    &&  strcmp(global_resource_type, "A-Record") 
    &&  strcmp(global_resource_type, "AAAA-Record") )
    {
	return;
    }


    /* Delete the selected resource from the reverse IPv4 zone */
    reverse_zone_file = g_strdup_printf("%s/etc/sites/%s/reverse.zone.ipv4", CHROOT_PATH_BUF, global_domain_name);
    if((fp=fopen(reverse_zone_file, "r"))==NULL)
    {
        info = g_strdup_printf(_("Could not open the reverse IPv4 zone file here:\n%s\n"), reverse_zone_file);
        show_info(info);
        g_free(info);

	g_free(reverse_zone_file);
        return;
    }
    fseek(fp, 0, SEEK_END);
    file_size = ftell(fp);
    rewind(fp);

    new_conf = allocate(file_size+1);
    line = allocate(file_size+1);

    tmp1 = allocate(MAX_CONF_LINE);
    tmp2 = allocate(MAX_CONF_LINE);
    tmp3 = allocate(MAX_CONF_LINE);
    tmp4 = allocate(MAX_CONF_LINE);


    /* Get the last octets for the IP address or NULL (Wont match any address) */
    last_octets = get_ptr_addr_from_forward_address((char *)global_resource_address);


    if( file_size > 1 )
    while(fgets(line, file_size, fp)!=NULL)
    {
	if( strlen(line) > MAX_CONF_LINE-3 )
	  continue;

	if( strlen(line) < 2 )
	  continue;
	
	tmp1[0]='\0', tmp2[0]='\0', tmp3[0]='\0', tmp4[0]='\0';
	sscanf(line, "%s %s %s %s", tmp1, tmp2, tmp3, tmp4);

	/* Skip lines that should be deleted */
	if(  !strcmp(global_resource_type, "NS-Record")
	&& ( !strcmp(tmp2, global_resource_hostname) || !strcmp(tmp3, global_resource_hostname) )
	&& ( !strcmp(tmp1, "NS") || !strcmp(tmp2, "NS") ) )
	{
	    printf("Deleting Reverse IPv4 NS: %s\n", line);
	}
	else
	if(  !strcmp(global_resource_type, "A-Record")
	&&   !strcmp(tmp1, last_octets)
	&& ( !strcmp(tmp3, global_resource_hostname) || !strcmp(tmp4, global_resource_hostname) )
	&& ( !strcmp(tmp2, "PTR") || !strcmp(tmp3, "PTR") ) )
	{
	    printf("Deleting Reverse IPv4 PTR: %s\n", line);
	}
	else
	  strcat(new_conf, line);
    }
    fclose(fp);

    /* Write the new conf */
    if((fp=fopen(reverse_zone_file, "w+"))==NULL)
    {
        info = g_strdup_printf(_("Could not write the reverse IPv4 zone file here:\n%s\n"), reverse_zone_file);
        show_info(info);
        g_free(info);

	free(new_conf);
	g_free(reverse_zone_file);
	g_free(last_octets);
        return;
    }
    fputs(new_conf, fp);
    free(new_conf);
    g_free(reverse_zone_file);
    fclose(fp);

    if( last_octets!=NULL )
      free(last_octets);
}
