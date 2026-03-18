#!/bin/sh

set -eu

mkdir -p /certs/

if [ ! -f /certs/key.pem ] || [ ! -f /certs/cert.pem ]; then
    openssl req -x509 -newkey rsa:2048 -nodes \
        -keyout /certs/key.pem \
        -out /certs/cert.pem \
        -days 30 \
        -subj "/CN=localhost"
fi

exec nginx -g 'daemon off;'