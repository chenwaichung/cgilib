/*
    cgi.h - Ein paar Routinen fuer die Programmierung von CGI-Programmen
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

#ifndef _CGI_H_
#define _CGI_H_

typedef struct var_s {
	char	*name,
		*value;
} c_var;

/*
   cgiParseInput
   liest die Variablen eines POST-Aufrufes und gibt sie zurueck.
 */
c_var **cgiParseInput ();

/*
   cgiGetValue
   gibt den Wert der Variablen zurueck oder NULL.
 */
char *cgiGetValue(c_var **parms, const char *var);

#endif /* _CGI_H_ */
