############################################################################
# apps/examples/hello/Make.defs
#
# Licensed to the Apache Software Foundation (ASF) under one or more
# contributor license agreements.  See the NOTICE file distributed with
# this work for additional information regarding copyright ownership.  The
# ASF licenses this file to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance with the
# License.  You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
# License for the specific language governing permissions and limitations
# under the License.
#
############################################################################

include $(APPDIR)/Make.defs

PROGNAME  = $(CONFIG_EXAMPLES_RUSSELLSAPP_PROGNAME)
PRIORITY  = $(CONFIG_EXAMPLES_RUSSELLSAPP_PRIORITY)
STACKSIZE = $(CONFIG_EXAMPLES_RUSSELLSAPP_STACKSIZE)
MODULE    = $(CONFIG_EXAMPLES_RUSSELLSAPP)

MAINSRC = rapp_main_1.c

CSRCS += fs_api.c
# COBJS = $(patsubst %.c, $(BINDIR)$(DELIM)$(DELIM)%$(OBJEXT), $(CSRCS))
# COBJS = $(patsubst %.c,%$(OBJEXT), $(CSRCS))
#  
# $(info CSRCS="$(CSRCS)")
# $(info COBJS="$(COBJS)")
#  
# $(COBJS): %.c
# 	$(call COMPILE, $<, $@)

checkgenromfs:
	@genromfs -h 1>/dev/null 2>&1 || { \
 echo "Host executable genromfs not available in PATH"; \
 echo "You may need to download in from http://romfs.sourceforge.net/"; \
 exit 1; \
	}

scripts.img : checkgenromfs scripts
	@genromfs -f $@ -d scripts -V "ROMFS_Test" || { echo "genromfs failed" ; exit 1 ; }

scripts.h : scripts.img
	@xxd -i $< >$@ || { echo "xxd of $< failed" ; exit 1 ; }

context:: scripts.h

include $(APPDIR)/Application.mk
