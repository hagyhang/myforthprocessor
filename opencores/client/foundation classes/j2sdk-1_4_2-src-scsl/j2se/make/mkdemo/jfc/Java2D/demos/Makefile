#
# @(#)Makefile	1.9 03/01/23
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# @(#)Makefile	1.9	03/01/23
#
# Makefile for building the Java 2D Demo subdirectories
#

BUILDDIR = ../../../..
include $(BUILDDIR)/common/Defs.gmk

SUBDIRS = Arcs_Curves Clipping Colors Composite Fonts Images Mix Lines Paint Paths Transforms

all optimized debug clean clobber::
	@for i in $(SUBDIRS) ; do \
	    $(ECHO) ">>>Recursively making "$$i" "$@" @ `$(DATE)` ..."; \
	    $(CD) $$i; $(MAKE) $@ RELEASE=$(RELEASE) FULL_VERSION=$(FULL_VERSION)\
            || exit 1; $(CD) ..;  \
	    $(ECHO) "<<<Finished Recursively making "$$i" "$@" @ `$(DATE)`." ; \
	done

clobber clean::
