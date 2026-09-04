#!/bin/sh
. "$(dirname "$0")/colors.sh"

#	Stop the script on any error
set -e

INET_VERSION="2.0"
INET_TARBALL="inetutils-${INET_VERSION}.tar.xz"
INET_URL="https://ftp.gnu.org/gnu/inetutils/${INET_TARBALL}"
INET_PREFIX="/opt/inetutils"
INET_PING="${INET_PREFIX}/bin/ping"

BUILD_DIR="/tmp/inetutils_build"

RELAX_CFLAGS=""
#RELAX_CFLAGS="-Wno-error=implicit-function-declaration \
#-Wno-error=implicit-int -Wno-error=int-conversion \
#-Wno-error=incompatible-pointer-types"

printf "${BOLD}${PP}=== ft_ping VM Provisioning ===${NC}\n"

if [ "$(id -u)" -ne 0 ]; then
	printf "${BOLD}${RED}Error:${NC} this script must be run as root (sudo)\n"
	exit 1
fi

#————————————————————————————————————————————————————————
#	Step 1: PACKAGES
#————————————————————————————————————————————————————————

printf "${PB}[1/4] Installing packages...${NC}\n"
apt update -qq
DEBIAN_FRONTEND=noninteractive apt install -y -qq \
	build-essential \
	wget \
	ca-certificates \
	xz-utils \
	openssh-server \
	tcpdump \
	gdb \
	valgrind \
	git \
	libcap2-bin \
	manpages \
	manpages-dev
printf "${PG}✔ Packages installed${NC}\n"

#————————————————————————————————————————————————————————
#	Step 2: SSH
#————————————————————————————————————————————————————————

#	Host-side counterpart: VirtualBox NAT port forwarding 127.0.0.1:2222 -> :22
printf "${PB}[2/4] Enabling SSH daemon...${NC}\n"
systemctl enable --now ssh
printf "${PG}✔ SSH listening on port 22${NC}\n"

#————————————————————————————————————————————————————————
#	Step 3: INETUTILS BUILD
#————————————————————————————————————————————————————————

printf "${PB}[3/4] Building inetutils-${INET_VERSION}...${NC}\n"

rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

printf "${PY}Downloading...${NC}\n"
wget -q "$INET_URL"

printf "${PY}Extracting...${NC}\n"
tar xf "$INET_TARBALL"
cd "inetutils-${INET_VERSION}"

#	--disable-servers: only ping is needed, not ftpd/telnetd/rlogind/...
#	CFLAGS passed to configure (not make) so its internal probes use them too
printf "${PY}Configuring...${NC}\n"
./configure --prefix="$INET_PREFIX" --disable-servers \
	CFLAGS="-O2 -g ${RELAX_CFLAGS}" > /dev/null

printf "${PY}Compiling...${NC}\n"
make -j"$(nproc)" > /dev/null
make install > /dev/null
printf "${PG}✔ inetutils installed in ${INET_PREFIX}${NC}\n"

rm -rf "$BUILD_DIR"

#————————————————————————————————————————————————————————
#	Step 4: CAPABILITIES & VERIFICATION
#————————————————————————————————————————————————————————

printf "${PB}[4/4] Setting capabilities and verifying reference...${NC}\n"

#	CAP_NET_RAW alone, instead of running the reference through sudo
setcap cap_net_raw+ep "$INET_PING"

#	Necessary check: A wrong reference silently invalidates every output comparison made afterwards.
VERSION_LINE=$("$INET_PING" -V 2>&1 | head -n 1)
if ! printf '%s' "$VERSION_LINE" | grep -qi "inetutils"; then
	printf "${BOLD}${RED}Error:${NC} not inetutils, got: %s\n" "$VERSION_LINE"
	exit 1
fi
if ! printf '%s' "$VERSION_LINE" | grep -q "$INET_VERSION"; then
	printf "${BOLD}${RED}Error:${NC} wrong version, got: %s\n" "$VERSION_LINE"
	exit 1
fi
printf "${PG}✔ Reference validated: %s${NC}\n" "$VERSION_LINE"

printf "\n${BOLD}${PINK}=== Environment Ready ===${NC}\n"
printf "${PY}Reference:${NC} %s\n" "$INET_PING"
printf "${PY}Never add ${INET_PREFIX}/bin to PATH, call it by absolute path.${NC}\n"