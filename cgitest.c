/*
    cgitest.c - Testprogram for cgi.o
    Copyright (c) 1998  Martin Schulze <joey@infodrom.north.de>

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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111, USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <cgi.h>

s_cgi **cgi;

void print_form()
{
    printf ("<h1>Test-Form</h1>\n");
    printf ("<form action=\"/cgi-bin/cgitest/insertdata\" method=post>\n");
    printf ("Input: <input name=string size=50>\n<br>");
    printf ("Text: <textarea name=text cols=50>\n</textarea>\n");
    printf ("<center><input type=submit value=Submit> ");
    printf ("<input type=reset value=Reset></center>\n");
    printf ("</form>\n");
}

void eval_cgi()
{
    printf ("<h1>Results</h1>\n\n");
    printf ("<b>string</b>: %s<p>\n", cgiGetValue(cgi, "string"));
    printf ("<b>text</b>: %s<p>\n", cgiGetValue(cgi, "text"));
}


void main ()
{
    char *path_info = NULL;

    cgiDebug(1, 0);
    cgi = cgiInit();
    cgiHeader();
    printf ("<html>\n<head><title>cgilib</title></title>\n\n<body>\n");
    printf ("<h1>cgilib</h1>\n");

    path_info = getenv("PATH_INFO");
    printf ("path_info: %s<br>\n", path_info);
    if (path_info) {
	if (!strcmp(path_info, "/insertdata")) {
	    eval_cgi();
	} else
	    print_form();
    } else
	print_form();

    printf ("\n<hr>\n</body>\n</html>\n");
}

/*
 * Local variables:
 *  c-indent-level: 4
 *  c-basic-offset: 4
 *  tab-width: 8
 * End:
 */
