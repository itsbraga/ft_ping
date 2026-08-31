#!/bin/sh
. "$(dirname "$0")/colors.sh"

# Stop the script on any error
set -e

#	Captures the inetutils-2.0 reference output for every test case.
#	Three files per case:
#		.out   raw output (stdout + stderr merged)
#		.cat   same, through cat -A: trailing spaces and tabs made visible
#		.code  exit status (0 success, 64 usage error, 1 runtime failure)

REF_PING="/opt/inetutils/bin/ping"
REFS_DIR="refs"

printf "${BOLD}${PP}===== Reference Corpus Capture =====${NC}\n"

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

	printf "${PY}%-16s${NC}" "$name"

	#	set +e: a non-zero exit is expected for several cases and must not
	#	abort the script. $? is read immediately, before anything else runs.
	set +e
	"$REF_PING" "$@" > "${REFS_DIR}/${name}.out" 2>&1
	echo $? > "${REFS_DIR}/${name}.code"
	set -e

	cat -A "${REFS_DIR}/${name}.out" > "${REFS_DIR}/${name}.cat"

	printf "${PG}✔ ${NC} exit=%s\n" "$(cat "${REFS_DIR}/${name}.code")"
}

#————————————————————————————————————————————————————————
#	NO PACKET SENT
#————————————————————————————————————————————————————————

capture help         -?
capture no_args
capture bad_host     -c1 nonexistent.invalid

#————————————————————————————————————————————————————————
#	NOMINAL
#————————————————————————————————————————————————————————

capture loopback     -c2 127.0.0.1
capture public_ip    -c2 8.8.8.8
capture fqdn         -c2 google.com
capture verbose      -c2 -v 127.0.0.1

#————————————————————————————————————————————————————————
#	FAILURE BRANCHES
#————————————————————————————————————————————————————————

#	Silent target: RFC 5737 documentation range, never routed.
capture unreachable  -c2 192.0.2.1

#	ICMP error branch: gateway replies Time to live exceeded.
capture ttl_exceeded -c2 --ttl 1 8.8.8.8

printf "\n${BOLD}${PINK}=========== Corpus Ready ===========${NC}\n"
printf "${PY}Location:${NC} %s/\n" "$REFS_DIR"
printf "${PY}Read the .cat files, not the .out ones.${NC}\n"
