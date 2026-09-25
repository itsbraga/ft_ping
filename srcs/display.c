/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   display.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: panther <panther@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/07 15:54:02 by annabrag          #+#    #+#             */
/*   Updated: 2026/09/24 00:15:18 by panther          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_ping.h"

static void	_display_try_help( const char *prog )
{
	fprintf(stderr, "Try '%s --help' or '%s --usage' for more information.\n",
		prog, prog);
}

void	display_missing_host( const char *prog )
{
	fprintf(stderr, "%s: missing host operand\n", prog);
	_display_try_help(prog);
}

void	display_invalid_option( const char *prog, int opt )
{
	fprintf(stderr, "%s: invalid option -- '%c'\n", prog, opt);
	_display_try_help(prog);
}

void	display_unrecognized_option( const char *prog, const char *arg )
{
	fprintf(stderr, "%s: unrecognized option '%s'\n", prog, arg);
	_display_try_help(prog);
}

void	display_missing_argument( const char *prog, int opt )
{
	fprintf(stderr, "%s: option requires an argument -- '%c'\n", prog, opt);
	_display_try_help(prog);
}

void	display_invalid_value( const char *prog, const char *value,
			const char *near )
{
	fprintf(stderr, "%s: invalid value (`%s' near `%s')\n", prog, value, near);
}

void	display_value_too_small( const char *prog, const char *arg )
{
	fprintf(stderr, "%s: option value too small: %s\n", prog, arg);
}

void	display_value_too_big( const char *prog, const char *arg )
{
	fprintf(stderr, "%s: option value too big: %s\n", prog, arg);
}

static const char	*_unreach_str( uint8_t code )
{
	static const char	*msg[] = {
		"Destination Net Unreachable",
		"Destination Host Unreachable",
		"Destination Protocol Unreachable",
		"Destination Port Unreachable",
		"Frag needed and DF set",
		"Source Route Failed",
		"Destination Net Unknown",
		"Destination Host Unknown",
		"Source Host Isolated",
		"Destination Net Prohibited",
		"Destination Host Prohibited",
		"Destination Net Unreachable for TOS",
		"Destination Host Unreachable for TOS",
		"Communication Administratively Prohibited",
		"Host Precedence Violation",
		"Precedence Cutoff in Effect",
	};

	if (code < sizeof(msg) / sizeof(msg[0]))
		return (msg[code]);
	return ("Dest Unreachable, Unknown Code");
}

void	display_icmp_error( const struct sockaddr_in *from, size_t icmp_size,
			uint8_t type, uint8_t code )
{
	char	addr[INET_ADDRSTRLEN];

	inet_ntop(AF_INET, &from->sin_addr, addr, sizeof(addr));
	printf("%zu bytes from %s: ", icmp_size, addr);
	if (type == ICMP_DEST_UNREACH)
		printf("%s", _unreach_str(code));
	else if (type == ICMP_TIME_EXCEEDED)
	{
		if (code == 0)
			printf("Time to live exceeded");
		else
			printf("Frag reassembly time exceeded");
	}
	printf("\n");
}

void	display_help( const char *prog )
{
	printf("Usage: %s [OPTION...] HOST\n", prog);
	fputs(
		"Send ICMP ECHO_REQUEST packets to network hosts.\n"
		"\n"
		" Options valid for all request types:\n"
		"\n"
		"  -c, --count=NUMBER         stop after sending NUMBER packets\n"
		"      --ttl=N                specify N as time-to-live\n"
		"  -v, --verbose              verbose output\n"
		"  -w, --timeout=N            stop after N seconds\n"
		"  -W, --linger=N             number of seconds to wait for response\n"
		"\n"
		" Options valid for --echo requests:\n"
		"\n"
		"  -s, --size=NUMBER          send NUMBER data octets\n"
		"\n"
		"  -?, --help                 give this help list\n"
		"\n"
		"Mandatory or optional arguments to long options are also mandatory or optional\n"
		"for any corresponding short options.\n",
		stdout);
}

void	display_target_info( t_ping *p )
{
	printf("PING %s (%s): %zu data bytes", p->hostname, p->target_ip_str, p->opts.size);
	if (p->opts.verbose)
		printf(", id 0x%04x = %u", p->id, p->id);

	printf("\n");
}

void	display_reply( const struct sockaddr_in *from, size_t icmp_size, uint16_t seq,
			uint8_t ttl, const double *rtt )
{
	char	addr[INET_ADDRSTRLEN];

	inet_ntop(AF_INET, &from->sin_addr, addr, sizeof(addr));
	printf("%zu bytes from %s: icmp_seq=%u ttl=%u", icmp_size, addr, seq, ttl);
	if (rtt)
		printf(" time=%.3f ms", *rtt);
	printf("\n");
}
	
void	display_stats( const char *host, const t_stats *s, size_t payload_size )
{
	double	avg;

	printf("--- %s ping statistics ---\n", host);
	printf("%zu packets transmitted, ", s->transmitted);
	printf("%zu packets received, ", s->received);
	if (s->transmitted != 0)
	{
		if (s->received > s->transmitted)
			printf("0%% packet loss");
		else
			printf("%d%% packet loss",
				(int)(((s->transmitted - s->received) * 100) / s->transmitted));
	}
	printf("\n");
	if (s->received && payload_size >= sizeof(struct timeval))
	{
		avg = s->rtt_sum / s->received;
		printf("round-trip min/avg/max/stddev = %.3f/%.3f/%.3f/%.3f ms\n",
			s->rtt_min, avg, s->rtt_max, stats_stddev(s));
	}
}