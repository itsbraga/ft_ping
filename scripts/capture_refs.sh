#!/bin/sh

. "$(dirname "$0")/colors.sh"

#	Stop the script on any error
set -e

#	Captures the inetutils-2.0 reference output for every test case
#	Two files per case:
#		.cat	output (stdout + stderr merged) through cat -A:
#				trailing spaces and tabs made visible
#		.code	exit status (0 success, 64 usage error, 1 runtime failure)

REF_PING="/opt/inetutils/bin/ping"

SCRIPT_DIR="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"
ROOT_DIR="$(dirname -- "$SCRIPT_DIR")"
REFS_DIR="${ROOT_DIR}/refs"

printf "${BOLD}${PP}=======  REFERENCE CORPUS CAPTURE  =======${NC}\n\n"

#	The reference is setuid-root and branches on getuid(): running this as
#	root would record the privileged code path, which ft_ping never takes
if [ "$(id -u)" -eq 0 ]; then
	printf "${BOLD}${RED}Error:${NC} this script must not be run as root\n"
	exit 1
fi

if [ ! -x "$REF_PING" ]; then
	printf "${BOLD}${RED}Error:${NC} %s not found, run setup.sh first\n" "$REF_PING"
	exit 1
fi

mkdir -p "$REFS_DIR"

#	capture <name> <args...>
capture()
{
	name="$1"
	shift

	printf "${PY}%-22s${NC}" "$name"

	tmp="$(mktemp)"

	#	set +e: a non-zero exit is expected for several cases and must not
	#	abort the script. $? is read immediately, before anything else runs
	set +e
	"$REF_PING" "$@" > "$tmp" 2>&1
	code=$?
	set -e

	cat -A "$tmp" > "${REFS_DIR}/${name}.cat"
	printf '%s\n' "$code" > "${REFS_DIR}/${name}.code"
	rm -f "$tmp"

	printf "${PG}✔ ${NC} exit=%s\n" "$code"
}

#————————————————————————————————————————————————————————
#	NO PACKET SENT
#————————————————————————————————————————————————————————

capture no_args
capture bad_host	-c1 nonexistent.invalid

#————————————————————————————————————————————————————————
#	OPTION PARSING ERRORS
#————————————————————————————————————————————————————————

capture invalid_option		-Z 127.0.0.1
capture unrecognized		--hello 127.0.0.1

#	A missing argument is worded differently for a short and a long
#	option, and a long option can also refuse an argument it does not take
capture missing_arg_short	-c
capture missing_arg_long	--count
capture missing_arg_ttl		--ttl
capture extra_arg_long		--verbose=1 127.0.0.1

#————————————————————————————————————————————————————————
#	OPTION VALUES
#————————————————————————————————————————————————————————

capture invalid_value			-c abc 127.0.0.1
capture invalid_value_partial	-c 127.0.0.1

#	Numbers are read in base 0: a 0x prefix means hex, a leading 0 octal,
#	so 8 is not a valid digit after it
capture count_hex		-c 0x2 127.0.0.1
capture count_octal		-c 08 127.0.0.1

capture ttl_zero		--ttl 0 127.0.0.1
capture ttl_too_big		--ttl 256 127.0.0.1

#	A minus sign is not rejected: the value wraps around to a huge number
capture ttl_negative	--ttl -1 127.0.0.1

#	The limit is 65535 minus the largest IP and ICMP headers (60 + 76)
capture size_too_big	-s 65400 127.0.0.1

#————————————————————————————————————————————————————————
#	MANDATORY OPTIONS
#————————————————————————————————————————————————————————

#	Format references only: ft_ping lists its own options, so these never
#	diff clean. Kept for argp indentation, column alignment and wrapping
capture help		'-?'
capture usage		--usage

capture verbose		-c2 -v 127.0.0.1

#	-v on an ICMP error
capture verbose_unreach		-c1 -v 10.0.2.99

#————————————————————————————————————————————————————————
#	BONUS OPTIONS
#————————————————————————————————————————————————————————

capture size			-c2 -s 100 127.0.0.1

#	Below 16 bytes the payload cannot carry the timestamp: no RTT shown
capture size_small		-c2 -s 8 127.0.0.1
capture size_max		-c1 -s 65399 127.0.0.1

capture verbose_ttl		-c1 -v --ttl 1 8.8.8.8
capture timeout			-w 1 192.0.2.1
capture linger			-c1 -W 1 192.0.2.1

#————————————————————————————————————————————————————————
#	NOMINAL
#————————————————————————————————————————————————————————

capture loopback	-c2 127.0.0.1
capture public_ip	-c2 8.8.8.8

#	Local alias from /etc/hosts: exercises the resolution path against a
#	fixed address, unlike a public FQDN whose answer varies between runs
capture fqdn		-c2 ftping.test
capture fqdn_public	-c2 google.com

#————————————————————————————————————————————————————————
#	FAILURE BRANCHES
#————————————————————————————————————————————————————————

#	Silent target: 192.0.2.0/24 is TEST-NET-1 (RFC 5737 paragraph 3),
#	reserved for documentation, no router forwards it, so packets are
#	silently dropped
capture silent	-c2 192.0.2.1

#	ICMP type 11 code 0 "Time Exceeded" (RFC 792):
#	--ttl 1 makes the datagram expire at the first hop, the NAT gateway,
#	which reports it: the packet never leaves the VM
capture ttl_exceeded	-c2 --ttl 1 8.8.8.8

#	ICMP type 3 code 1 "Destination Unreachable" (RFC 792):
#	10.0.2.99 sits in the NAT subnet but nobody answers its ARP requests,
#	so the VM's own kernel gives up and reports the error: it comes from
#	10.0.2.15, the VM itself, not from a router
capture host_unreach	-c2 10.0.2.99

#	198.51.100.0/24 is TEST-NET-2 (RFC 5737 paragraph 3). setup.sh adds an
#	iptables OUTPUT REJECT rule for this address: since the rejection
#	happens locally (not at a remote router), it surfaces as EPERM from
#	sendto() - no ICMP error packet is generated.
#	Exercises the send-failure branch of _send_echo_request()
capture send_denied		-c2 198.51.100.7

printf "\n${BOLD}${PINK}=============  CORPUS ${BLINK}READY${NC}${BOLD}${PINK}  =============${NC}\n\n"
printf "${PY}Location:${NC} %s\n" "$REFS_DIR"