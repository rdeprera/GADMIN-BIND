/* GADMIN-BIND - An easy to use GTK+ frontend for the ISC BIND DNS Server.
 * Copyright (C) 2008 - 2010 Magnus Loef <magnus-swe@telia.com> and
 * Jean-Jacques Sarton <jj.sarton@t-online.de>
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

/*************************************************************************
 *
 * Handling of IP addresses
 *
 * Content:
 *   check_ip()           check if the given IPv[46] is in a valid format
 *   build_reverse_ip()   build the reverse ip according to arguments
 *   reverse_to_forward() construct the (partial) ip for the given
 *                        reverse ip address *.in-addr.arpa or *.ip6.arpa
 *
 *************************************************************************
*/

 
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <stdlib.h>
#include <ctype.h>

#include "ip_handling.h"

#ifndef BUF_SIZE
# define BUF_SIZE 1024
#endif

#define IPV6_SIZE 40 /* (4+1)*8) */

#define REV_SIZE 73  /* ((128/4)*2) + 8 + 1 */

extern char *build_reverse_ip(char *addr, int mask, int start, int end);
extern int   check_ip(char *ip, int range);
extern char *reverse_to_forward(char *ip);
 
 
/*************************************************
 *
 * Function zone_from_range()
 *
 * input:  char *range ef 1.2.3.0/24
 *
 * Return: NULL on error, reverse zone name if OK
 *
 ************************************************/

char *zone_from_range(char *range)
{
   int start = 0;
   int end   = 0;
   int bits  = 0;
   
   if ( set_range(range, &start, &end, &bits) == 1 )
   {
      return build_reverse_ip(range, -bits, start, end);
   }
   return NULL;
}
 
/*************************************************
 *
 * Function expandIPv6()
 *
 * input:  char *ipi the IPv4 or IPv6 Address
 *
 * Output: char *ipo pointer to a buffer with a size
 *                  of at least IPV6_SIZE bytes
 *
 * Return: 0 on error, 1 if all is OK
 ************************************************/
 
static int expandIPv6(char *ipi, char *ipo)
{
   char *s, *n, *p;
   unsigned int a[8] = { -1,-1,-1,-1,-1,-1,-1,-1};
   int seg = 0;
   long v;
   int i = 0;
   int hasIPv4 = strchr(ipi, '.')?1:0;

   memset(ipo, 0, IPV6_SIZE);

    if ( ipi == NULL ) return 0;
    if ( !isxdigit(*ipi) )
    {
       if ( *ipi != ':' )
       {
         return 0;
       }
    }

    s = ipi;
    seg = 0;

    do
    {
       if ( *s == ':' )
       {
          s++;
          break;
       }
       v = strtol(s, &n, 16);
       if ( n && *n == '.' )
          break;
       else
          a[seg++] = v;
          
       if ( n && *n == ':')
          n++;
       s = n;
       
       if ( s && *s == ':' )
          break;
    } while ( seg < 8 );

    if ( seg < 8 )
    {
       if ( s && *s == ':' )
       {
          /* a lot of 0 */
          s++;
          
          i = 0;
          n = p = s;
          while ( (n = strchr(p, ':')) )
          {
             n++;
             p = n;
             i++;
          }
          if ( p && isxdigit(*p) && ! hasIPv4 )
          {
             i++;
          }
       }

       /* fill fields with 0 */
       while ( seg < 8 - i - hasIPv4?2:0)
       {
          a[seg++] = 0;
       }

       while ( seg < 8 - hasIPv4?2:0 )
       {
           a[seg++] = strtol(s, &n, 16);
           if ( n && *n == ':' )
              s = n+1;
           else
             break;
       }
    }

   /* ipv6 with ipv4 part at the end */
   s = strchr(ipi, '.');
   if ( s )
   {
       int x1, x2, x3, x4;
       while ( s > ipi && *s != ':')
          s--;
       if ( sscanf(s, ":%d.%d.%d.%d", &x1,&x2,&x3,&x4) != 4 )
       {
           return 0;
       }
       a[6] = x1 << 8 |x2;
       a[7] = x3 << 8 |x4;
    }


    /* end check */
    for (i=0; i < 8; i++ )
    {
       if ( a[i] < 0 || a[i] > 0xffff )
          return 0;
    }
    /* all is OK */
    sprintf(ipo, "%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x",
            a[0],a[1],a[2],a[3],a[4],a[5],a[6],a[7]);
    return 1;
}

