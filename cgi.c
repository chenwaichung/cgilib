/*
    cgi.c - Some simple routines for CGI programming
    Copyright (c) 1996-9,2007  Martin Schulze <joey@infodrom.org>

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

#define _GNU_SOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <syslog.h>
#include <cgi.h>
#include "aux.h"

char *cgiHeaderString = NULL;
char *cgiType = NULL;

extern s_cookie **cgiReadCookies();

int cgiSetHeader (char *name, char *value)
{
    char *cp, *vp, *pivot;
    int len;

    if (!name || !strlen (name) || !value || !strlen (value))
	return 0;
    
    for (cp=name;*cp && *cp!=' ' && *cp!='\r' && *cp!='\n' && *cp!=':';cp++);
    for (vp=value;*vp && *vp!='\r' && *vp!='\n';vp++);

    if (cgiHeaderString) {
	len = (strlen (cgiHeaderString) + cp-name + vp-value + 5) * sizeof (char);
	if ((pivot = (char *)realloc (cgiHeaderString,len)) == NULL)
	    return 0;
	cgiHeaderString = pivot;
	pivot += strlen (cgiHeaderString);
    } else {
	len = (cp-name + vp-value + 5) * sizeof (char);
	if ((cgiHeaderString = (char *)malloc (len)) == NULL)
	    return 0;
	pivot = cgiHeaderString;
    }
    strncpy (pivot, name, cp-name);
    strncat (pivot, ": ", 2);
    strncat (pivot, value, vp-value);
    strncat (pivot, "\r\n", 2);

    return 1;
}

int cgiSetType (char *type)
{
    int len;
    char *cp;

    if (!type || !strlen (type))
	return 0;
    if (cgiType)
	free (cgiType);

    for (cp=type;*cp && *cp!='\r' && *cp!='\n';cp++);

    len = (cp-type+1) * sizeof (char);
    if ((cgiType = (char *)malloc (len+20)) == NULL)
	return 0;
    memset (cgiType, 0, len);
    strncpy (cgiType, type, cp-type);
    
    return 1;
}

void cgiHeader ()
{
    if (cgiType)
	printf ("Content-type: %s\r\n", cgiType);
    else
	printf ("Content-type: text/html\r\n");
    if (cgiHeaderString)
	printf ("%s", cgiHeaderString);
    printf ("\r\n");
}

void cgiDebug (int level, int where)
{
    if (level > 0)
	cgiDebugLevel = level;
    else
	cgiDebugLevel = 0;
    if (where > 0) {
	if (where < 3) {
	    cgiDebugType = where;
	    if (where == 2)
		openlog("cgilib", LOG_PID, LOG_USER);
	} else
	    cgiDebugType = 0;
    }
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

/* cgiReadMultipart()
 *
 * Decode multipart/form-data
 */
