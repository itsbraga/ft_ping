/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   packet.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: annabrag <annabrag@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/26 19:01:23 by annabrag          #+#    #+#             */
/*   Updated: 2026/08/31 16:17:18 by annabrag         ###   ########.fr       */
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
uint16_t	packet_checksum( const void *data, size_t len )
{
	const uint16_t	*words;
	uint32_t		sum;
	uint16_t		last;

	words = (const uint16_t *)data;
	sum = 0;

	while (len > 1)
	{
		sum += *words++;
		len -= 2;	// 2 bytes per round
	}
	if (len == 1)
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

size_t	packet_build( t_ping *ping, uint8_t *buf )
{
	struct icmphdr	*hdr;
	uint8_t			*payload;
	size_t			total;
	size_t			i;

	total = ICMP_HDR_SIZE + ping->opts.size;
	memset(buf, 0, total);
	hdr = (struct icmphdr *)buf;
	payload = buf + ICMP_HDR_SIZE;

	hdr->type = ICMP_ECHO;	// Echo Request (8)
	hdr->un.echo.id = htons(ping->id);
	hdr->un.echo.sequence = htons(ping->seq);

	gettimeofday((struct timeval *)payload, NULL);

	i = sizeof(struct timeval);
	while (i < (size_t)ping->opts.size)
	{
		payload[i] = (i - sizeof(struct timeval)) & 0xFF;
		i++;
	}

	hdr->checksum = packet_checksum(buf, total);
	return (total);
}