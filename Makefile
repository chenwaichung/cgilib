#   Copyright (c) 1998,9,2001 by Martin Schulze <joey@infodrom.org>

#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.

#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.

#   You should have received a copy of the GNU General Public License
#   along with this program; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111, USA.

CFLAGS = -Wall -O2 -g
IFLAGS = -I.
LDFLAGS = -L.
LIBS = -lcgi

OBJS = cgi.o cookies.o

libcgi.a: $(OBJS)
	ar rc $@ $^

cgitest: cgitest.o libcgi.a
	$(CC) $(CFLAGS) $(IFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

jumpto: jumpto.o libcgi.a
	$(CC) $(CFLAGS) $(IFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

all: libcgi.a cgitest jumpto

install: cgitest
	install -m 755 cgitest /usr/lib/cgi-bin

clean:
	rm -f cgitest cgitest.o jumpto jumpto.o libcgi.a $(OBJS) *.[35].html

htmlman:
	for f in *.[35]; do \
	  man -l $$f|rman -f HTML --title $$f -r "%s.%s.html" > $$f.html; \
	done
