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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>
#include <unistd.h>
#include "widgets.h"
#include "allocate.h"
#include "show_info.h"

extern int MAX_READ_POPEN;



void run_command_show_err(gchar *command)
{
    FILE *fp;
    char *line, *info;
    int once = 0;
    
    if((fp=popen(command, "r"))==NULL)
    {
	perror("popen");
	return;
    }
    else
      {
	  line = allocate(MAX_READ_POPEN+2);
	  info = allocate(MAX_READ_POPEN+2);
	  while(fgets(line, MAX_READ_POPEN, fp)!=NULL)
	  {
	     strcat(info, line);
	     /* Show the error line a bit more clearly */
	     if( strstr(line, "visit") && ! once )
	     {
	        once = 1;
		strcat(info, "\n");
	     }
	  }
	  pclose(fp);
	  show_info(info);
	  free(info);
	  free(line);
      }
}


int run_command(gchar *command)
{
    FILE *fp;
    int status=0, exit_status=0;
    
    if((fp=popen(command, "w"))==NULL)
    {
	perror("popen");
	return 0;
    }
    status = pclose(fp);

    exit_status = WEXITSTATUS(status);
    
    if( exit_status == 1)
      exit_status = 0;
    else
      exit_status = 1;

    return exit_status;
}


int file_exists(char *infile)
{
    FILE *fp;
    if((fp=fopen(infile, "r"))==NULL)
      return 0;

    fclose(fp);
    return 1;
}
