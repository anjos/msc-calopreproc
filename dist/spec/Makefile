#
# $Id: Makefile,v 1.1 2000/03/13 21:03:42 rabello Exp $
#
all:
	cd src; $(MAKE) $(MFLAGS)

clean:
	cd src; $(MAKE) $(MFLAGS) clean

dist:
	(cd ..; tar cvf - `cat ./spec/FILES` | gzip > spec-`cat ./spec/VERSION`.tar.gz )
