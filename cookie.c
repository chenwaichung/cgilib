/*
    cookie.c - Cookie support
    Copyright (C) 1998 Mikko Torni <mtorni@freenet.hut.fi>
     
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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

/*
    Sun Dec  6 15:39:31 MET 1998: Martin Schulze <joey@infodrom.north.de>
	Reformatted, added exclusion if no cookie has been provided.
*/


#include <stdlib.h>
#include <string.h>

static struct cookie_jar {
    char *name,*val;
} *jar;

static void cookie_collect(char *s) {
    int jarsz = 0;
    int cl,el;

    if(s) el=strlen(s);
    while(s && (el>0)) {
	while(*s==' ') s++;
	jar=realloc(jar,sizeof(struct cookie_jar)*++jarsz);
	cl=strcspn(s,"=");
	jar[jarsz-1].name=malloc(cl+1);
	strncpy(jar[jarsz-1].name,s,cl);
	jar[jarsz-1].name[cl]=0;
	s+=cl+1;
	cl=strcspn(s,";");
	jar[jarsz-1].val=malloc(cl+1);
	strncpy(jar[jarsz-1].val,s,cl);
	jar[jarsz-1].val[cl]=0;
	el-=cl+1;
	s+=cl+1;
    }
    jar=realloc(jar,sizeof(struct cookie_jar)*++jarsz);
    jar[jarsz-1].name=0;
    jar[jarsz-1].val=0;
    return;
}

char *cgiGetCookie(char *name) {
    int i=0;
    if (name==0) return 0;
    if (jar==0) cookie_collect(getenv("HTTP_COOKIE"));
    if (jar > 0) {
	while(jar[i].name) {
	    if(strcmp(jar[i].name,name)==0) {
		return jar[i].val;
	    }
	    i++;
	}
    }
    return NULL;
}


/*
 * Local variables:
 *  c-indent-level: 4
 *  c-basic-offset: 4
 *  tab-width: 8
 * End:
 */
