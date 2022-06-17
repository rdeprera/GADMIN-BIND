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
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "allocate.h"
#include "reverse_address.h"
#include "ip_handling.h"

#define MAX_ADDR_LEN 1000



char * reverse_address(char *addr)
{
    char *reverse, *temp, *dot1, *dot2, *dot3, *dot4;
    int num_dots = 0;
    int mask = 0;
    int i    = 0;
    char *rev;
    int rev_length = 0;

    /* Reverse IPv6 address x.x.x.x.x.x.x.x.ip6.arpa */
    if( strstr(addr, ".ip6.arpa") )
    {
	reverse = reverse_to_forward(addr);
	return reverse;
    }

    reverse = allocate(MAX_ADDR_LEN+3);
    dot1  = allocate(MAX_ADDR_LEN);
    dot2  = allocate(MAX_ADDR_LEN);
    dot3  = allocate(MAX_ADDR_LEN);
    dot4  = allocate(MAX_ADDR_LEN);
    temp  = allocate(MAX_ADDR_LEN);

    /* IPv4 - Add a dot in the beginning and one at the end before strtok */
    if( strstr(addr, ".") && ! strstr(addr, ":") && ! strstr(addr, "ip6.arpa") )
    {
	snprintf(reverse, MAX_ADDR_LEN-2, ".%s.", addr);
	temp = strtok(reverse, ".");
	while( temp != NULL )
	{
	    num_dots++;
	    if( num_dots == 1 )
	      snprintf(dot1, MAX_ADDR_LEN-1, "%s", temp);
	    if( num_dots == 2 )
	      snprintf(dot2, MAX_ADDR_LEN-1, "%s", temp);
	    if( num_dots == 3 )
	      snprintf(dot3, MAX_ADDR_LEN-1, "%s", temp);
	    if( num_dots == 4 )
	      snprintf(dot4, MAX_ADDR_LEN-1, "%s", temp);
	    /* We pass NULL to get the next token */	
	    temp = strtok(NULL, ".");
	}

	if( num_dots == 1  )
    	    snprintf(reverse, MAX_ADDR_LEN-1, "%s", dot1);

	if( num_dots == 2 )
    	    snprintf(reverse, MAX_ADDR_LEN-1, "%s.%s", dot2, dot1);

	if( num_dots == 3 )
    	    snprintf(reverse, MAX_ADDR_LEN-1, "%s.%s.%s", dot3, dot2, dot1);

	if( num_dots == 4 )
    	    snprintf(reverse, MAX_ADDR_LEN-1, "%s.%s.%s.%s", dot4, dot3, dot2, dot1);

	free(dot1); free(dot2); free(dot3); free(dot4);
    }
    else
    if( strstr(addr, ":") )
    {
	/* IPv6 - Get the network address */
	snprintf(temp, MAX_ADDR_LEN-1, "%s", addr);

	if( ! strstr(temp, "/") )
	{
//	    printf("Reverse_address.c: Error, no slash found.\n");
//            printf("Missing network mask, IE: fe80::213:8fff:feea:9b79/64 <-\n\n");
	    snprintf(reverse, MAX_ADDR_LEN-1, "%s", "ADDRESS_ERROR");
	}
    
        /* Split address and mask */
        for(i=0; temp[i+1]!='\0'; i++) // added +1
        if( temp[i]=='/')
        {
            /* 3 == max 128 mask */
            mask = atoi(&temp[i+1]);
            temp[i]='\0';
        }

        /* Build the host part of the IPv6 address */
        rev = build_reverse_ip((char *)temp, mask, 0, 0); // IPv4 Range: start, end);
        if( rev )
        {
//            printf("Reverse host part IPv6 address: %s\n", rev);
	    
	    // Here we must pass mask 0 to get the entire host address.
	    // Then cut away the length of the host part
	    rev_length = strlen(rev);
            free(rev);

	    /* Build the entire ip of this host with mask 0 */
    	    rev = build_reverse_ip((char *)temp, 0, 0, 0); // IPv4 Range: start, end);
    	    if( rev )
    	    {
		// Snip ".ip6.arpa" at the end.
		rev[strlen(rev)-9]='\0';

		// Reverse zone network address is the right part.
		snprintf(reverse, 1000, "%s", &rev[rev_length+1]);

//        	printf("Reverse network part IPv6 address: %s\n", reverse);
		free(rev);    		
    	    }
    	    else
    	    {
//    	        printf("Reverse_address.c: Error, Build network address failed.\n");
    	    	snprintf(reverse, MAX_ADDR_LEN-1, "%s", "ADDRESS_ERROR");
    	    }
        }
        else
	{
//            printf("Building reverse host part IPv6 address failed.\n");
	    snprintf(reverse, MAX_ADDR_LEN-1, "%s", "ADDRESS_ERROR");
        }
    }
    else
    {
	printf("This is not a valid ipv4 or ipv6 address\n");
	snprintf(reverse, MAX_ADDR_LEN-1, "%s", "ADDRESS_ERROR");
    }

    free(temp);

    return reverse;
}


/* Converts a forward IP address to a PTR host part address */
char * get_ptr_addr_from_forward_address(char *forward_address)
{
    gchar **split_addr, **p;
    gchar *last_octets=NULL;
    char *address, *rev;
    int i, mask = 0;
    int is_ipv4 = 0;
    int is_ipv6 = 0;

    if( forward_address==NULL || strlen(forward_address) < 5 )
      return NULL;

    if( strstr(forward_address, ":") )
      is_ipv6 = 1;
    else
      is_ipv4 = 1;

    address = allocate(1024);
    snprintf(address, 1000, "%s", forward_address);


    /* Get the host part for an IPv4 address.
       This is used in the reverse IPv4 zone like this:
       50 IN PTR dns1.example.org */

    if( is_ipv4 )
    {
	p = split_addr = g_strsplit(address, ".", 0); /* 0 = split all tokens */
	while( *p )
	{
	    if( last_octets!=NULL )
              g_free(last_octets);

            last_octets = g_strdup_printf("%s", *p);
            p++;
	}
	g_strfreev(split_addr);

	if( last_octets == NULL )
	{
	    printf("Adress splitting error.\n");
	    
	    if( last_octets!=NULL )
              g_free(last_octets);

            free(address);
	    return NULL;
	}
    }

    /* Get the host part from an IPv6 address with mask.
       This is used in the reverse IPv6 zone like this:
       9.7.b.9.a.e.e.f.f.f.f.8.3.1.2.0 IN PTR dns1.example.org */

    if( is_ipv6 )
    {
        if( ! strstr(address, "/") )
        {
            mask = 0; // Doesnt matter if theres no mask
        }

        /* Split address and mask if any */
        for(i=0; address[i]!='\0'; i++)
	if( address[i]=='/')
        {
            /* 3 == max 128 mask */
            mask = atoi(&address[i+1]);
            address[i]='\0';
            break;
        }

        /* Build the host part of the IPv6 address */
        rev = build_reverse_ip((char *)address, mask, 0, 0); // IPv4 Range: start, end);
        if( rev )
        {
            printf("Reverse host part IPv6 address: %s\n", rev);
            last_octets = g_strdup_printf("%s", rev);
            free(rev);
        }
        else
        {
            printf("Building reverse host part IPv6 address failed.\n");
	    free(address);
	    return NULL;
	}
    }

    snprintf(address, 1000, "%s", last_octets);

    if( last_octets!=NULL )
      g_free(last_octets);

    return address;
}
