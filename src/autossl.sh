#!/bin/bash

echo "autossl.sh here called with $*" >&2

ip=$1
plain_port=$2
declare -A tls_ports
tls_ports=([21]=990 [23]=992 [25]=465 [80]=443 [110]=995 [119]=563 [143]=993 [194]=994 [389]=636)

tls_port=${tls_ports[$plain_port]}

if [ -z $tls_port ]
then
	unset AUTOSSL_UPGRADE_PORT
	exec nc -v "$ip" "$plain_port"
else
	echo "autossl.sh: opening TLS channel to $ip:$tls_port" >&2
	openssl s_client -connect "$ip:$tls_port" -quiet
fi
