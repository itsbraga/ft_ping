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

#	Silent target: 192.0.2.0/24 is TEST-NET-1 (RFC 5737 §3), reserved for
#	documentation — no router forwards it, so packets are silently dropped
capture silent		-c2 192.0.2.1

#	ICMP type 11 code 0 "Time Exceeded" (RFC 792 §3.3.3):
#	a router MUST discard a datagram whose TTL reaches zero and SHOULD
#	send a Time Exceeded message back to the source.
#	--ttl 1 forces expiry at the first hop (the NAT gateway), so the
#	packet never leaves the VM and a reply always comes back
capture ttl_exceeded	-c2 --ttl 1 8.8.8.8

#	ICMP type 3 "Destination Unreachable" (RFC 792 §3.3.1):
#	16 sub-codes (RFC 792 + RFC 1812 §5.2.7.1). Triggering one requires
#	a router to actually send an error back, which is not achievable in
#	a simple NAT VM without network namespaces or iptables REJECT on a
#	FORWARD chain. The display strings come straight from the RFCs and
#	share the same output format as type 11 (verified above)

#	198.51.100.0/24 is TEST-NET-2 (RFC 5737 §3). setup.sh adds an
#	iptables OUTPUT REJECT rule for this address: since the rejection
#	happens locally (not at a remote router), it surfaces as EPERM from
#	sendto() — no ICMP error packet is generated.
#	Exercises the send-failure branch of _send_echo_request()
capture send_denied		-c2 198.51.100.7

printf "\n${BOLD}${PINK}=============  CORPUS ${BLINK}READY${NC}${BOLD}${PINK}  =============${NC}\n\n"
printf "${PY}Location:${NC} %s\n" "$REFS_DIR"