/*************************************************
 *
 * Function check_ip()
 *
 * input:  char *ip  the IPv4 or IPv6 Address
 *                   to check
 *
 *         int range if set don'ckeck the last
 *                   ipv4 address part
 *
 * Return: 0 on error, Version 5/6 for valid IP
 ************************************************/
 
int check_ip(char *ip, int range)
{
   char  ipv6[IPV6_SIZE];
   int   ver = 0;

   if ( strchr(ip, ':') )
   {
      /* IPv6 */
      /* allowed chars 0-9, 1-f ':' and possibly at the end
       * IPv4 like
       */
      ver = 6;
      if ( !expandIPv6(ip, ipv6) )
      {
         return 0;
      }
   }
   else
   {
      /* iPv4 Check */
      int a,b,c,d;
      a = b = c = 0;
      d = 1;
      ver = 4;
      if ( !range && sscanf(ip, "%d.%d.%d.%d", &a,&b,&c,&d) != 4 )
      {
          return 0;
      }
      else if ( range && sscanf(ip, "%d.%d.%d", &a,&b,&c) < 1 )
      {
         return 0;
      }
      if ( a > 255 || a < 1 ) return 0;
      if ( b > 255 || b < 0 ) return 0;
      if ( c > 255 || c < 0 ) return 0;
      if ( d > 255 || d < 1 ) return 0;
      /* we nust have 3 dots check that */
      d = 0;
      if ( !range )
      {
         while(*ip)
         {
            if ( *ip == '.' )
               d++;
            ip++;
         }
         if ( d != 3 )
            ver = 0;
      }
   }
   
   return ver; /* a valid IP */
}

/*************************************************
 *
 * Function buildRevAddr()
 *
 * Input:  char *ip    the IPv4 or IPv6 Address
 *                     to print out
 * Output  char *rev   pointer to buffer of length
 *                     REV_SIZE
 *
 * Return: -
 *
 ************************************************/

static void buildRevAddr(char *ip, char *rev)
{
   int a,b,c,d;
   char ipv6[IPV6_SIZE];
   a=b=c=d=0;
   if ( strchr(ip, ':' ) == NULL )
   {
      sscanf(ip, "%d.%d.%d.%d", &a,&b,&c,&d);
      sprintf(rev, "%d.%d.%d.%d.in-addr.arpa",d,c,b,a);
   }
   else
   {
      expandIPv6(ip, ipv6);
      for(a=IPV6_SIZE-2, b=0; a >=0; a-- )
      {
          if ( ipv6[a] != ':' )
          {
             rev[b++] = ipv6[a];
             rev[b++] = '.';
             rev[b]   = '\0';
          }
      }
      strcat(rev,"ip6.arpa");
   }
}

/*************************************************
 *
 * Function build_reverse_ip()
 *
 * Input:  char *addr  the IPv4 or IPv6 Address
 *                     which is to be transformed
 *
 *         int mask    for building the zone name/
 *                     entry part of the address
 *                     if < 0 generate zone name
 *                     if > 0 gerenare entry
 *                     if   0 generate full reverse
 *                            IP
 *
 *         int start \ If both are non 0 generate
 *         int end   / a reverse zone name for range
 *
 *
 * Return: allocated char buffer to be freed by
 *         caller
 *
 ************************************************/

char *build_reverse_ip(char *addr, int mask, int start, int end)
{
   char *s;
   int   pos;
   char  rev[REV_SIZE];
   char  obuf[REV_SIZE];

   /* build full reverse entry */
   buildRevAddr(addr, rev);
   
   *obuf = '\0';

   if ( mask == 0 )
   {
      return strdup(rev);
   }
   else if ( strstr(rev, "ip6" ) == NULL )
   {
      pos = mask / 8;
      if ( pos < 0 )
         pos = -pos;
      s = rev;

      while( pos < 4 )
      {
         if ( *s == '.')
         {
            pos++;
         }
         s++;
      }

      if ( mask < 0 )
      {
         if ( start && end )
         {
            if ( start > end || start < 1 || start > 254
                 || end > 2554 
               )
            {
               return NULL;
            }
            snprintf(obuf,REV_SIZE ,"%d-%d.%s",start,end, s);
         }
         else
         {
            snprintf(obuf,REV_SIZE ,"%s",s);
         }
      }
      else
      {
          s--;
          *s = '\0';
          snprintf(obuf,REV_SIZE, "%s",rev);
          *s = '.';
      }
   }
   else /* IPv6 Address */
   {
      if ( mask > 0 )
      {
         pos = (mask / 2) - 1;
      }
      else
      {
         pos = (-mask / 2) - 1;
      }

      if ( mask < 0 )
      {
         snprintf(obuf,REV_SIZE,"%s",rev+pos+1);
      }
      else
      {
         rev[pos] = '\0';
         snprintf(obuf,REV_SIZE,"%s",rev);
         rev[pos] = '.';
      }
   }
   if ( *obuf )
   {
      return strdup(obuf);
   }
   else
   {
      return NULL;
   }
}

