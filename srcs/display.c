/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   display.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: annabrag <annabrag@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/07 15:54:02 by annabrag          #+#    #+#             */
/*   Updated: 2026/09/07 17:33:08 by annabrag         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_ping.h"

static double	_rtt_compute( const struct timeval *sent )
{
	struct timeval	now;
	double			seconds;
	double			micro;

	gettimeofday(&now, NULL);
	seconds = (now.tv_sec - sent->tv_sec) * 1000.0;
	micro = (now.tv_usec - sent->tv_usec) / 1000.0;

	return (seconds + micro);
}

void	display_header( t_ping *ping )
{
	if (!ping->target)
		return ;

	if (!ping->opts.verbose)
		printf("PING %s (%s): %d data bytes\n", 
			ping->target,
			ping->ip_str,
			ping->opts.size);
	else
		printf("PING %s (%s): %d data bytes, id 0x%04x = %u\n",
			ping->target,
			ping->ip_str,
			ping->opts.size,
			ping->id,
			ping->id);
}

void	display_reply( t_ping *ping, const uint8_t* buf, ssize_t received )
{
	(void)ping; // remove when opts are done
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