#define MULTIPART_DELTA 5
s_cgi *cgiReadMultipart (char *boundary)
{
    char *line;
    char *cp, *xp;
    char *name = NULL, *type = NULL;
    int header = 1;
    s_var **result = NULL;
    s_var **tmp;
    int numresults = 0, current = 0;
    int index = 0;
    size_t len;
    s_cgi *res;
    
    while ((line = cgiGetLine (stdin)) != NULL) {

	if (!strncmp (line, boundary, strlen(boundary))) {
	    header = 1;
	} else if (!strncasecmp (line, "Content-Disposition: form-data; ", 32)) {
	    if (!name) {
		if ((cp = strstr (line, "name=\"")) == NULL)
		    continue;
		cp += 6;
		if ((xp = strchr (cp, '\"')) == NULL)
		    continue;
		name = strndup (cp, xp-cp);
		cgiDebugOutput (2, "Found field name %s", name);
	    }
	} else if (!strncasecmp (line, "Content-Type: ", 14)) {
	    if (!type) {
		cp = line + 14;
		type = strdup (cp);
		cgiDebugOutput (2, "Found mime type %s", type);
	    }
	} else if (header) {
	    if (!strlen(line))
		header = 0;
	} else {
	    cgiDebugOutput (2, "Found data: %s", line);
	    if (name) {

		/* try to find out if there's already such a variable */
		for (index=0; index<current && strcmp (result[index]->name,name); index++);

		if (index == current) {
		    if (!result) {
			len = MULTIPART_DELTA * sizeof (s_var *);
			if ((result = (s_var **)malloc (len)) == NULL)
			    return NULL;
			numresults = MULTIPART_DELTA;
			memset (result, 0, len);
			current = 0;
		    } else {
			if (current+2 > numresults) {
			    len = (numresults + MULTIPART_DELTA) * sizeof(s_var *);
			    if ((tmp = (s_var **)realloc (result, len)) == NULL) {
				for (index=0; result[index]; index++)
				    free (result[index]);
				free (result);
				free (name);
				return NULL;
			    }
			    result = tmp;
			    memset (result + numresults*sizeof(s_var *), 0, len - numresults*sizeof(s_var *));
			    numresults += MULTIPART_DELTA;
			}
		    }
		    if ((result[current] = (s_var *)malloc(sizeof(s_var))) == NULL) {
			for (index=0; result[index]; index++)
			    free (result[index]);
			free (result);
			free (name);
			return NULL;
		    }
		    current++;
		    cgiDebugOutput (3, "Set #%d to %s=%s", index, name, line);
		    result[index]->name = name; name = NULL;
		    result[index]->value = strdup (line);
		} else {
		    cgiDebugOutput (3, "Set #%d to %s=%s", index, name, line);
		    free (name);
		    if ((name = (char *)realloc (result[index]->value, strlen(result[index]->value)+strlen(line)+2)) != NULL) {
			strcat(name, "\n");
			strcat(name, line);
			result[index]->value = name;
			name = NULL;
		    }
		}
	    } else {
		if (index > 0) {
		    if ((name = (char *)malloc (strlen(result[index]->value)+strlen(line)+3)) == NULL) {
			for (index=0; result[index]; index++)
			    free (result[index]);
			free (result);
			return NULL;
		    }
		    sprintf (name, "%s\r\n%s", result[index]->value, line);
		    free (result[index]->value);
		    result[index]->value = name;
		    name = NULL;
		}
	    }
	}

    }

    if ((res = (s_cgi *)malloc (sizeof (s_cgi))) == NULL)
       return NULL;

    res->vars = result;
    res->cookies = NULL;
    res->files = NULL;

    return res;
}

/*  cgiReadVariables()
 *
 *  Read from stdin if no string is provided via CGI.  Variables that
 *  doesn't have a value associated with it doesn't get stored.
 */
