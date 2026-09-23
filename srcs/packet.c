/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   packet.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: annabrag <annabrag@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/26 19:01:23 by annabrag          #+#    #+#             */
/*   Updated: 2026/09/22 17:56:57 by annabrag         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_ping.h"

/*
	Internet checksum — RFC 1071.

	Sums the data as 16-bit words into a 32-bit accumulator, folds the
	carries back into the low 16 bits, and returns the one's complement.

	The one's complement is what makes verification cheap: since
	x + ~x == 0xFFFF, the receiver only has to sum the whole message
	(checksum field included) and check the result against 0xFFFF.

	The checksum field MUST be zeroed before calling this: it is part of
	the summed range, and zero is the neutral element of the addition.
*/
static uint16_t	_internet_checksum( const void *data, size_t size )
{
	const uint16_t	*words;
	uint32_t		sum;
	uint16_t		last;

	words = (const uint16_t *)data;
	sum = 0;
	while (size > 1)
	{
		sum += *words++;
		size -= 2;	// 2 bytes per round
	}
	if (size == 1)
	{
		last = 0;
		memcpy(&last, words, 1);
		sum += last;
	}
	/*
		sum >> 16		the accumulated carries (high 16 bits)
		sum & 0xFFFF	the partial result (low 16 bits)
	
		Adding them back together is the end-around carry required by
		one's complement arithmetic. while, not if: the fold can itself
		carry (0x0001FFFF -> 0x00010000 -> 0x00000001).
	*/
	while (sum >> 16)
		sum = (sum >> 16) + (sum & 0xFFFF);
	return ((uint16_t)~sum);
}

static void	_fill_payload( uint8_t *payload, size_t size )
{
	struct timeval	now;
	size_t			i;

	if (size >= sizeof(now))
	{
		gettimeofday(&now, NULL);
		memcpy(payload, &now, sizeof(now));
		i = sizeof(now);
		while (i < size)
		{
			payload[i] = (i - sizeof(now)) & 0xFF;
			i++;
		}
	}
	else
	{
		i = 0;
		while (i < size)
		{
			payload[i] = i & 0xFF;
			i++;
		}
	}
}

size_t	build_echo_request( t_ping *p, uint8_t *buf )
{
	size_t			icmp_size;
	struct icmphdr	*icmp_hdr;

	icmp_size = ICMP_HDR_SIZE + p->opts.size;
	memset(buf, 0, icmp_size);
	icmp_hdr = (struct icmphdr *)buf;
	icmp_hdr->type = ICMP_ECHO;
	icmp_hdr->un.echo.id = htons(p->id);
	icmp_hdr->un.echo.sequence = htons(p->seq);
	_fill_payload(buf + ICMP_HDR_SIZE, p->opts.size);
	icmp_hdr->checksum = _internet_checksum(buf, icmp_size);
	return (icmp_size);
}