/*************************************************
 *
 * Function reverseToForward()
 *
 * input:  char *rip  reverse zone address
 *
 * Output: char *ip  pointer to a buffer with a size
 *                  of a least IPV6_SIZE bytes
 *
 * Return: 0 on error, 1 if all is OK
 ************************************************/

static int reverseToForward(char *rip, char *ip)
{
   char *s;
   char *t = ip;
   long  val;
   unsigned int  part[9] = { 0,0,0,0, 0,0,0,0 };
   int   colons;
   int   cnt;
   int   first  = -1;
   int   second = -1;

   if ( (s = strstr(rip, ".ip6.arpa")) )
   {
      s--;
      cnt    = 0;
      val    = 0;
      colons = 0;
      
      /* preliminary checks */
      if ( strlen(rip) > 72 )
      {
         return 0;
      }

      /* read all values */
      while (s >= rip )
      {
         if ( isxdigit(*s) )
         {
            if ( isdigit(*s) )
            {
               val = (val << 4) + (*s - '0' );
            }
            else if ( *s >= 'A' && *s <= 'F' )
            {
               val = (val << 4) + (*s - 'A' + 10 );
            }
            else
            {
               val = (val << 4) + (*s - 'a' + 10 );
            }
         }
         if ( *s == '.' || s == rip )
         {
            cnt++;
            if ( s > rip && s[-1] == '.' )
            {
               /* ".." -> error */
               return 0;
            }
            if ( cnt == 4 )
            {
                  part[colons] = val;
                  colons++;
                  if ( colons > 8 )
                  {
                     return 0;
                  }
               val = 0;
               cnt = 0;
            }
         }

         /* unallowed characters -> error */
         if ( ! (isxdigit(*s) || *s == '.') )
         {
            return 0;
         }
         s--;
      }
      
      /* look for the longest count of consecutive 0 */
      colons = 0;
      val    = 0;
      first  = -1;
      second = -1;
      for ( val = 0; val < 8; )
      {
         if ( part[val] == 0 )
         {
            first = val;
            cnt=1;
            val++;
            for ( ; val < 8; )
            {
               if ( part[val] == 0 )
               {
                  cnt++;
               }
               if ( cnt > colons )
               {
                  colons = cnt;
               }
               if ( part[val] )
               {
                  val++;
                  break;
               }
               val++;
            }
            break;
         }
         else
         {
            if ( first > -1 )
            {
               val++;
               break;
            }
         }
         val++;
      }

      /* if there is a second area of 0, this take precedence */
      for ( cnt=0; val < 8; )
      {
         if ( part[val] == 0 )
         {
            if ( second == -1 )
            {
               second = val;
               cnt=1;
               val++;
               for ( ; val < 8; )
               {
                  if ( part[val] == 0 )
                  {
                     cnt++;
                  }
                  if ( cnt >= colons )
                  {
                     colons = cnt;
                     if ( second )
                     {
                        first = second;
                     }
                  }
                  if ( part[val] )
                     break;
                  val++;
               }
               break;
            }
         }
         else if ( second > 0 )
         {
            break;
         }
         val++;
      }

      /* print out */
      if ( first == 0)
      {
         *t++ = ':';
         if ( colons == 8 )
         {
            *t++ = ':';
            *t++ = '\0';
         }
         for ( val = colons; val < 8; val++ )
         {
            sprintf(t, ":%x",part[val]);
            t += strlen(t);
         }
      }
      else if ( first > 0 )
      {
         sprintf(t, "%x",part[0]);
         t += strlen(t);
         if ( 1 + colons == 8 )
            *t++ = ':';
         for ( val = 1; val < first; val++ )
         {
            sprintf(t, ":%x",part[val]);
            t += strlen(t);
            
         }
         *t++=':';
         if ( first + colons == 8 )
            *t++ = ':';
         
         for ( val += colons ; val < 8; val++ )
         {
            sprintf(t, ":%x",part[val]);
            t += strlen(t);
            
         }
      }
      else
      {
         sprintf(t, "%x",part[0]);
         t += strlen(t);
         for ( val = 1; val < 8; val++ )
         {
            sprintf(t, ":%x",part[val]);
            t += strlen(t);
         }
      }
      *t = '\0';
      return 1;
   }
   else if ( (s = strstr(rip, ".in-addr.arpa")) )
   {
      val    = 0;
      cnt    = 1;
      colons = 0;
      s--;
      if ( *s == '.' )
         return 0;
      while ( s >= rip )
      {
         if ( isdigit(*s) )
         {
            val += (*s - '0') * cnt;
            cnt *= 10;
         }
         if ( *s == '.' || s == rip )
         {
            if ( val > 255 || val < 0 )
            {
               return 0;
            }
            else
            {
               if ( t != ip )
               {
                  *t++ = '.';
                  colons++;
                  if ( colons > 3 )
                     return 0;
               }
               sprintf(t,"%d",(int)val);
               t += strlen(t);
               val = 0;
               cnt = 1;
            }
            if ( s > rip && s[-1] == '.' )
               return 0; 
         }
         s--;
      }
      return 1;
   }
   return 0;
}

