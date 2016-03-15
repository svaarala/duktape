#!/bin/bash
#
#  Generate SSL keypair and self-signed certificate.  Use a two level
#  hierarchy to avoid limitations in SSL libraries (for some reason a
#  self-signed one level trust root is an issue).
#

if [ -f ca.key ]; then
    echo "ca.key already exists"
    exit 1
fi
if [ -f ca.csr ]; then
    echo "ca.csr already exists"
    exit 1
fi
if [ -f ca.crt ]; then
    echo "ca.crt already exists"
    exit 1
fi
if [ -f server.key ]; then
    echo "server.key already exists"
    exit 1
fi
if [ -f server.csr ]; then
    echo "server.csr already exists"
    exit 1
fi
if [ -f server.crt ]; then
    echo "server.crt already exists"
    exit 1
fi

echo
echo "*** Creating non-encrypted private key (CA) ***"
echo
#openssl genrsa -des3 -out ca.key 2048
openssl genrsa -out ca.key 2048

echo
echo "*** Creating certificate signing request (CA) ***"
echo
echo "Respond to CSR prompts manually.  Common name should probably match"
echo "FQDN or IP address used by client."
echo
openssl req -new -key ca.key -out ca.csr

echo
echo "*** Creating self-signed certificate (CA) ***"
echo

DAYS=5475  # 15*365
openssl x509 -req -days $DAYS -in ca.csr -signkey ca.key -out ca.crt

echo
echo "*** Creating non-encrypted private key (server) ***"
echo
#openssl genrsa -des3 -out server.key 2048
openssl genrsa -out server.key 2048

echo
echo "*** Creating certificate signing request (server) ***"
echo
echo "Respond to CSR prompts manually.  Common name should probably match"
echo "FQDN or IP address used by client."
echo
openssl req -new -key server.key -out server.csr

echo
echo "*** Creating certificate (server) ***"
echo

DAYS=5475  # 15*365
openssl x509 -req -days $DAYS -in server.csr -CA ca.crt -CAkey ca.key -CAcreateserial -out server.crt
