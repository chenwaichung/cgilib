/*
    cgi.c - Some simple routines for CGI programming
    Copyright (c) 1996-9  Martin Schulze <joey@infodrom.north.de>

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
    Sat Aug 24 10:39:31 MET 1999: Martin Schulze <joey@infodrom.north.de>
	Added cgiGetVariables(), corrected multiple values code
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <malloc.h>
#include <cgi.h>

int cgiDebugLevel = 0;
int cgiDebugStderr = 1;

void cgiHeader ()
{
    printf ("Content-type: text/html\n\n");
}

void cgiDebug (int level, int where)
{
    if (level > 0)
	cgiDebugLevel = level;
    else
	cgiDebugLevel = 0;
    if (where)
	cgiDebugStderr = 0;
    else
	cgiDebugStderr = 1;
}

char *cgiDecodeString (char *text)
{
    char *cp, *xp;

    for (cp=text,xp=text; *cp; cp++) {
	if (*cp == '%') {
	    if (strchr("0123456789ABCDEFabcdef", *(cp+1))
		&& strchr("0123456789ABCDEFabcdef", *(cp+2))) {
		if (islower(*(cp+1)))
		    *(cp+1) = toupper(*(cp+1));
		if (islower(*(cp+2)))
		    *(cp+2) = toupper(*(cp+2));
		*(xp) = (*(cp+1) >= 'A' ? *(cp+1) - 'A' + 10 : *(cp+1) - '0' ) * 16
		    + (*(cp+2) >= 'A' ? *(cp+2) - 'A' + 10 : *(cp+2) - '0');
		xp++;cp+=2;
	    }
	} else {
	    *(xp++) = *cp;
	}
    }
    memset(xp, 0, cp-xp);
    return text;
}

/*  cgiInit()
 *
 *  Read from stdin if no string is provided via CGI.  Variables that
 *  doesn't have a value associated with it doesn't get stored.
 */
