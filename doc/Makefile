# This Makefile is part of the calo-preproc package.
# It builds the User Manual and other documents automatically

# $Id: Makefile,v 1.3 2000/11/07 16:27:26 rabello Exp $

# André Rabello <Andre.Rabello@ufrj.bf>

MAKEINFO=/usr/bin/makeinfo
TEXI2DVI=/usr/bin/texi2dvi

SRC = userman.texinfo

# the targets that produce documents
all: info html dvi

info:
	@$(MAKEINFO) $(SRC)
	@echo \* Built INFO file.

html:
	@$(MAKEINFO) --html $(SRC)
	@echo \* Built HTML file.

dvi:
	@$(TEXI2DVI) --texinfo=@afourpaper --clean --quiet $(SRC)
	@mv $(SRC:%.texinfo=%.dvi) calo-preproc.dvi
	@echo \* Built DVI file. Print it directly with \`lpr -d\'

# the targets that don't need to be checked to be run
.PHONY: clean

clean:
	rm -rf *.aux *.log *.dvi *.fn *.ky *.toc *.tp *.vr *.pg *.cp *~
	rm -rf *.html *.info
	@cd fig; rm -rf *.bak *~

