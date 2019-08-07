
all: lnbuf forcebindip getenv pathrewrite recyclix cupsgetpass ip_dscp


lnbuf:
	gcc $(CCFLAGS) -Wall -fPIC -s -include stdio.h -x c $@.c -shared -o $@.so
	strip $@.so
	chmod 0644 $@.so

forcebindip:
	gcc $(CCFLAGS) -Wall -fPIC -D _GNU_SOURCE -shared $@.c -o $@.so
	strip $@.so
	chmod 0644 $@.so

getenv:
	gcc $(CCFLAGS) -Wall -fPIC -D _GNU_SOURCE -O0 -ldl -shared $@.c -o $@.so
	strip $@.so
	chmod 0644 $@.so

saveunlinedinode:
	gcc -D _GNU_SOURCE -O0 -ldl -shared $@.c -o $@.so
	strip $@.so
	chmod 0644 $@.so

pathrewrite:
	gcc -D _GNU_SOURCE -O0 -ldl -shared $@.c -o $@.so
	strip $@.so
	chmod 0644 $@.so

recyclix:
	gcc $(CCFLAGS) -Wall -fPIC -D _GNU_SOURCE -O0 -ldl -shared $@.c -o $@.so
	strip $@.so
	chmod 0644 $@.so

recyclix-dbg:
	gcc -D DEBUG $(CCFLAGS) -Wall -fPIC -D _GNU_SOURCE -O0 -ldl -shared recyclix.c -o $@.so
	strip $@.so
	chmod 0644 $@.so

cupsgetpass:
	gcc $(CCFLAGS) -s -x c $@.c -fPIC -shared -o $@.so
	strip $@.so
	chmod 0644 $@.so

xdev:
	gcc -D _GNU_SOURCE -fPIC -O0 -ldl -shared $@.c -o $@.so
	strip $@.so
	chmod 0644 $@.so

ip_dscp:
	gcc $(CCFLAGS) $@.c -fPIC -shared -ldl -D_GNU_SOURCE -o $@.so
	strip $@.so
	chmod 0633 $@.so
