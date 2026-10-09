/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   packet.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: annabrag <annabrag@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/26 19:01:23 by annabrag          #+#    #+#             */
/*   Updated: 2026/10/09 19:41:19 by annabrag         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_ping.h"

/*
	RFC 1071

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
	const uint8_t	*bytes;
	uint32_t		sum;
	uint16_t		word;

	bytes = (const uint8_t *)data;
	sum = 0;
	while (size > 1)
	{
		memcpy(&word, bytes, sizeof(word));
		sum += word;
		bytes += 2;
		size -= 2;
	}
	if (size == 1)
	{
		word = 0;
		memcpy(&word, bytes, 1);
		sum += word;
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
	size_t			offset;
	size_t			i;

	offset = 0;
	if (size >= sizeof(now))
	{
		gettimeofday(&now, NULL);
		memcpy(payload, &now, sizeof(now));
		offset = sizeof(now);
	}
	i = offset;
	while (i < size)
	{
		payload[i] = (i - offset) & 0xFF;
		i++;
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