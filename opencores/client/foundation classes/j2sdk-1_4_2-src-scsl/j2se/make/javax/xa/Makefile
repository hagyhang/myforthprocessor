#
# @(#)Makefile	1.6 03/01/23
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for building JDBC javax.sql
#

BUILDDIR = ../..
PACKAGE = javax.transaction.xa
PRODUCT = sun
include $(BUILDDIR)/common/Defs.gmk

#
# Files to compile
#
FILES_java = \
	javax/transaction/xa/XAException.java \
        javax/transaction/xa/Xid.java \
        javax/transaction/xa/XAResource.java

#
# Rules
#
include $(BUILDDIR)/common/Classes.gmk

#
# Clean up.
#
clean::
	$(RM) -r $(CLASSDESTDIR)/javax/transaction/xa
