/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   display.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: annabrag <annabrag@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/07 15:54:02 by annabrag          #+#    #+#             */
/*   Updated: 2026/09/22 17:56:12 by annabrag         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_ping.h"

static void	_display_try_help( const char *prog_name )
{
	fprintf(stderr, "Try '%s --help' or '%s --usage' for more information.\n",
		short_name(prog_name), short_name(prog_name));
}

void	display_missing_host( const char *prog_name )
{
	fprintf(stderr, "%s: missing host operand\n", short_name(prog_name));
	_display_try_help(prog_name);
}

void	display_invalid_option( const char *prog_name, int opt )
{
	fprintf(stderr, "%s: invalid option -- '%c'\n", prog_name, opt);
	_display_try_help(prog_name);
}

void	display_unrecognized_option( const char *prog_name, const char *arg )
{
	fprintf(stderr, "%s: unrecognized option '%s'\n", prog_name, arg);
	_display_try_help(prog_name);
}

void	display_missing_argument( const char *prog_name, int opt )
{
	fprintf(stderr, "%s: option requires an argument -- '%c'\n", prog_name, opt);
	_display_try_help(prog_name);
}

void	display_invalid_value( const char *prog_name, const char *value,
			const char *near )
{
	fprintf(stderr, "%s: invalid value (`%s' near `%s')\n", prog_name, value, near);
}

void	display_help( const char *prog_name )
{
	printf("Usage: %s [OPTION...] HOST ...\n", short_name(prog_name));
	fputs(
		"Send ICMP ECHO_REQUEST packets to network hosts.\n"
		"\n"
		" Options valid for all request types:\n"
		"\n"
		"  -c, --count=NUMBER         stop after sending NUMBER packets\n"
		"  -n, --numeric              do not resolve host addresses\n"
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
	printf("%zd bytes from %s: icmp_seq=%u ttl=%u", icmp_size, addr, seq, ttl);
	if (rtt)
		printf(" time=%.3f ms", *rtt);
	printf("\n");
}
	
void	display_stats( const char *host, const t_stats *s, size_t payload_size )
{
	double	avg;

	printf("--- %s ping statistics ---\n", host);
	printf("%zu packets transmitted, %zu packets received, ", s->transmitted, s->received);
	if (s->transmitted != 0)
		printf("%d%% packet loss", (int)(((s->transmitted - s->received) * 100) / s->transmitted));
	printf("\n");
	if (s->received && payload_size >= sizeof(struct timeval))
	{
		avg = s->rtt_sum / s->received;
		printf("round-trip min/avg/max/stddev = %.3f/%.3f/%.3f/%.3f ms\n",
			s->rtt_min, avg, s->rtt_max, stats_stddev(s));
	}
}