PKGS="ecore ecore-evas evas edje"

CFLAGS+=`pkg-config --cflags ${PKGS}`
LDFLAGS+=`pkg-config --libs ${PKGS}`

.PHONY: default

default: proxytest basic.edj

proxytest: proxytest.c smartproxy.c

basic.edj: basic.edc
	edje_cc basic.edc
