include Makedefs

.PHONY: all clean backup

all: Makefile
	$(MAKE) -f Makerules sueca
clean:
	$(RM) $(OBJS) $(DEPS) *~ sueca core core.[0-9]*

backup: PROJBASE="$(shell basename $(CURDIR))"
backup: clean
	cd ..; tar czf sueca-$(shell date '+%F-%H-%M').tar.gz $(PROJBASE)
