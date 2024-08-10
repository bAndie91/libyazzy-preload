
all: \
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

forcebindiface.so: forcebindiface.c
	gcc -nostartfiles -fpic -shared $< -o $@ -ldl -D_GNU_SOURCE

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

psai.so: proxy-socket-address-info.c
	$(CC) -D_GNU_SOURCE -nostartfiles -shared -fpic -fPIC \
		-fvisibility=hidden $(CCFLAGS) \
		$< -o $@ \
		-ldl -Wl,-z,relro,-z,now -Wl,-z,noexecstack $(LDFLAGS)
