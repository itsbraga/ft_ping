#!/bin/sh

. "$(dirname "$0")/colors.sh"

#	Stop the script on any error
set -e

INET_VERSION="2.0"
INET_TARBALL="inetutils-${INET_VERSION}.tar.xz"
INET_URL="https://ftp.gnu.org/gnu/inetutils/${INET_TARBALL}"
INET_PREFIX="/opt/inetutils"
INET_PING="${INET_PREFIX}/bin/ping"

#	Filled in after the first signature-verified download
#	GPG proves the tarball really comes from GNU; a hash only proves it has
#	not changed since
INET_SHA256="e573d566e55393940099862e7f8994164a0ed12f5a86c3345380842bdc124722"

#	Stable FQDN target: keeps the name-resolution test case reproducible
#	without depending on public DNS answers changing between captures
TEST_FQDN="ftping.test"

#	RFC 5737 TEST-NET-2, never routed
TEST_DENIED_IP="198.51.100.7"

BUILD_DIR="/tmp/inetutils_build"

printf "${BOLD}${PP}=======  ft_ping VM PROVISIONNING  =======${NC}\n\n"

if [ "$(id -u)" -ne 0 ]; then
	printf "${BOLD}${RED}Error:${NC} this script must be run as root (sudo)\n"
	exit 1
fi

#————————————————————————————————————————————————————————
#	Step 1: PACKAGES
#————————————————————————————————————————————————————————

printf "${PB}[1/6] Installing packages...${NC}\n"
apt update -qq
DEBIAN_FRONTEND=noninteractive apt install -y -qq \
	build-essential \
	wget \
	ca-certificates \
	gpgv \
	iptables \
	xz-utils \
	openssh-server \
	tcpdump \
	valgrind \
	git \
	manpages \
	manpages-dev
printf "${PG}✔ Packages installed${NC}\n"

#————————————————————————————————————————————————————————
#	Step 2: SSH
#————————————————————————————————————————————————————————

printf "\n${PB}[2/6] Enabling SSH daemon...${NC}\n"
systemctl enable --now ssh
printf "${PG}✔ SSH listening on port 22${NC}\n"

#————————————————————————————————————————————————————————
#	Step 3: TEST ENVIRONMENT
#————————————————————————————————————————————————————————

printf "\n${PB}[3/6] Setting up test targets...${NC}\n"

#	grep guard: the script is re-runnable, /etc/hosts must not accumulate
if ! grep -q "$TEST_FQDN" /etc/hosts; then
	printf '127.0.0.1\t%s\n' "$TEST_FQDN" >> /etc/hosts
fi
printf "${PG}✔ ${NC}%s resolves to 127.0.0.1\n" "$TEST_FQDN"

#	-C tests for the rule first: -I would add a duplicate on every run
if ! iptables -C OUTPUT -d "$TEST_DENIED_IP" -j REJECT \
		--reject-with icmp-host-unreachable 2>/dev/null; then
	iptables -I OUTPUT -d "$TEST_DENIED_IP" -j REJECT \
		--reject-with icmp-host-unreachable
fi
printf "${PG}✔ ${NC}%s denied at send time\n" "$TEST_DENIED_IP"

#————————————————————————————————————————————————————————
#	Step 4: DOWNLOAD AND VERIFY
#————————————————————————————————————————————————————————

printf "\n${PB}[4/6] Downloading inetutils-${INET_VERSION}...${NC}\n"

rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

wget -q "$INET_URL"
wget -q "${INET_URL}.sig"
wget -q -O gnu-keyring.gpg "https://ftp.gnu.org/gnu/gnu-keyring.gpg"

printf "${PY}Verifying signature...${NC}\n"

#	gpgv over gpg --verify: no keyring mutation, no trust database,
#	and the exit status alone is the verdict
if ! gpgv --keyring ./gnu-keyring.gpg "${INET_TARBALL}.sig" "$INET_TARBALL" \
		2>/dev/null; then
	printf "${BOLD}${RED}Error:${NC} signature verification failed\n"
	exit 1
fi
printf "${PG}✔ Signature verified against the GNU keyring${NC}\n"

if [ -n "$INET_SHA256" ]; then
	printf '%s  %s\n' "$INET_SHA256" "$INET_TARBALL" | sha256sum -c - > /dev/null
	printf "${PG}✔ Checksum matches the pinned value${NC}\n"
else
	printf "${PY}No pinned checksum, sha256 is:${NC} %s\n" \
		"$(sha256sum "$INET_TARBALL" | cut -d' ' -f1)"
fi

#————————————————————————————————————————————————————————
#	Step 5: BUILD
#————————————————————————————————————————————————————————

printf "\n${PB}[5/6] Building...${NC}\n"

printf "${PY}Extracting...${NC}\n"
tar xf "$INET_TARBALL"
cd "inetutils-${INET_VERSION}"

#	--disable-servers: only ping is needed, not ftpd/telnetd/rlogind/...
#	CFLAGS passed to configure (not make) so its internal probes use them too
printf "${PY}Configuring...${NC}\n"
./configure --prefix="$INET_PREFIX" --disable-servers \
	CFLAGS="-O2 -g" > /dev/null

printf "${PY}Compiling...${NC}\n"
make -j"$(nproc)" > /dev/null
make install > /dev/null
printf "${PG}✔ inetutils installed in ${INET_PREFIX}${NC}\n"

cd /
rm -rf "$BUILD_DIR"

#————————————————————————————————————————————————————————
#	Step 6: VERIFICATION
#————————————————————————————————————————————————————————

printf "\n${PB}[6/6] Verifying reference...${NC}\n"

#	Necessary check: A wrong reference silently invalidates every output comparison made afterwards
VERSION_LINE=$("$INET_PING" -V 2>&1 | head -n 1)
if ! printf '%s' "$VERSION_LINE" | grep -qi "inetutils"; then
	printf "${BOLD}${RED}Error:${NC} not inetutils, got: %s\n" "$VERSION_LINE"
	exit 1
fi
if ! printf '%s' "$VERSION_LINE" | grep -q "$INET_VERSION"; then
	printf "${BOLD}${RED}Error:${NC} wrong version, got: %s\n" "$VERSION_LINE"
	exit 1
fi

#	The reference branches on getuid(), so captures are only valid if it is
#	setuid-root and invoked unprivileged. Checked here, enforced at capture time
if [ ! -u "$INET_PING" ]; then
	printf "${BOLD}${RED}Error:${NC} reference is not setuid-root\n"
	exit 1
fi
printf "${PG}✔ Reference validated: %s${NC}\n" "$VERSION_LINE"

printf "\n${BOLD}${PINK}===========  ENVIRONMENT ${BLINK}READY${NC}${BOLD}${PINK}  ==========${NC}\n\n"
printf "${PY}Reference:${NC} %s\n" "$INET_PING"