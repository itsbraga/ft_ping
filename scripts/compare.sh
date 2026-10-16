#!/bin/bash

#	Runs ft_ping and the inetutils-2.0 reference side by side on every test
#	case and diffs their output and exit status.
#
#	Both programs are launched with argv[0] set to "ping", so that error
#	messages compare equal. What legitimately changes between two runs is
#	masked before the diff: round-trip times, the identifier (derived from
#	the PID), and the ID and checksum fields of a quoted IP header.
#
#	usage: scripts/compare.sh [case-name-filter]

REF_PING="${REF_PING:-/opt/inetutils/bin/ping}"
SCRIPT_DIR="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"
ROOT_DIR="$(dirname -- "$SCRIPT_DIR")"
FT_PING="${FT_PING:-${ROOT_DIR}/ft_ping}"
OUT_DIR="${OUT_DIR:-$(mktemp -d)}"
FILTER="${1:-}"

#	Targets, overridable from the environment for another topology
SILENT="${SILENT:-192.0.2.1}"				#	never answers
TTL_TARGET="${TTL_TARGET:-8.8.8.8}"			#	--ttl 1 expires at the gateway
NO_ARP="${NO_ARP:-10.0.2.99}"				#	on-link, nobody answers ARP
DENIED="${DENIED:-198.51.100.7}"			#	iptables OUTPUT REJECT
FQDN="${FQDN:-ftping.test}"					#	/etc/hosts alias
BCAST="${BCAST:-255.255.255.255}"

. "${SCRIPT_DIR}/colors.sh"

if [ "$(id -u)" -eq 0 ]; then
	printf "${BOLD}${RED}Error:${NC} run unprivileged, like the setuid binaries it compares\n"
	exit 1
fi
for bin in "$REF_PING" "$FT_PING"; do
	if [ ! -x "$bin" ] || [ ! -u "$bin" ]; then
		printf "${BOLD}${RED}Error:${NC} %s missing or not setuid (make setuid)\n" "$bin"
		exit 1
	fi
done

pass=0
fail=0

#	Masks what differs between two runs that are both correct
normalize()
{
	sed -E \
		-e 's/time=[0-9.]+ ms/time=T ms/' \
		-e 's#= [0-9.]+/[0-9.]+/[0-9.]+/[0-9.]+ ms#= R ms#' \
		-e 's/id 0x[0-9a-f]{4} = [0-9]+/id 0xID = N/' \
		-e 's/id 0x[0-9a-f]{4}, seq/id 0xID, seq/' \
		-e 's/^ ([0-9a-f]{4} [0-9a-f]{4}) [0-9a-f]{4} ([0-9a-f]{4} [0-9a-f]{4}) [0-9a-f]{4} / \1 IDID \2 CKSM /' \
		-e 's/^( [0-9a-f]  [0-9a-f]  [0-9a-f]{2} [0-9a-f]{4}) [0-9a-f]{4}(   [0-9a-f] [0-9a-f]{4}  [0-9a-f]{2}  [0-9a-f]{2}) [0-9a-f]{4} /\1 IDID\2 CKSM /' \
		"$1" | cat -A
}

#	run <binary> <output file> <args...>
run()
{
	local bin="$1" out="$2"
	shift 2
	timeout 40 bash -c 'exec -a ping "$@"' _ "$bin" "$@" > "$out" 2>&1
	echo $? > "${out}.code"
}

