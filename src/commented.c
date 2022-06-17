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



#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "commented.h"


int commented(char *line)
{
    int i, ret = 0;

    if( line!=NULL && strlen(line) > 0 )
    {
	for(i=0; line[i]!='\0'; i++)
	{
	    if( i > 9000 )
	    {
		ret = 1;
		break;
	    }

	    /* Skip whitespace */
	    if( line[i]==' ' )
	      continue;
	    
	    /* If the first char thats not whitespace is '#' or ';'
	       then the line is commented out */
	    if( line[i]=='#' || line[i]==';' )
	      ret = 1;
	    else
	      ret = 0;
	
	    break;
	}
    }

    return ret;
}