s_cgi **cgiInit ()
{
    int length;
    char *line = NULL;
    int numargs;
    char *cp, *ip, *esp, *sptr;
    s_cgi **result;
    int i, k, len;
    char tmp[101];

    cp = getenv("REQUEST_METHOD");
    ip = getenv("CONTENT_LENGTH");

    if (cp && !strcmp(cp, "POST")) {
	if (ip) {
	    length = atoi(ip);
	    if ((line = (char *)malloc (length+2)) == NULL)
		return NULL;
	    fgets(line, length+1, stdin);
	} else
	    return NULL;
    } else if (cp && !strcmp(cp, "GET")) {
	esp = getenv("QUERY_STRING");
	if (esp && strlen(esp)) {
	    if ((line = (char *)malloc (strlen(esp)+2)) == NULL)
		return NULL;
	    sprintf (line, "%s", esp);
	} else
	    return NULL;
    } else {
        length = 0;
	printf ("(offline mode: enter name=value pairs on standard input)\n");
	memset (tmp, 0, sizeof(tmp));
	while((cp = fgets (tmp, 100, stdin)) != NULL) {
	    if (strlen(tmp)) {
		if (tmp[strlen(tmp)-1] == '\n')
		    tmp[strlen(tmp)-1] = '&';
		if (length) {
		    length += strlen(tmp);
		    len = (length+1) * sizeof(char);
		    if ((line = (char *)realloc (line, len)) == NULL)
		        return NULL;
		    strcat (line, tmp);
		} else {
		    length = strlen(tmp);
		    len = (length+1) * sizeof(char);
		    if ((line = (char *)malloc (len)) == NULL)
		        return NULL;
		    memset (line, 0, len);
		    strcpy (line, tmp);
		}
	    }
	    memset (tmp, 0, sizeof(tmp));
	}
	if (!line)
	    return NULL;
	if (line[strlen(line)-1] == '&')
	    line[strlen(line)-1] = '\0';
    }

    /*
     *  From now on all cgi variables are stored in the variable line
     *  and look like  foo=bar&foobar=barfoo&foofoo=
     */

    if (cgiDebugLevel > 0) {
	if (cgiDebugStderr)
	    fprintf (stderr, "Received cgi input: %s\n", line);
	else
	    printf ("<b>Received cgi input</b><br>\n<pre>\n--\n%s\n--\n</pre>\n\n", line);
    }

    for (cp=line; *cp; cp++)
	if (*cp == '+')
	    *cp = ' ';

    if (strlen(line)) {
	for (numargs=1,cp=line; *cp; cp++)
	    if (*cp == '&') numargs++;
    } else
	numargs = 0;
    if (cgiDebugLevel > 0) {
	if (cgiDebugStderr)
	    fprintf (stderr, "%d cgi variables found.\n", numargs);
	else
	    printf ("%d cgi variables found.<br>\n", numargs);
    }

    len = (numargs+1) * sizeof(s_cgi *);
    if ((result = (s_cgi **)malloc (len)) == NULL)
	return NULL;
    memset (result, 0, len);

    cp = line;
    i=0;
    while (*cp) {
	if ((ip = (char *)strchr(cp, '&')) != NULL) {
	    *ip = '\0';
	}else
	    ip = cp + strlen(cp);

	if ((esp=(char *)strchr(cp, '=')) == NULL) {
	    cp = ++ip;
	    continue;
	}

	if (!strlen(esp)) {
	    cp = ++ip;
	    continue;
	}

	if (i<numargs) {

	    for (k=0; k<i && (strncmp(result[k]->name,cp, esp-cp)); k++);
	    /* try to find out if there's already such a variable */
	    if (k == i) {	/* No such variable yet */
		if ((result[i] = (s_cgi *)malloc(sizeof(s_cgi))) == NULL)
		    return NULL;
		if ((result[i]->name = (char *)malloc((esp-cp+1) * sizeof(char))) == NULL)
		    return NULL;
		memset (result[i]->name, 0, esp-cp+1);
		strncpy(result[i]->name, cp, esp-cp);
		cp = ++esp;
		if ((result[i]->value = (char *)malloc((ip-esp+1) * sizeof(char))) == NULL)
		    return NULL;
		memset (result[i]->value, 0, ip-esp+1);
		strncpy(result[i]->value, cp, ip-esp);
		result[i]->value = cgiDecodeString(result[i]->value);
		if (cgiDebugLevel) {
		    if (cgiDebugStderr)
			fprintf (stderr, "%s: %s\n", result[i]->name, result[i]->value);
		    else
			printf ("<h3>Variable %s</h3>\n<pre>\n%s\n</pre>\n\n", result[i]->name, result[i]->value);
		}
		i++;
	    } else {	/* There is already such a name, suppose a mutiple field */
		cp = ++esp;
		len = (strlen(result[k]->value)+(ip-esp)+2) * sizeof (char);
		if ((sptr = (char *)malloc(len)) == NULL)
		    return NULL;
		memset (sptr, 0, len);
		sprintf (sptr, "%s\n", result[k]->value);
		strncat(sptr, cp, ip-esp);
		free(result[k]->value);
		result[k]->value = sptr;
	    }
	}
	cp = ++ip;
    }
    return result;
}

char *cgiGetValue (s_cgi **parms, const char *var)
{
    int i;

    if (parms)
	for (i=0;parms[i]; i++)
	    if (!strcmp(var,parms[i]->name)) {
		if (cgiDebugLevel > 0) {
		    if (cgiDebugStderr)
			fprintf (stderr, "%s found as %s\n", var, parms[i]->value);
		    else
			printf ("%s found as %s<br>\n", var, parms[i]->value);
		}
		return parms[i]->value;
	    }
    if (cgiDebugLevel) {
	if (cgiDebugStderr)
	    fprintf (stderr, "%s not found\n", var);
	else
	    printf ("%s not found<br>\n", var);
    }
    return NULL;
}

char **cgiGetVariables (s_cgi **parms)
{
    int i;
    char **res = NULL;
    int len;

    if (parms) {
	for (i=0;parms[i]; i++);
	len = sizeof (char *) * ++i;
	if ((res = (char **)malloc (len)) == NULL)
	    return NULL;
	memset (res, 0, len);
	for (i=0;parms[i]; i++) {
	    len = strlen (parms[i]->name) +1;
	    if ((res[i] = (char *)malloc (len)) == NULL)
		return NULL;
	    memset (res[i], 0, len);
	    strcpy (res[i], parms[i]->name);
	}
    }
    return res;
}

void cgiRedirect (const char *url)
{
    if (url && strlen(url)) {
	printf ("Content-type: text/html\nContent-length: %d\n", 77+(strlen(url)*2));
	printf ("Status: 302 Temporal Relocation\n");
	printf ("Location: %s\n\n", url);
	printf ("<html>\n<body>\nThe page has been moved to <a href=\"%s\">%s</a>\n</body>\n</html>\n", url, url);
    }
}

/*
 * Local variables:
 *  c-indent-level: 4
 *  c-basic-offset: 4
 *  tab-width: 8
 * End:
 */
