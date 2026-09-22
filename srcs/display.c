/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   display.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: annabrag <annabrag@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/07 15:54:02 by annabrag          #+#    #+#             */
/*   Updated: 2026/09/22 04:07:39 by annabrag         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/ft_ping.h"

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

void	display_unknown_host( const char *prog_name )
{
	fprintf(stderr, "%s: unknown host\n", prog_name);
}

void	display_target_info( t_ping *p )
{
	printf("PING %s (%s): %zu data bytes", p->hostname, p->target_ip_str, p->opts.size);

	if (p->opts.verbose)
		printf(", id 0x%04x = %u", p->id, p->id);

	printf("\n");
}

void	display_reply( t_ping *p, const uint8_t* buf, ssize_t datagram_size )
{
	const struct ip			*ip_hdr;
	const struct icmphdr	*icmp_hdr;
	const struct timeval	*sent_at;
	unsigned int			ip_hdr_size;
	uint16_t				curr_seq;
	
	ip_hdr = (const struct ip *)buf;
	ip_hdr_size = IP_HDR_SIZE(buf);
	icmp_hdr = (const struct icmphdr *)(buf + ip_hdr_size);
	sent_at = (const struct timeval *)(buf + ip_hdr_size + ICMP_HDR_SIZE);
	curr_seq = ntohs(icmp_hdr->un.echo.sequence);
	
	printf("%zd bytes from %s: icmp_seq=%u ttl=%u time=%.3f ms\n",
		(ssize_t)(datagram_size - ip_hdr_size), p->target_ip_str, curr_seq, ip_hdr->ip_ttl, compute_rtt(sent_at));
}
	
void	display_stats( const char *host, const t_stats *s, size_t payload_size )
{
	double	avg;
	double	variance;

	printf("--- %s ping statistics ---\n", host);
	printf("%zu packets transmitted, ", s->transmitted);
	printf("%zu packets received, ", s->received);
	if (s->transmitted != 0)
		printf("%d%% packet loss", (int)(((s->transmitted - s->received) * 100) / s->transmitted));
	printf("\n");

	if (s->received && payload_size >= sizeof(struct timeval))
	{
		avg = s->rtt_sum / s->received;
		variance = s->rtt_sum_sq / s->received - avg * avg;
		// printf("round-trip min/avg/max/stddev = %.3f/%.3f/%.3f/%.3f ms\n",
		// 	s->rtt_min, avg, s->rtt_max, )
	}
}