#   Copyright (c) 1998  Martin Schulze <joey@infodrom.north.de>

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

prefix=/usr

CFLAGS = -I. -Wall -O2 -g

LOBJS=cgi.o cookie.o

libcgi.a: $(LOBJS)
	ar rc libcgi.a $(LOBJS)

cgitest: cgitest.o libcgi.a
	$(CC) $(CFLAGS) -L. -o cgitest cgitest.o -lcgi

install: cgitest libcgi.a
	test -d $(prefix)/include || mkdir -p $(prefix)/include
	install -o root -g root -m 644 cgi.h $(prefix)/include
	test -d $(prefix)/lib || mkdir -p $(prefix)/lib
	install -o root -g root -m 644 libcgi.a $(prefix)/lib
	test -d $(prefix)/man/man3 || mkdir -p $(prefix)/man/man3
	install -o root -g root -m 644 *.3 $(prefix)/man/man3
	test -d $(prefix)/man/man5 || mkdir -p $(prefix)/man/man5
	install -o root -g root -m 644 *.5 $(prefix)/man/man5
	install -o root -g root -m 755 cgitest $(prefix)/lib/cgi-bin

clean:
	rm -f cgitest $(LOBJS)
