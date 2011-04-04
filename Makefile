PKGS="ecore ecore-evas evas edje"

CFLAGS+=`pkg-config --cflags ${PKGS}`
LDFLAGS+=`pkg-config --libs ${PKGS}`

.PHONY: default

default: proxytest proxytest2 test_proxy basic.edj

proxytest: proxytest.c smartproxy.c
proxytest2: proxytest2.c smartproxy.c
test_proxy: test_proxy.c

basic.edj: basic.edc
	edje_cc basic.edc

install:
	mkdir -p ${PREFIX}/share/proxytest
	cp *.jpg *.png ${PREFIX}/share/proxytest
	cp proxytest ${PREFIX}/bin
	cp proxytest.sh ${PREFIX}/bin

clean:
	rm -f proxytest proxytest2 basic.edj
