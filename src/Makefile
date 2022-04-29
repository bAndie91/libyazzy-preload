
all: lnbuf forcebindip getenv pathrewrite recyclix cupsgetpass ip_dscp ld_env_restore connredir autossl xopenhook


lnbuf:
	gcc $(CCFLAGS) -Wall -fPIC -s -include stdio.h -x c $@.c -shared -o $@.so
	strip $@.so
	chmod -x $@.so

forcebindip:
	gcc $(CCFLAGS) -Wall -fPIC -D _GNU_SOURCE -shared $@.c -o $@.so
	strip $@.so
	chmod -x $@.so

getenv:
	gcc $(CCFLAGS) -Wall -fPIC -D _GNU_SOURCE -O0 -ldl -shared $@.c -o $@.so
	strip $@.so
	chmod -x $@.so

saveunlinedinode:
	gcc -D _GNU_SOURCE -O0 -ldl -shared $@.c -o $@.so
	strip $@.so
	chmod -x $@.so

pathrewrite:
	gcc -D _GNU_SOURCE -O0 -ldl -shared $@.c -o $@.so
	strip $@.so
	chmod -x $@.so

recyclix:
	gcc $(CCFLAGS) -Wall -fPIC -D _GNU_SOURCE -O0 -ldl -shared $@.c -o $@.so
	strip $@.so
	chmod -x $@.so

cupsgetpass:
	gcc $(CCFLAGS) -s -x c $@.c -ldl -fPIC -shared -o $@.so
	strip $@.so
	chmod -x $@.so

xdev:
	gcc -D _GNU_SOURCE -fPIC -O0 -ldl -shared $@.c -o $@.so
	strip $@.so
	chmod -x $@.so

ip_dscp:
	gcc $(CCFLAGS) $@.c -fPIC -shared -ldl -D_GNU_SOURCE -o $@.so
	strip $@.so
	chmod 0644 $@.so

uds_chmod:
	gcc $(CCFLAGS) $@.c -fPIC -shared -ldl -D_GNU_SOURCE -o $@.so
	strip $@.so
	chmod 0644 $@.so

nofail_setfacl:
	gcc $(CCFLAGS) -D_GNU_SOURCE -shared -fPIC $@.c -o $@.so
	strip $@.so
	chmod -x $@.so

ignore_read_flock:
	gcc $(CCFLAGS) -ldl -D_GNU_SOURCE -shared -fPIC $@.c -o $@.so
	strip $@.so
	chmod -x $@.so

openhelper:
	gcc $(CCFLAGS) -ldl -D_GNU_SOURCE -shared -fPIC $@.c -o $@.so
	strip $@.so
	chmod -x $@.so

ld_env_restore:
	gcc $(CCFLAGS) $@.c -fPIC -shared -ldl -lbsd -D_GNU_SOURCE -o $@.so
	strip $@.so
	chmod -x $@.so

connredir:
	gcc $(CCFLAGS) $@.c -D_GNU_SOURCE -ldl -shared -fPIC -o $@.so
	strip $@.so
	chmod -x $@.so

autossl:
	gcc $(CCFLAGS) $@.c -D_GNU_SOURCE -ldl -lbsd -shared -fPIC -o $@.so
	strip $@.so
	chmod -x $@.so

xopenhook:
	gcc $(CCFLAGS) $@.c -D_GNU_SOURCE -ldl -lXv -shared -fPIC -o $@.so
	strip $@.so
	chmod -x $@.so