#	check <name> <args...>
check()
{
	local name="$1"
	shift
	if [ -n "$FILTER" ] && [[ "$name" != *"$FILTER"* ]]; then
		return
	fi
	run "$REF_PING" "${OUT_DIR}/${name}.ref" "$@"
	#	ICMP errors are rate-limited per destination: let the bucket refill
	sleep 1
	run "$FT_PING" "${OUT_DIR}/${name}.ft" "$@"
	local ref_code ft_code
	ref_code="$(cat "${OUT_DIR}/${name}.ref.code")"
	ft_code="$(cat "${OUT_DIR}/${name}.ft.code")"
	if diff <(normalize "${OUT_DIR}/${name}.ref") <(normalize "${OUT_DIR}/${name}.ft") \
			> "${OUT_DIR}/${name}.diff" && [ "$ref_code" = "$ft_code" ]; then
		printf "${PG}OK  ${NC} %-24s exit=%s\n" "$name" "$ref_code"
		pass=$((pass + 1))
	else
		printf "${RED}DIFF${NC} %-24s exit ref=%s ft=%s\n" "$name" "$ref_code" "$ft_code"
		sed 's/^/       /' "${OUT_DIR}/${name}.diff" | head -20
		fail=$((fail + 1))
	fi
}

printf "${BOLD}${PP}=========  ft_ping vs inetutils-2.0  =========${NC}\n\n"

#	Parsing
check no_args
check bad_host				-c1 nonexistent.invalid
check invalid_option		-Z 127.0.0.1
check unrecognized			--hello 127.0.0.1
check missing_arg_short		-c
check missing_arg_long		--count
check missing_arg_ttl		--ttl
check extra_arg_long		--verbose=1 127.0.0.1
check abbreviation			--cou=1 127.0.0.1
check grouped_short			-vc1 127.0.0.1
check host_first			127.0.0.1 -c 1
check double_dash			-c1 -- 127.0.0.1
check empty_host			-c1 ''

#	Values
check invalid_value			-c abc 127.0.0.1
check invalid_value_partial	-c 127.0.0.1
check count_hex				-c 0x2 127.0.0.1
check count_octal			-c 08 127.0.0.1
check ttl_zero				--ttl 0 127.0.0.1
check ttl_too_big			--ttl 256 127.0.0.1
check ttl_negative			--ttl -1 127.0.0.1
check size_too_big			-s 65400 127.0.0.1
check timeout_zero			-w 0 127.0.0.1
check timeout_too_big		-w 2147483648 127.0.0.1
check linger_zero			-W 0 127.0.0.1
check linger_too_big		-W 2147483648 127.0.0.1

#	Nominal
check loopback				-c2 127.0.0.1
check verbose				-c2 -v 127.0.0.1
check localhost				-c1 localhost
check zero_address			-c1 0.0.0.0
check fqdn					-c2 "$FQDN"
check broadcast				-c1 "$BCAST"

#	Size
check size					-c2 -s 100 127.0.0.1
check size_zero				-c1 -s 0 127.0.0.1
check size_small			-c2 -s 8 127.0.0.1
check size_timestamp_only	-c1 -s 16 127.0.0.1
check size_max				-c1 -s 65399 127.0.0.1

#	Timing
check timeout				-w 2 127.0.0.1
check timeout_silent		-w 2 "$SILENT"
check linger				-c1 -W 1 "$SILENT"
check silent				-c2 "$SILENT"

#	ICMP errors
check ttl_exceeded			-c2 --ttl 1 "$TTL_TARGET"
check verbose_ttl			-c1 -v --ttl 1 "$TTL_TARGET"
check host_unreach			-c2 "$NO_ARP"
check verbose_unreach		-c1 -v "$NO_ARP"
check unreach_net			-c1 127.0.0.10
check unreach_host			-c1 127.0.0.11
check unreach_proto			-c1 127.0.0.12
check unreach_port			-c1 127.0.0.13
check unreach_net_prohib	-c1 127.0.0.19
check unreach_host_prohib	-c1 127.0.0.20
check unreach_admin			-c1 127.0.0.23
check verbose_admin			-c1 -v 127.0.0.23

#	Send failure
check send_denied			-c2 "$DENIED"

printf "\n${BOLD}%d OK, %d DIFF${NC}  (outputs kept in %s)\n" "$pass" "$fail" "$OUT_DIR"
[ "$fail" -eq 0 ]