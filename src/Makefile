
all: \
  autossl.so \
  connredir.so \
  cupsgetpass.so \
  forcebindip.so \
  getenv.so \
  ip_dscp.so \
  ld_env_restore.so \
  lnbuf.so \
  pathrewrite.so \
  recyclix.so \
  xopenhook.so

.PHONY: all


autossl.so:
	gcc $(CCFLAGS) autossl.c -D_GNU_SOURCE -ldl -lbsd -shared -fPIC -o $@
	strip $@
	chmod -x $@

install-autossl: /usr/tool/autossl /usr/tool/autossl-conn /usr/lib/tool/preload/autossl.so
.PHONY: install-autossl
/usr/tool/autossl: autossl
	install $< $@
/usr/tool/autossl-conn: autossl-conn
	install $< $@
/usr/lib/tool/preload/autossl.so: autossl.so | /usr/lib/tool/preload
	install $< $@
/usr/lib/tool/preload:
	mkdir -p $@
/usr/share/man/man8/autossl.1.xz: autossl.pod
	pod2man --name=autossl --section 1 --utf8 $< | xz > $@.tmp && \
	mv $@.tmp $@
/usr/share/man/man8/autossl.so.8.xz: autossl.so.pod
	pod2man --name=autossl.so --section 8 --utf8 $< | xz > $@.tmp && \
	mv $@.tmp $@
define rid
	$(shell [ ! -e ] || rm)
endef
uninstall-autossl:
	$(rid /usr/tool/autossl)
	$(rid /usr/tool/autossl-conn)
	$(rid /usr/lib/tool/preload/autossl.so)
	$(rid /usr/share/man/man8/autossl.1.xz.tmp)
	$(rid /usr/share/man/man8/autossl.1.xz)
	$(rid /usr/share/man/man8/autossl.so.8.xz.tmp)
	$(rid /usr/share/man/man8/autossl.so.8.xz)
.PHONY: uninstall-autossl

connredir.so:
	gcc $(CCFLAGS) connredir.c -D_GNU_SOURCE -ldl -shared -fPIC -o $@
	strip $@
	chmod -x $@

cupsgetpass.so:
	gcc $(CCFLAGS) -s -x c cupsgetpass.c -ldl -fPIC -shared -o $@
	strip $@
	chmod -x $@

forcebindip.so:
	gcc $(CCFLAGS) -Wall -fPIC -D _GNU_SOURCE -shared forcebindip.c -o $@
	strip $@
	chmod -x $@

getenv.so:
	gcc $(CCFLAGS) -Wall -fPIC -D _GNU_SOURCE -O0 -ldl -shared getenv.c -o $@
	strip $@
	chmod -x $@

ignore_read_flock.so:
	gcc $(CCFLAGS) -ldl -D_GNU_SOURCE -shared -fPIC ignore_read_flock.c -o $@
	strip $@
	chmod -x $@

ip_dscp.so:
	gcc $(CCFLAGS) ip_dscp.c -fPIC -shared -ldl -D_GNU_SOURCE -o $@
	strip $@
	chmod -x $@

ld_env_restore.so:
	gcc $(CCFLAGS) ld_env_restore.c -fPIC -shared -ldl -lbsd -D_GNU_SOURCE -o $@
	strip $@
	chmod -x $@

lnbuf.so:
	gcc $(CCFLAGS) -Wall -fPIC -s -include stdio.h -x c lnbuf.c -shared -o $@
	strip $@
	chmod -x $@

nofail_setfacl.so:
	gcc $(CCFLAGS) -D_GNU_SOURCE -shared -fPIC nofail_setfacl.c -o $@
	strip $@
	chmod -x $@

openhelper.so:
	gcc $(CCFLAGS) -ldl -D_GNU_SOURCE -shared -fPIC openhelper.c -o $@
	strip $@
	chmod -x $@

pathrewrite.so:
	gcc -D _GNU_SOURCE -O0 -ldl -shared pathrewrite.c -o $@
	strip $@
	chmod -x $@

recyclix.so:
	gcc $(CCFLAGS) -Wall -fPIC -D _GNU_SOURCE -O0 -ldl -shared recyclix.c -o $@
	strip $@
	chmod -x $@

saveunlinedinode.so:
	gcc -D _GNU_SOURCE -O0 -ldl -shared saveunlinedinode.c -o $@
	strip $@
	chmod -x $@

uds_chmod.so:
	gcc $(CCFLAGS) uds_chmod.c -fPIC -shared -ldl -D_GNU_SOURCE -o $@
	strip $@
	chmod -x $@

xdev.so:
	gcc -D _GNU_SOURCE -fPIC -O0 -ldl -shared xdev.c -o $@
	strip $@
	chmod -x $@

xopenhook.so:
	gcc $(CCFLAGS) xopenhook.c -D_GNU_SOURCE -ldl -lXv -shared -fPIC -o $@
	strip $@
	chmod -x $@
