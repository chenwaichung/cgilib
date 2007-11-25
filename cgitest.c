/*
    cgitest.c - Testprogram for cgi.o
    Copyright (c) 1998,9,2007 by Martin Schulze <joey@infodrom.org>

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

/*
 * Compile with: cc -o cgitest cgitest.c -lcgi
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cgi.h>

s_cgi *cgi;

#define URL "http://www.infodrom.org/projects/cgilib/"

void print_form()
{
    printf ("<h1>Test-Form</h1>\n");
    printf ("<form action=\"/cgi-bin/cgitest/insertdata\" method=post>\n");
    printf ("Input: <input name=string size=50>\n<br>");
    printf ("<select name=select multiple>\n<option>Nr. 1\n<option>Nr. 2\n<option>Nr. 3\n<option>Nr. 4\n</select>\n");
    printf ("Text: <textarea name=text cols=50>\n</textarea>\n");
    printf ("<center><input type=submit value=Submit> ");
    printf ("<input type=reset value=Reset></center>\n");
    printf ("</form>\n");
    printf ("<hr width=50%%><form action=\"/cgi-bin/cgitest/listall\" method=post>\n");
    printf ("Input: <input name=string size=50>\n<br>");
    printf ("<select name=select multiple>\n<option>Nr. 1\n<option>Nr. 2\n<option>Nr. 3\n<option>Nr. 4\n</select>\n");
    printf ("Text: <textarea name=text cols=50>\n</textarea>\n");
    printf ("<center><input type=submit value=Show> ");
    printf ("<input type=reset value=Reset></center>\n");
    printf ("</form>\n");
    printf ("<p><br><p><br><a href=\"/cgi-bin/cgitest/redirect\">Redirect</a><p>\n");
    printf ("<p><br><p><br><a href=\"/cgi-bin/cgitest/listall?var=value&var2=val2;var3=val3\">List Everything</a><p>\n");
    printf ("<p><br><p><br><a href=\"/cgi-bin/cgitest/setcookie\">Set Cookie</a><p>\n");
}

void eval_cgi()
{
    printf ("<h1>Results</h1>\n\n");
    printf ("<b>string</b>: %s<p>\n", cgiGetValue(cgi, "string"));
    printf ("<b>text</b>: %s<p>\n", cgiGetValue(cgi, "text"));
    printf ("<b>select</b>: %s<p>\n", cgiGetValue(cgi, "select"));
}

void listall (char **env)
{
  char **vars;
  char *val;
  char *tmp;
  s_cookie *cookie;
  int i;

  printf ("<h3>Environment Variables</h3>\n<pre>\n");
  for (i=0; env[i]; i++) {
      tmp = cgiEscape (env[i]);
      if (tmp) {
	  printf ("%s\n", tmp);
	  free (tmp);
      }
  }
  
  printf ("</pre>\n<h3>CGI Variables</h3>\n<pre>\n");

  vars = cgiGetVariables (cgi);
  if (vars) {
      for (i=0; vars[i] != NULL; i++) {
	  val = cgiGetValue (cgi, vars[i]);
	  if (val) {
	      tmp = cgiEscape (val);
	      if (tmp) {
		  printf ("%s=%s\n", vars[i], tmp);
		  free (tmp);
	      } else
		  printf ("%s=...\n", vars[i]);
	  } else
	      printf ("%s=\n", vars[i]);
      }
      for (i=0; vars[i] != NULL; i++)
	  free (vars[i]);
  }

  printf ("</pre>\n<h3>Cookies</h3>\n<pre>\n");

  vars = cgiGetCookies (cgi);
  if (vars) {
      for (i=0; vars[i] != NULL; i++) {
	  cookie = cgiGetCookie (cgi, vars[i]);
	  if (cookie) {
	      tmp = cgiEscape (cookie->value);
	      if (tmp) {
		  printf ("%s=%s\n", vars[i], tmp);
		  free (tmp);
	      } else
		  printf ("%s=...\n", vars[i]);
	  } else
	      printf ("%s=\n", vars[i]);
      }
      for (i=0; vars[i] != NULL; i++)
	  free (vars[i]);
  }
  printf ("</pre>\n");
}

int main (int argc, char **argv, char **env)
{
    char *path_info = NULL;

    cgiDebug(0, 0);
    cgi = cgiInit();

    path_info = getenv("PATH_INFO");
    if (path_info) {
	if (!strcmp(path_info, "/redirect")) {
	    cgiRedirect("http://www.infodrom.org/");
	    exit (0);
	} else if (!strcmp(path_info, "/setcookie")) {
	    cgiSetHeader ("Set-Cookie", "Version=1; Library=cgilib; Path=/");
            cgiHeader();
	    printf ("<html>\n<head><title>cgilib</title></title>\n\n<body bgcolor=\"#ffffff\">\n");
	    printf ("<h1><a href=\"%s\">cgilib</a></h1>\n", URL);
	    printf ("<h3>Cookie Library set</h3>\n");
	    printf ("<p><br><p><br><a href=\"/cgi-bin/cgitest\">Test</a><p>\n");
	    printf ("<p><br><p><br><a href=\"/cgi-bin/cgitest/redirect\">Redirect</a><p>\n");
	    printf ("<p><br><p><br><a href=\"/cgi-bin/cgitest/listall\">List Everything</a><p>\n");
	} else if (!strcmp(path_info, "/listall")) {
            cgiHeader();
	    printf ("<html>\n<head><title>cgilib</title></title>\n\n<body bgcolor=\"#ffffff\">\n");
	    printf ("<h1><a href=\"%s\">cgilib</a></h1>\n", URL);
	    listall (env);
	} else {
	    cgiHeader();
	    printf ("<html>\n<head><title>cgilib</title></title>\n\n<body bgcolor=\"#ffffff\">\n");
	    printf ("<h1><a href=\"%s\">cgilib</a></h1>\n", URL);
	    if (strlen (path_info))
		printf ("path_info: %s<br>\n", path_info);
	    if (!strcmp(path_info, "/insertdata")) {
		eval_cgi();
	    } else
		print_form();
	}
    } else {
	cgiHeader();
	printf ("<html>\n<head><title>cgilib</title></title>\n\n<body bgcolor=\"#ffffff\">\n");
	printf ("<h1><a href=\"%s\">cgilib</a></h1>\n", URL);
	print_form();
    }

    printf ("\n<hr>\n</body>\n</html>\n");
    return 0;
}

/*
 * Local variables:
 *  c-indent-level: 4
 *  c-basic-offset: 4
 *  tab-width: 8
 * End:
 */
