PKGS="ecore ecore-evas evas"

CFLAGS+=`pkg-config --cflags ${PKGS}`
LDFLAGS+=`pkg-config --libs ${PKGS}`

proxytest: proxytest.c smartproxy.c
