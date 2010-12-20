PKGS="ecore ecore-evas evas edje"

CFLAGS+=`pkg-config --cflags ${PKGS}`
LDFLAGS+=`pkg-config --libs ${PKGS}`

.PHONY: default

default: proxytest proxytest2 basic.edj

proxytest: proxytest.c smartproxy.c
proxytest2: proxytest2.c smartproxy.c

basic.edj: basic.edc
	edje_cc basic.edc
