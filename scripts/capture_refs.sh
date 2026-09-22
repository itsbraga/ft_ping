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

capture invalid_option	-Z 127.0.0.1
capture unrecognized	--hello 127.0.0.1

capture invalid_value			-c abc 127.0.0.1
capture invalid_value_partial	-c 127.0.0.1

#————————————————————————————————————————————————————————
#	MANDATORY OPTIONS
#————————————————————————————————————————————————————————

#	Format reference only: ft_ping lists its own options, so this never
#	diffs clean. Kept for argp indentation and column alignment
capture help		'-?'
capture verbose		-c2 -v 127.0.0.1

#————————————————————————————————————————————————————————
#	BONUS OPTIONS
#————————————————————————————————————————————————————————

capture size		-c2 -s 100 127.0.0.1
capture timeout		-w 1 192.0.2.1
capture linger		-c1 -W 1 192.0.2.1

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

#	Silent target: RFC 5737 documentation range, never routed
capture silent			-c2 192.0.2.1

#	ICMP type 11: the NAT gateway replies Time to live exceeded
#	Expires at the first hop, so the packet never leaves the VM
capture ttl_exceeded	-c2 --ttl 1 8.8.8.8

#	OUTPUT REJECT on a locally generated packet surfaces as EPERM from
#	sendto(), never as an ICMP packet: exercises the send failure branch
capture send_denied		-c2 198.51.100.7

printf "\n${BOLD}${PINK}=============  CORPUS READY  =============${NC}\n\n"
printf "${PY}Location:${NC} %s\n" "$REFS_DIR"