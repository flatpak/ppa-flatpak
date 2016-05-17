
bwrap_SOURCES = \
	/home/smcv/src/xdg-app/bubblewrap/bubblewrap.c \
	/home/smcv/src/xdg-app/bubblewrap/bind-mount.h \
	/home/smcv/src/xdg-app/bubblewrap/bind-mount.c \
	/home/smcv/src/xdg-app/bubblewrap/network.h \
	/home/smcv/src/xdg-app/bubblewrap/network.c \
	/home/smcv/src/xdg-app/bubblewrap/utils.h \
	/home/smcv/src/xdg-app/bubblewrap/utils.c \
	$(NULL)

bwrap_CFLAGS = $(AM_CFLAGS)
bwrap_LDFLAGS = $(SELINUX_LIBS)
