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
#include "allocate.h"
#include "functions.h"



int
user_exists(G_CONST_RETURN gchar *username)
{
    /* Checks if the system user exists */
    FILE *fp;
    long file_size;
    int x, user_exists = 0;
    char tempname[4096]="";
    char *check_buffer;

    /* Checks if the user exists in passwd */
    if((fp=fopen(PASSWD_FILE,"r"))==NULL)
    {
	/* Dont show anything */
    }
    else
      {
	    fseek(fp, 0, SEEK_END);
	    file_size = ftell(fp);
	    rewind(fp);
	    check_buffer = allocate(file_size);

	    if( file_size > 1 )
	    while(fgets(check_buffer, file_size, fp)!=NULL)
	    {
		for(x=0; check_buffer[x]; x++)
		{
		    if(check_buffer[x]==':')
		    {
			sprintf(tempname, "%s", check_buffer);
			tempname[x]='\0';
			if( ! strcmp(username, tempname) )
			{
			    user_exists = 1;
			    break;
			}
		    }
		    if( check_buffer[x]=='\0' || check_buffer[x]=='\n' )
		    break;
		}
	    }
	    free(check_buffer);
      }

    fclose(fp);


    /* Checks if the user exists in shadow */
    if((fp=fopen(SHADOW_FILE,"r"))==NULL)
    {
	/* Dont show anything */
    }
    else
      {
	    fseek(fp, 0, SEEK_END);
	    file_size = ftell(fp);
	    rewind(fp);
	    check_buffer = allocate(file_size);

	    if( file_size > 1 )
	    while(fgets(check_buffer, file_size, fp)!=NULL)
	    {
		for(x=0; check_buffer[x]; x++)
		{
		    if(check_buffer[x]==':')
		    {
			sprintf(tempname, "%s", check_buffer);
			tempname[x]='\0';
			if( strstr(username, tempname) 
			&& strlen(username) == strlen(tempname) )
			{
			    user_exists = 1;
			    break;
			}
		    }
		    if( check_buffer[x]=='\0' || check_buffer[x]=='\n' )
		    break;
		}
	    }
	    free(check_buffer);
	    fclose(fp);
      }

    return user_exists;
}


int
group_exists(G_CONST_RETURN gchar *groupname)
{
    /* Checks if the system group already exists */
    FILE *fp;
    long file_size;
    int x, group_exists=0;
    char tempname[4096]="";
    char *check_buffer;

    /* Checks if the group exists in group */
    if((fp=fopen(GROUP_FILE,"r"))==NULL)
    {
	/* Dont show anything  */
    }
    else
      {
	    fseek(fp, 0, SEEK_END);
	    file_size = ftell(fp);
	    rewind(fp);
	    check_buffer = allocate(file_size);

	    if( file_size > 1 )
	    while(fgets(check_buffer, file_size, fp)!=NULL)
	    {
		for(x=0; check_buffer[x]; x++)
		{
		    if(check_buffer[x]==':')
		    {
			sprintf(tempname, "%s", check_buffer);
			tempname[x]='\0';
			if( ! strcmp(groupname, tempname) )
			{
			    group_exists = 1;
			    break;
			}
		    }
		    if( check_buffer[x]=='\0' || check_buffer[x]=='\n' )
		    break;
		}
	    }
	    free(check_buffer);
	    fclose(fp);
      }

    /* Checks if the group exists in gshadow */
    if((fp = fopen(GSHADOW_FILE,"r"))==NULL)
    {
	/* Dont show anything */
    }
    else
      {
	    fseek(fp, 0, SEEK_END);
	    file_size = ftell(fp);
	    rewind(fp);
	    check_buffer = allocate(file_size);

	    if( file_size > 1 )
	    while(fgets(check_buffer, file_size, fp)!=NULL)
	    {
		for(x=0; check_buffer[x]; x++)
		{
		    if(check_buffer[x]==':')
		    {
			sprintf(tempname, "%s", check_buffer);
			tempname[x]='\0';
			if( strstr(groupname, tempname) && strlen(groupname)==strlen(tempname) )
			{
			    group_exists = 1;
			    break;
			}
		    }
		    if( check_buffer[x]=='\0' || check_buffer[x]=='\n' )
		    break;
		}
	    }
	    free(check_buffer);
	    fclose(fp);
      }

    return group_exists;
}
