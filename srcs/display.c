/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   display.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: annabrag <annabrag@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/07 15:54:02 by annabrag          #+#    #+#             */
/*   Updated: 2026/09/27 23:04:36 by annabrag         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_ping.h"

static void	_display_try_help( void )
{
	fprintf(stderr, "Try '%s --help' or '%s --usage' for more information.\n",
		PROG_NAME, PROG_NAME);
}

void	display_missing_host( void )
{
	fprintf(stderr, "%s: missing host operand\n", PROG_NAME);
	_display_try_help();
}

void	display_invalid_option( int arg )
{
	fprintf(stderr, "%s: invalid option -- '%c'\n", PROG_NAME, arg);
	_display_try_help();
}

void	display_unrecognized_option( const char *name )
{
	fprintf(stderr, "%s: unrecognized option '%s'\n", PROG_NAME, name);
	_display_try_help();
}

void	display_missing_argument( int arg )
{
	fprintf(stderr, "%s: option requires an argument -- '%c'\n", PROG_NAME, arg);
	_display_try_help();
}

void	display_long_missing_argument( const char *name )
{
	fprintf(stderr, "%s: option '--%s' requires an argument\n", PROG_NAME, name);
	_display_try_help();
}

void	display_long_extra_argument( const char *name )
{
	fprintf(stderr, "%s: option '--%s' doesn't allow an argument\n", PROG_NAME, name);
	_display_try_help();
}

void	display_invalid_value( const char *value, const char *near )
{
	fprintf(stderr, "%s: invalid value (`%s' near `%s')\n", PROG_NAME, value, near);
}

void	display_value_too_small( const char *value )
{
	fprintf(stderr, "%s: option value too small: %s\n", PROG_NAME, value);
}

void	display_value_too_big( const char *value )
{
	fprintf(stderr, "%s: option value too big: %s\n", PROG_NAME, value);
}

void	display_icmp_error( const struct sockaddr_in *from, size_t icmp_size,
			uint8_t type, uint8_t code )
{
	char	addr[INET_ADDRSTRLEN];

	inet_ntop(AF_INET, &from->sin_addr, addr, sizeof(addr));
	printf("%zu bytes from %s: ", icmp_size, addr);
	if (type == ICMP_DEST_UNREACH)
		printf("%s", unreach_str(code));
	else if (type == ICMP_TIME_EXCEEDED)
	{
		if (code == 0)
			printf("Time to live exceeded");
		else
			printf("Frag reassembly time exceeded");
	}
	printf("\n");
}

void	display_help( void )
{
	printf("Usage: %s [OPTION...] HOST\n", PROG_NAME);
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
		"      --usage                give a short usage message\n"
		"\n"
		"Mandatory or optional arguments to long options are also mandatory or optional\n"
		"for any corresponding short options.\n",
		stdout);
}

void	display_usage( void )
{
	printf("Usage: %s [-v?] [-c NUMBER] [-w N] [-W N] [-s NUMBER] [--count=NUMBER]\n", PROG_NAME);
	fputs(
		"            [--ttl=N] [--verbose] [--timeout=N] [--linger=N] [--size=NUMBER]\n"
		"            [--help] [--usage]\n"
		"            HOST\n",
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
	if (s->transmitted)
	{
		if (s->received > s->transmitted)
			printf("-- somebody is printing forged packets!");
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