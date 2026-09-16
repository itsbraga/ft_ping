/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   display.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: annabrag <annabrag@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/07 15:54:02 by annabrag          #+#    #+#             */
/*   Updated: 2026/09/16 19:43:43 by annabrag         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_ping.h"

static double	_rtt_compute( const struct timeval *sent )
{
	struct timeval	now;

	gettimeofday(&now, NULL);
	return ((now.tv_sec - sent->tv_sec) * 1000.0
		+ (now.tv_usec - sent->tv_usec) / 1000.0);
}

static const char	*_short_name( const char *prog_name )
{
	const char	*slash;

	slash = strrchr(prog_name, '/');
	return (slash ? slash + 1 : prog_name);
}

static void	_display_try_help( const char *prog_name )
{
	fprintf(stderr, "Try '%s --help' or '%s --usage' for more information.\n",
		_short_name(prog_name), _short_name(prog_name));
}

void	display_help( const char *prog_name )
{
	printf("Usage: %s [OPTION...] HOST ...\n", _short_name(prog_name));
	fputs(
		"Send ICMP ECHO_REQUEST packets to network hosts.\n"
		"\n"
		" Options controlling ICMP request types:\n"
		"      --address              send ICMP_ADDRESS packets (root only)\n"
		"      --echo                 send ICMP_ECHO packets (default)\n"
		"      --mask                 same as --address\n"
		"      --timestamp            send ICMP_TIMESTAMP packets\n"
		"  -t, --type=TYPE            send TYPE packets\n"
		"\n"
		" Options valid for all request types:\n"
		"\n"
		"  -c, --count=NUMBER         stop after sending NUMBER packets\n"
		"  -d, --debug                set the SO_DEBUG option\n"
		"  -i, --interval=NUMBER      wait NUMBER seconds between sending each packet\n"
		"  -n, --numeric              do not resolve host addresses\n"
		"  -r, --ignore-routing       send directly to a host on an attached network\n"
		"      --ttl=N                specify N as time-to-live\n"
		"  -T, --tos=NUM              set type of service (TOS) to NUM\n"
		"  -v, --verbose              verbose output\n"
		"  -w, --timeout=N            stop after N seconds\n"
		"  -W, --linger=N             number of seconds to wait for response\n"
		"\n"
		" Options valid for --echo requests:\n"
		"\n"
		"  -f, --flood                flood ping (root only)\n"
		"      --ip-timestamp=FLAG    IP timestamp of type FLAG, which is one of\n"
		"                             \"tsonly\" and \"tsaddr\"\n"
		"  -l, --preload=NUMBER       send NUMBER packets as fast as possible before\n"
		"                             falling into normal mode of behavior (root only)\n"
		"  -p, --pattern=PATTERN      fill ICMP packet with given pattern (hex)\n"
		"  -q, --quiet                quiet output\n"
		"  -R, --route                record route\n"
		"  -s, --size=NUMBER          send NUMBER data octets\n"
		"\n"
		"  -?, --help                 give this help list\n"
		"      --usage                give a short usage message\n"
		"  -V, --version              print program version\n"
		"\n"
		"Mandatory or optional arguments to long options are also mandatory or optional\n"
		"for any corresponding short options.\n"
		"\n"
		"Options marked with (root only) are available only to superuser.\n"
		"\n"
		"Report bugs to <bug-inetutils@gnu.org>.\n",
		stdout);
}

void	display_missing_host( const char *prog_name )
{
	fprintf(stderr, "%s: missing host operand\n", _short_name(prog_name));
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
	fprintf(stderr, "%s: invalid value (`%s' near `%s')\n",
		prog_name, value, near);
}

void	display_unknown_host( const char *prog_name )
{
	fprintf(stderr, "%s: unknown host\n", prog_name);
}

void	display_target_info( t_ping *p )
{
	printf("PING %s (%s): %d data bytes",
		p->hostname,
		p->ip_str,
		p->opts.size);
	if (p->opts.verbose)
		printf(", id 0x%04x = %u", p->id, p->id);
	printf("\n");
}

void	display_reply( t_ping *p, const uint8_t* buf, ssize_t received )
{
	(void)p; // remove when opts are done
	const struct ip			*ip_hdr;
	const struct icmphdr	*icmp;
	const struct timeval	*sent;
	unsigned int			hdr_len;
	char					src[INET_ADDRSTRLEN];

	hdr_len = IP_HDR_LEN(buf);
	ip_hdr = (const struct ip *)buf;
	icmp = (const struct icmphdr *)(buf + hdr_len);
	sent = (const struct timeval *)(buf + hdr_len + ICMP_HDR_SIZE);

	inet_ntop(AF_INET, &ip_hdr->ip_src, src, sizeof(src));
	printf("%zd bytes from %s: icmp_seq=%u ttl=%u time=%.3f ms\n",
		(ssize_t)(received - hdr_len),
		src,
		ntohs(icmp->un.echo.sequence),
		ip_hdr->ip_ttl,
		_rtt_compute(sent));
}