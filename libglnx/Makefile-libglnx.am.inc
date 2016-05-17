# Copyright (C) 2015 Colin Walters <walters@verbum.org>
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the
# Free Software Foundation, Inc., 59 Temple Place - Suite 330,
# Boston, MA 02111-1307, USA.

EXTRA_DIST += /home/smcv/src/xdg-app/libglnx/README.md /home/smcv/src/xdg-app/libglnx/COPYING

libglnx_la_SOURCES = \
	/home/smcv/src/xdg-app/libglnx/glnx-alloca.h \
	/home/smcv/src/xdg-app/libglnx/glnx-backport-autocleanups.h \
	/home/smcv/src/xdg-app/libglnx/glnx-backport-autoptr.h \
	/home/smcv/src/xdg-app/libglnx/glnx-backports.h \
	/home/smcv/src/xdg-app/libglnx/glnx-backports.c \
	/home/smcv/src/xdg-app/libglnx/glnx-local-alloc.h \
	/home/smcv/src/xdg-app/libglnx/glnx-local-alloc.c \
	/home/smcv/src/xdg-app/libglnx/glnx-errors.h \
	/home/smcv/src/xdg-app/libglnx/glnx-errors.c \
	/home/smcv/src/xdg-app/libglnx/glnx-console.h \
	/home/smcv/src/xdg-app/libglnx/glnx-console.c \
	/home/smcv/src/xdg-app/libglnx/glnx-dirfd.h \
	/home/smcv/src/xdg-app/libglnx/glnx-dirfd.c \
	/home/smcv/src/xdg-app/libglnx/glnx-fdio.h \
	/home/smcv/src/xdg-app/libglnx/glnx-fdio.c \
	/home/smcv/src/xdg-app/libglnx/glnx-lockfile.h \
	/home/smcv/src/xdg-app/libglnx/glnx-lockfile.c \
	/home/smcv/src/xdg-app/libglnx/glnx-libcontainer.h \
	/home/smcv/src/xdg-app/libglnx/glnx-libcontainer.c \
	/home/smcv/src/xdg-app/libglnx/glnx-xattrs.h \
	/home/smcv/src/xdg-app/libglnx/glnx-xattrs.c \
	/home/smcv/src/xdg-app/libglnx/glnx-shutil.h \
	/home/smcv/src/xdg-app/libglnx/glnx-shutil.c \
	/home/smcv/src/xdg-app/libglnx/libglnx.h \
	$(NULL)

libglnx_la_CFLAGS = $(AM_CFLAGS) $(libglnx_cflags)
libglnx_la_LDFLAGS = -avoid-version -Bsymbolic-functions -export-symbols-regex "^glnx_" -no-undefined -export-dynamic 
libglnx_la_LIBADD = $(libglnx_libs)
