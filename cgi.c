/*
    cgi.c - Ein paar Routinen fuer die Programmierung von CGI-Programmen
    Copyright (c) 1996  Martin Schulze <joey@office.individual.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cgi.h"

c_var *cgiParseVar (char *binding)
{
  c_var *result;
  char *ip;

  if ((ip = (char *)index(binding, '=')) == NULL)
    return NULL;
  if (((result = (c_var *)malloc(sizeof(c_var))) == NULL)
      ||((result->name = (char *)malloc(ip - binding +1)) == NULL)
      ||((result->value = (char *)malloc(strlen(ip))) == NULL))
    return NULL;

  bzero(result->name,sizeof(result->name));
  bzero(result->value,sizeof(result->value));
  strncpy (result->name, binding, ip++ - binding);
  strcpy (result->value, ip);

  return result;
}

c_var **cgiParseInput ()
{
  int length;
  char *line;
  char frag[50];
  int numargs;
  char *cp, *ip;
  c_var **result;
  int i;

  if (!getenv("CONTENT_LENGTH"))
    return NULL;
  length = atoi(getenv("CONTENT_LENGTH"));
  if ((line = (char *)malloc (length+2)) == NULL)
    return NULL;

  fgets(line, length+1, stdin);

  /* line contains all values stored like okz=0923&plz=23 */
  for (numargs=1,cp=line; *cp; cp++)
    if (*cp == '&') numargs++;
  if ((result = (c_var **)malloc((numargs * sizeof(c_var *))+1)) == NULL)
    return NULL;

  cp = line;
  i=0;
  while ((ip = (char *)index(cp, '&')) != NULL) {
    bzero (frag, sizeof(frag));
    strncpy (frag, cp, ip - cp);
    result[i++] = cgiParseVar(frag);
    cp = ++ip;
  }
  result[i++] = cgiParseVar(cp);
  result[i] = NULL;

/*
  for (i=0;result[i]; i++)
    printf ("%s=%s<br>\n", result[i]->name,result[i]->value);
*/

  return result;
}

char *cgiGetValue(c_var **parms, const char *var)
{
  int i;

  if (parms)
    for (i=0;parms[i]; i++)
      if (!strcmp(var,parms[i]->name))
	return parms[i]->value;
  return NULL;
}
