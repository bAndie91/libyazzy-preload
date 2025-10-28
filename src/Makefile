
INSTALL_PREFIX = /usr/lib/tool/preload
INSTALL_WRAPPER = 

ALL_TARGET = \
  connredir.so \
  cupsgetpass.so \
  forcebindip.so \
  getenv.so \
  ip_dscp.so \
  ld_env_restore.so \
  lnbuf.so \
  pathrewrite.so \
  recyclix.so \
#  xopenhook.so \
  x11wmclass.so \



.PHONY: all
all: $(ALL_TARGET)

ALL_INSTALL_TARGET = $(foreach target,$(ALL_TARGET),$(INSTALL_PREFIX)/$(target))

.PHONY: install-all
install-all: $(ALL_INSTALL_TARGET)

$(ALL_INSTALL_TARGET): $(INSTALL_PREFIX)/%: %
	$(INSTALL_WRAPPER) install --compare $< $@


connredir.so:
	gcc $(CCFLAGS) connredir.c -D_GNU_SOURCE -ldl -shared -fPIC -o $@
	chmod -x $@

cupsgetpass.so:
	gcc $(CCFLAGS) -s -x c cupsgetpass.c -ldl -fPIC -shared -o $@
	chmod -x $@

forcebindip.so:
	gcc $(CCFLAGS) -Wall -fPIC -D _GNU_SOURCE -shared forcebindip.c -o $@
	chmod -x $@

forcebindiface.so: forcebindiface.c
	gcc -nostartfiles -fpic -shared $< -o $@ -ldl -D_GNU_SOURCE
	chmod -x $@

getenv.so:
	gcc $(CCFLAGS) -Wall -fPIC -D _GNU_SOURCE -O0 -ldl -shared getenv.c -o $@
	chmod -x $@

ignore_read_flock.so:
	gcc $(CCFLAGS) -ldl -D_GNU_SOURCE -shared -fPIC ignore_read_flock.c -o $@
	chmod -x $@

ip_dscp.so:
	gcc $(CCFLAGS) ip_dscp.c -fPIC -shared -ldl -D_GNU_SOURCE -o $@
	chmod -x $@

ld_env_restore.so:
	gcc $(CCFLAGS) ld_env_restore.c -fPIC -shared -ldl -lbsd -D_GNU_SOURCE -o $@
	chmod -x $@

lnbuf.so:
	gcc $(CCFLAGS) -Wall -fPIC -s -include stdio.h -x c lnbuf.c -shared -o $@
	chmod -x $@

nofail_setfacl.so:
	gcc $(CCFLAGS) -D_GNU_SOURCE -shared -fPIC nofail_setfacl.c -o $@
	chmod -x $@

openhelper.so:
	gcc $(CCFLAGS) -ldl -D_GNU_SOURCE -shared -fPIC openhelper.c -o $@
	chmod -x $@

pathrewrite.so:
	gcc -D _GNU_SOURCE -O0 -ldl -shared -fPIC pathrewrite.c -o $@
	chmod -x $@

recyclix.so:
	gcc $(CCFLAGS) -Wall -fPIC -D _GNU_SOURCE -O0 -ldl -shared recyclix.c -o $@
	chmod -x $@

saveunlinedinode.so:
	gcc -D _GNU_SOURCE -O0 -ldl -shared saveunlinedinode.c -o $@
	chmod -x $@

uds_chmod.so:
	gcc $(CCFLAGS) uds_chmod.c -fPIC -shared -ldl -D_GNU_SOURCE -o $@
	chmod -x $@

xdev.so:
	gcc -D _GNU_SOURCE -fPIC -O0 -ldl -shared xdev.c -o $@
	chmod -x $@

xopenhook.so:
	gcc $(CCFLAGS) xopenhook.c -D_GNU_SOURCE -ldl -lXv -shared -fPIC -o $@
	chmod -x $@

oobpsai.so: oob-proxy-socket-address-info.c oob-proxy-socket-address-info-preload.c
	$(CC) -D_GNU_SOURCE -nostartfiles -shared -fpic -fPIC \
		-fvisibility=hidden $(CCFLAGS) \
		$^ -o $@ \
		-ldl -Wl,-z,relro,-z,now -Wl,-z,noexecstack $(LDFLAGS)

x11wmclass.so: x11wmclass.c
	gcc -shared -fPIC -o $@ $< -ldl -lX11
	chmod -x $@
