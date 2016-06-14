# Simple Makefile to handle the installation of config files

DIR_CONF := /cfht/conf
INSTALL = /usr/bin/install

FILES = wircamcp_wcs.sex wircamcp_wcs.param \
	wircamcp_default.conv wircamcp_default.nnw\
	wircamcp_wcs.scamp wircam_config_zeropoint.txt \
	wircamcp_absorption_mdoff.sex\
	wircamcp_dark.conf wircamcp_flat.conf    \
	wircamcp_ctrm.sex wircamcp_ctrm.param    \
	wircamcp_wcs.missfits wircam.ahead       \
	wircamcp_wcs.swarp wircamcp_wcs.ww       \
	wircamcp_imwcs.sex wircamcp_imwcs.param  \
	wircamcp_imwcs.missfits 

XSL = scamp.xsl swarp.xsl ww.xsl missfits.xsl sextractor.xsl

$(DIR_CONF)/%: %
	@echo "--> Installing conf file $(notdir $<)"
	@test -d $(DIR_CONF) || $(INSTALL) -m 0775 -d $(DIR_CONF)
	@$(INSTALL) -m 0555 $< $@ || ( rm -f $@ && exit 1)
	cp -fp $@ $@-$(shell date +%y%m%d) #2> /dev/null

all:
	@for file in $(FILES); do \
	  make -s $(DIR_CONF)/$$file ; \
        done
	@for file in $(XSL); do \
	  make -s $(DIR_CONF)/$$file ; \
        done

install:
	@for file in $(FILES); do \
	  make -s $(DIR_CONF)/$$file ; \
	done
	@for file in $(XSL); do \
	  make -s $(DIR_CONF)/$$file ; \
	done