/*************************************************
 *
 * Function set_range()
 *
 * input:  char *ipr  iprange eg 10.1.1.1-254
 *                            or 192.168
 *                            or 192.168/16
 *                            or 2001:2:3:4/64
 *
 * output: int  *start       start of range if appl.
 *         int  *end         end of range if appl.
 *                             
 *         int  *bits        bit mask if applicable
 *
 * Return: 1 if OK -1 on error else 0 ( not range
 *                                      syntax)
 ************************************************/

int set_range(char *ipr, int *start, int *end, int *bits)
{
   char *s;
   int   cnt;
   int   val;

   if ( (s = strchr(ipr, '/')) )
   {
      *bits = atoi(s+1);
      if ( strstr(ipr, ":") )
      {
         s -=2;
         if ( s < ipr )
            return -1;
         if ( *s++  != ':' )
            return -1;
         if ( *s != ':' )
            return -1;
         if ( *bits & 15 || *bits < 16 || *bits > 112 )
            return -1;
         return 1;
      }
      else
      {
         if (*bits < 8 || *bits > 24 )
            return -1;
         s   = ipr;
         cnt = 1;
         do
         {
            s = strchr(s, '.');
            if ( s )
            {
               cnt++;
               s++;
            }            
         } while(s && *s != '/');
         if ( cnt != 4 )
           return -1;
         return 1;
      }
   }
   else if ( (s = strchr(ipr, '-')) )
   {
      if ( strchr(ipr, ':') )
          return -1;

      *end = atoi(s+1);
      if ( *end > 254 || *end < 1)
         return -1;
      while ( s > ipr && *s != '.' )
         s--;
      if ( s == ipr )
         return -1;
      *start = atoi(s+1);
      if ( *start >= *end || *start > 254 || *start < 1)
         return .1;
      s = ipr;
      cnt = 0;
      do
      {
         s = strchr(s, '.');
         if ( s )
         {
            cnt++;
            s++;
         }
      } while(s);
      if ( cnt != 3 )
         return -1;
      *bits = 24;
      return 1;
   }
   else
   {
      if ( strchr(ipr, ':') )
         return -1;
      cnt = 32;
      s = ipr;
      do
      {
         val = strtol(s, &s, 10);
         if ( cnt == 32 && (val < 1 || val > 254) )
            return -1;
         else if ( val < 0 || val > 255 )
            return -1;
         cnt -= 8;
         if ( *s == '.' )
            s++;
         else if ( *s )
            return -1;
      } while(*s);
      if ( cnt < 0 )
         return -1;
      *bits = cnt;
      if ( cnt )
         return 1;
   }
   return 0;
}

/*************************************************
 *
 * Function reverse_to_forward()
 *
 * input:  char *ip  reverse zone address
 *
 * Return:allocated buffer with content or NULL
 ************************************************/

char *reverse_to_forward(char *ip)
{
   char  rev[REV_SIZE];
   if ( reverseToForward(ip, rev) )
      return strdup(rev);
   else
      return NULL;
}
