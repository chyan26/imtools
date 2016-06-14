# Simple Makefile to handle the installation of config files
ifndef OSTYPE
    OSTYPE = $(shell uname -s|awk '{print tolower($$0)}')
endif

ifeq ($(OSTYPE),darwin)
	DIR_CONF := ../bin
endif

ifeq ($(OSTYPE),linux)
	DIR_CONF := /cfht/bin
endif




INSTALL = /usr/bin/install

FILES = wircamlibperl.pl wircamcpchip.pl wircamcpscamp.pl wircamcpstack.pl wircamcpswarp.pl \
		wircamcpwcs.pl wircamcpwm.pl wircamfiltersort.pl wircamcpsex.pl wircamrunidget.pl  \
		wircamrunidunfz.pl


$(DIR_CONF)/%: %.pl
	@echo "--> Installing scripts $(notdir $<)"
	@test -d $(DIR_CONF) || $(INSTALL) -m 0775 -d $(DIR_CONF)
	@$(INSTALL) -m 0555 $< $@ || ( rm -f $@ && exit 1)
	cp -fp $@ $@-$(shell date +%y%m%d) #2> /dev/null

all:
	@for file in $(FILES); do \
	  t=`echo $$file | sed "s/.pl//"` ;\
	  make -s $(DIR_CONF)/$$t ;\
	done

install:
	@for file in $(FILES); do \
          make -s $(DIR_CONF)/$$file ; \
    done