s_cgi *cgiReadVariables ()
{
    int length;
    char *line = NULL;
    int numargs;
    char *cp, *ip, *esp, *sptr;
    s_var **result;
    int i, k, len;
    char tmp[101];
    s_cgi *res;

    cp = getenv("CONTENT_TYPE");
    cgiDebugOutput (2, "Content-Type: %s", cp);
    if (cp && strstr(cp, "multipart/form-data") && strstr(cp, "boundary=")) {
	cp = strstr(cp, "boundary=") + strlen ("boundary=") - 2;
	*cp = *(cp+1) = '-';
	return cgiReadMultipart (cp);
    }

    cp = getenv("REQUEST_METHOD");
    cgiDebugOutput (2, "REQUEST_METHOD: %s", cp);
    ip = getenv("CONTENT_LENGTH");

    if (cp && !strcmp(cp, "POST")) {
	if (ip) {
	    length = atoi(ip);
	    if (length <= 0)
		return NULL;
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

    cgiDebugOutput (1, "Received CGI input: %s", line);

    for (cp=line; *cp; cp++)
	if (*cp == '+')
	    *cp = ' ';

    if (strlen(line)) {
	for (numargs=1,cp=line; *cp; cp++)
	    if (*cp == '&' || *cp == ';' ) numargs++;
    } else
	numargs = 0;
    cgiDebugOutput (1, "%d cgi variables found.", numargs);

    len = (numargs+1) * sizeof(s_var *);
    if ((result = (s_var **)malloc (len)) == NULL)
	return NULL;
    memset (result, 0, len);

    cp = line;
    i=0;
    while (*cp) {
	if ((ip = (char *)strchr(cp, '&')) != NULL) {
	    *ip = '\0';
	} else if ((ip = (char *)strchr(cp, ';')) != NULL) {
	    *ip = '\0';
	} else
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

	    /* try to find out if there's already such a variable */
	    for (k=0; k<i && (strncmp (result[k]->name,cp, esp-cp) || !(strlen (result[k]->name) == esp-cp)); k++);

	    if (k == i) {	/* No such variable yet */
		if ((result[i] = (s_var *)malloc(sizeof(s_var))) == NULL)
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
		cgiDebugOutput (1, "%s: %s", result[i]->name, result[i]->value);
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
		result[k]->value = cgiDecodeString (sptr);
	    }
	}
	cp = ++ip;
    }

    if ((res = (s_cgi *)malloc (sizeof (s_cgi))) == NULL)
	return NULL;

    res->vars = result;
    res->cookies = NULL;
    res->files = NULL;

    return res;
}

/*  cgiInit()
 *
 *  Read from stdin if no string is provided via CGI.  Variables that
 *  doesn't have a value associated with it doesn't get stored.
 */
s_cgi *cgiInit()
{
    s_cgi *res;

    res = cgiReadVariables ();
    res->cookies = cgiReadCookies ();

    if (!res->vars && !res->cookies && !res->files) {
	free (res);
	return NULL;
    }

    return res;
}

char *cgiGetValue (s_cgi *parms, const char *name)
{
    int i;

    if (!parms || !parms->vars)
	return NULL;
    for (i=0;parms->vars[i]; i++)
	if (!strcmp(name,parms->vars[i]->name)) {
	    cgiDebugOutput (1, "%s found as %s", name, parms->vars[i]->value);
	    if (strlen(parms->vars[i]->value) > 0)
		return parms->vars[i]->value;
	    else
		return NULL;
	}
    cgiDebugOutput (1, "%s not found", name);
    return NULL;
}

char **cgiGetVariables (s_cgi *parms)
{
    int i;
    char **res = NULL;
    int len;

    if (!parms || !parms->vars)
	return NULL;
    for (i=0;parms->vars[i]; i++);
    len = sizeof (char *) * ++i;
    if ((res = (char **)malloc (len)) == NULL)
	return NULL;
    memset (res, 0, len);
    for (i=0;parms->vars[i]; i++) {
	len = strlen (parms->vars[i]->name) +1;
	if ((res[i] = (char *)malloc (len)) == NULL)
	    return NULL;
	memset (res[i], 0, len);
	strcpy (res[i], parms->vars[i]->name);
    }
    return res;
}

void cgiRedirect (const char *url)
{
    if (url && strlen(url)) {
	printf ("Content-type: text/html\r\nContent-length: %d\r\n", 77+(strlen(url)*2));
	printf ("Status: 302 Temporal Relocation\r\n");
	printf ("Location: %s\r\n\r\n", url);
	printf ("<html>\n<body>\nThe page has been moved to <a href=\"%s\">%s</a>\n</body>\n</html>\n", url, url);
    }
}

void cgiFreeList (char **list)
{
    int i;

    for (i=0; list[i] != NULL; i++)
	free (list[i]);
	free (list);
}

void cgiFree (s_cgi *parms)
{
    int i;

    if (!parms)
	return;
    if (parms->vars) {
	for (i=0;parms->vars[i]; i++) {
	    if (parms->vars[i]->name)
		free (parms->vars[i]->name);
	    if (parms->vars[i]->value)
		free (parms->vars[i]->value);
	    free (parms->vars[i]);
	}
	free (parms->vars);
    }
    if (parms->cookies) {
	if (parms->cookies[0]->version)
	    free (parms->cookies[0]->version);
	for (i=0;parms->cookies[i]; i++) {
	    if (parms->cookies[i]->name)
		free (parms->cookies[i]->name);
	    if (parms->cookies[i]->value)
		free (parms->cookies[i]->value);
	    if (parms->cookies[i]->path)
		free (parms->cookies[i]->path);
	    if (parms->cookies[i]->domain)
		free (parms->cookies[i]->domain);
	    free (parms->cookies[i]);
	}
	free (parms->cookies);
    }
    free (parms);

    if (cgiHeaderString) {
	free (cgiHeaderString);
	cgiHeaderString = NULL;
    }
    if (cgiType) {
	free (cgiType);
	cgiType = NULL;
    }
}

/*
 * Local variables:
 *  c-indent-level: 4
 *  c-basic-offset: 4
 *  tab-width: 8
 * End:
 */
