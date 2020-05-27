
all: lnbuf forcebindip getenv pathrewrite recyclix cupsgetpass ip_dscp ld_env_restore


lnbuf.so:
	gcc $(CCFLAGS) -Wall -fPIC -s -include stdio.h -x c $(basename $@).c -shared -o $@
	strip $@
	chmod -x $@

forcebindip.so:
	gcc $(CCFLAGS) -Wall -fPIC -D _GNU_SOURCE -shared $(basename $@).c -o $@
	strip $@
	chmod -x $@

getenv.so:
	gcc $(CCFLAGS) -Wall -fPIC -D _GNU_SOURCE -O0 -ldl -shared $(basename $@).c -o $@
	strip $@
	chmod -x $@

saveunlinedinode.so:
	gcc -D _GNU_SOURCE -O0 -ldl -shared $(basename $@).c -o $@
	strip $@
	chmod -x $@

pathrewrite.so:
	gcc -D _GNU_SOURCE -O0 -ldl -shared $(basename $@).c -o $@
	strip $@
	chmod -x $@

recyclix.so:
	gcc $(CCFLAGS) -Wall -fPIC -D _GNU_SOURCE -O0 -ldl -shared $(basename $@).c -o $@
	strip $@
	chmod -x $@

cupsgetpass.so:
	gcc $(CCFLAGS) -s -x c $(basename $@).c -fPIC -shared -o $@
	strip $@
	chmod -x $@

xdev.so:
	gcc -D _GNU_SOURCE -fPIC -O0 -ldl -shared $(basename $@).c -o $@
	strip $@
	chmod -x $@

ip_dscp.so:
	gcc $(CCFLAGS) $(basename $@).c -fPIC -shared -ldl -D_GNU_SOURCE -o $@
	strip $@
	chmod 0644 $@

uds_chmod.so:
	gcc $(CCFLAGS) $(basename $@).c -fPIC -shared -ldl -D_GNU_SOURCE -o $@
	strip $@
	chmod 0644 $@

nofail_setfacl.so:
	gcc $(CCFLAGS) -D_GNU_SOURCE -shared -fPIC $(basename $@).c -o $@
	strip $@
	chmod -x $@

ignore_read_flock.so:
	gcc $(CCFLAGS) -ldl -D_GNU_SOURCE -shared -fPIC $(basename $@).c -o $@
	strip $@
	chmod -x $@

openhelper.so:
	gcc $(CCFLAGS) -ldl -D_GNU_SOURCE -shared -fPIC $(basename $@).c -o $@
	strip $@
	chmod -x $@

ld_env_restore.so:
	gcc $(CCFLAGS) $(basename $@).c -fPIC -shared -ldl -lbsd -D_GNU_SOURCE -o $@
	chmod -x $@
