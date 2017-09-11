#!/bin/sh
set -e
gtkdocize
autoreconf -fi
cd builder
autoreconf -fi
