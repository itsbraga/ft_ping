/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   display.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: annabrag <annabrag@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/07 15:54:02 by annabrag          #+#    #+#             */
/*   Updated: 2026/10/12 17:33:46 by annabrag         ###   ########.fr       */
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
	fprintf(stderr, "%s: invalid option -- '%c'\n", PROG_ARGV0, arg);
	_display_try_help();
}

void	display_unrecognized_option( const char *name )
{
	fprintf(stderr, "%s: unrecognized option '%s'\n", PROG_ARGV0, name);
	_display_try_help();
}

void	display_missing_argument( int arg )
{
	fprintf(stderr, "%s: option requires an argument -- '%c'\n", PROG_ARGV0, arg);
	_display_try_help();
}

void	display_long_missing_argument( const char *name )
{
	fprintf(stderr, "%s: option '--%s' requires an argument\n", PROG_ARGV0, name);
	_display_try_help();
}

void	display_long_extra_argument( const char *name )
{
	fprintf(stderr, "%s: option '--%s' doesn't allow an argument\n", PROG_ARGV0, name);
	_display_try_help();
}

void	display_invalid_value( const char *value, const char *near )
{
	fprintf(stderr, "%s: invalid value (`%s' near `%s')\n", PROG_ARGV0, value, near);
}

void	display_value_too_small( const char *value )
{
	fprintf(stderr, "%s: option value too small: %s\n", PROG_ARGV0, value);
}

void	display_value_too_big( const char *value )
{
	fprintf(stderr, "%s: option value too big: %s\n", PROG_ARGV0, value);
}

//	-v: the IP header our error quotes, raw then decoded field by field
static void	_display_quoted_ip( const struct ip *ip )
{
	const uint8_t	*bytes;
	char			src[INET_ADDRSTRLEN];
	char			dest[INET_ADDRSTRLEN];
	size_t			i;

	bytes = (const uint8_t *)ip;
	printf("IP Hdr Dump:\n ");
	i = 0;
	while (i < sizeof(*ip))
	{
		printf("%02x%s", bytes[i], (i % 2) ? " " : "");
		i++;
	}
	printf("\nVr HL TOS  Len   ID Flg  off TTL Pro  cks      Src\tDst\tData\n");
	inet_ntop(AF_INET, &ip->ip_src, src, sizeof(src));
	inet_ntop(AF_INET, &ip->ip_dst, dest, sizeof(dest));
	printf(" %1x  %1x  %02x %04x %04x   %1x %04x  %02x  %02x %04x %s  %s \n",
		ip->ip_v, ip->ip_hl, ip->ip_tos, ntohs(ip->ip_len), ntohs(ip->ip_id),
		(ntohs(ip->ip_off) & 0xE000) >> 13, ntohs(ip->ip_off) & 0x1FFF,
		ip->ip_ttl, ip->ip_p, ntohs(ip->ip_sum), src, dest);
}

void	display_icmp_error( const t_packet *pkt, bool verbose )
{
	char					addr[INET_ADDRSTRLEN];
	const struct ip			*quoted;
	const struct icmphdr	*orig;

	inet_ntop(AF_INET, &pkt->from.sin_addr, addr, sizeof(addr));
	printf("%zu bytes from %s: ", pkt->icmp_size, addr);
	if (pkt->icmp->type == ICMP_TIME_EXCEEDED)
		printf("Time to live exceeded\n");
	else if (unreach_str(pkt->icmp->code))
		printf("%s\n", unreach_str(pkt->icmp->code));
	else
		printf("Dest Unreachable, Unknown Code: %u\n", pkt->icmp->code);
	if (!verbose)
		return ;
	quoted = (const struct ip *)((const uint8_t *)pkt->icmp + ICMP_HDR_SIZE);
	orig = (const struct icmphdr *)((const uint8_t *)quoted + IP_HDR_SIZE(quoted));
	_display_quoted_ip(quoted);
	printf("ICMP: type %u, code %u, size %d, id 0x%04x, seq 0x%04x\n",
		orig->type, orig->code, ntohs(quoted->ip_len) - IP_HDR_SIZE(quoted),
		ntohs(orig->un.echo.id), ntohs(orig->un.echo.sequence));
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

void	display_reply( const t_packet *pkt, const double *rtt )
{
	char	addr[INET_ADDRSTRLEN];

	inet_ntop(AF_INET, &pkt->from.sin_addr, addr, sizeof(addr));
	printf("%zu bytes from %s: icmp_seq=%u ttl=%u", pkt->icmp_size, addr,
		ntohs(pkt->icmp->un.echo.sequence), pkt->ip->ip_ttl);
	if (rtt)
		printf(" time=%.3f ms", *rtt);
	printf("\n");
}

static int	_loss_percent( const t_stats *s )
{
	if (s->received >= s->transmitted)
		return (0);
	return ((int)((s->transmitted - s->received) * 100 / s->transmitted));
}

void	display_stats( const char *host, const t_stats *s, size_t payload_size )
{
	printf("--- %s ping statistics ---\n", host);
	printf("%zu packets transmitted, %zu packets received, %d%% packet loss\n",
		s->transmitted, s->received, _loss_percent(s));
	if (s->received && payload_size >= sizeof(struct timeval))
	{
		printf("round-trip min/avg/max/stddev = %.3f/%.3f/%.3f/%.3f ms\n",
			s->rtt_min, s->rtt_sum / s->received, s->rtt_max, stats_stddev(s));
	}
}