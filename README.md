# ft_ping

A reimplementation of `ping` in C, taking GNU inetutils 2.0 as its reference.
ft_ping sends ICMP Echo Requests over a raw socket and reproduces the
reference's output line for line, round-trip times aside.

## Build and run

```sh
make            # build
make setuid     # hand the binary to root and set the setuid bit
./ft_ping -c 3 example.com
```

A raw ICMP socket requires root. Either run `make setuid` once after each
build, or launch ft_ping with `sudo`.

## Options

| Option | Effect |
|---|---|
| `-v`, `--verbose` | Show the identifier on the `PING` line, and dump the quoted IP header on ICMP errors |
| `-?`, `--help` | Print the help list |
| `--usage` | Print a short usage message |
| `-c`, `--count=N` | Stop after sending N requests |
| `-s`, `--size=N` | Send N data bytes (default 56, maximum 65399) |
| `--ttl=N` | Set the IP time-to-live of outgoing requests |
| `-w`, `--timeout=N` | Stop after N seconds |
| `-W`, `--linger=N` | Wait N seconds for late replies after the last request (default 10) |

Numeric values are read like the reference reads them: a `0x` prefix means
hexadecimal and a leading `0` octal.

Exit status: `0` if at least one reply arrived, `1` if none did or a runtime
or value error occurred, `64` on a usage error.

## Examples

```
$ ./ft_ping -c 2 127.0.0.1
PING 127.0.0.1 (127.0.0.1): 56 data bytes
64 bytes from 127.0.0.1: icmp_seq=0 ttl=64 time=0.026 ms
64 bytes from 127.0.0.1: icmp_seq=1 ttl=64 time=0.024 ms
--- 127.0.0.1 ping statistics ---
2 packets transmitted, 2 packets received, 0% packet loss
round-trip min/avg/max/stddev = 0.024/0.025/0.026/0.000 ms
```

A TTL of 1 makes each request expire at the first router, which answers with
an ICMP error instead of the target:

```
$ ./ft_ping -c 2 --ttl 1 8.8.8.8
PING 8.8.8.8 (8.8.8.8): 56 data bytes
36 bytes from 10.0.2.2: Time to live exceeded
36 bytes from 10.0.2.2: Time to live exceeded
--- 8.8.8.8 ping statistics ---
2 packets transmitted, 0 packets received, 100% packet loss
```

## How it works

**Privileges.** The raw socket is opened first, then `setuid(getuid())`
drops root for good. Name resolution and every packet read from the network
happen afterwards, without privileges. If root cannot be dropped, ft_ping
stops rather than carry on as root.

**Requests.** Each Echo Request carries an 8-byte ICMP header (identifier
derived from the PID, sequence number starting at 0) and a payload made of a
`struct timeval` timestamp followed by an incrementing byte pattern. The
checksum follows RFC 1071. Below 16 data bytes there is no room for the
timestamp, so no round-trip time is shown.

**Event loop.** A single `poll()` waits for whichever comes first: a datagram,
or the next scheduled request, one per second. After the last request, the run
waits `linger` seconds for late replies, and ends early once every request has
been answered. `SIGINT` only sets a flag; the run then ends through the same
path as a normal one, statistics included.

**Reception.** A raw ICMP socket receives every ICMP datagram the host gets,
so every length is checked before the byte it guards is read. Echo Replies are
matched on their identifier. ICMP errors (Destination Unreachable, Time
Exceeded) are matched through the header they quote: protocol, destination,
type and identifier of the original request.

**Statistics.** Minimum, maximum, sum and sum of squares are kept as replies
arrive. The standard deviation is the population one, mean of the squares
minus square of the mean; a variance below 0.0005 is shown as `0.000`, as the
reference shows it.

**Output order.** stdout is line-buffered even when redirected, so messages on
stdout and stderr keep the order in which they were written.

## Conformity testing

Every behaviour is checked against the reference's actual output.

`scripts/setup.sh` provisions the Debian VM: packages, SSH, test targets, and
inetutils 2.0 built from the official GNU tarball once its GPG signature has
been verified against the GNU keyring and its SHA-256 matches a pinned value.

`scripts/capture_refs.sh` records the reference's output for each test case in
`refs/`: a `.cat` file passed through `cat -A`, so trailing spaces and tabs
show, and a `.code` file holding the exit status. It runs unprivileged, like
the setuid reference it captures.

The test targets avoid depending on the outside world where possible:

| Target | What it exercises |
|---|---|
| `192.0.2.1` | RFC 5737 documentation range, never routed: requests go unanswered |
| `--ttl 1 8.8.8.8` | Expires at the VM's NAT gateway: *Time to live exceeded* |
| `10.0.2.99` | No ARP answer on the NAT subnet: *Destination Host Unreachable*, sent by the VM itself |
| `198.51.100.7` | Rejected by a local iptables rule: `sendto()` fails with `EPERM` |
| `ftping.test` | Alias in `/etc/hosts`: name resolution without public DNS |

## Deliberate differences

- `--help` and `--usage` list only the options ft_ping implements, and a single
  `HOST`: the subject asks for one address or hostname.
- Raw socket only. Without privileges, the reference falls back on an
  unprivileged ICMP datagram socket; in that mode it reports a wrong TTL
  (`ttl=150` on the loopback, where the real value is 64). ft_ping does not
  reproduce this fallback and requires root or the setuid bit.
- Duplicate replies are not detected.
- No reverse DNS resolution on reply sources, as the subject requires.
- ICMP error codes the corpus cannot trigger are reported as `Dest Unreachable, Unknown Code: N`, and Time Exceeded errors other than an expired TTL are ignored.

## Layout

```
includes/   headers
srcs/       sources
scripts/    setup.sh, capture_refs.sh
refs/       reference corpus
```