# This Makefile is part of the calo-preproc package.
# It builds the User Manual and other documents automatically

# $Id: Makefile,v 1.1 2000/09/11 13:57:04 andre Exp $

# André Rabello <Andre.Rabello@ufrj.bf>

MAKEINFO=/usr/bin/makeinfo
TEXI2DVI=/usr/bin/texi2dvi

SRC = userman.texinfo

# the targets that produce documents
all: info

info:
	@$(MAKEINFO) $(SRC)

html:
	@$(MAKEINFO) --html $(SRC)

dvi:
	@$(TEXI2DVI) --texinfo=@afourpaper --clean $(SRC)

# the targets that don't need to be checked to be run
.PHONY: clean

clean:
	rm -rf *.aux *.log *.dvi *.fn *.ky *.toc *.tp *.vr *.pg *.cp *~
	rm -rf *.html *.info
	@cd fig; rm -rf *.bak *~
