/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   defines.h                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: annabrag <annabrag@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/15 19:17:10 by annabrag          #+#    #+#             */
/*   Updated: 2026/09/22 17:55:43 by annabrag         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

/**************************\
 *	DEBUG MACROS
\**************************/

# define ERR_PREFIX	"\033[1m\033[31mError:\033[0m"

/**************************\
 *	EXIT CODES
\**************************/

# define EXIT_USAGE			64
# define RUNTIME_FAILURE	1	//	total loss, unknown host

/**************************\
 *	DEFAULTS
\**************************/

# define DEFAULT_TTL			64
# define DEFAULT_INTERVAL_MS	1000	//	milliseconds between two Echo Requests
# define DEFAULT_LINGER			10		//	seconds to wait for late replies once -c is reached

/**************************\
 *	PACKET LAYOUT
\**************************/

/*
		20	IP header (IHL=5, no options)
	+	 8	ICMP header
	+	56	payload		= 16 struct timeval + 40 pattern bytes
	——————
		84	total datagram

	=>	Payload pattern is incremental: byte at index i holds (i & 0xFF),
		starting at 0x00 right after the timeval
*/

# define ICMP_HDR_SIZE			8
# define DEFAULT_PAYLOAD_SIZE	56
# define MAX_PAYLOAD_SIZE		65507	//	maximum user data size allowed inside an ICMP Echo Request
										//	packet over IPv4
# define MAX_DATAGRAM_SIZE		65535	//	maximum total length of an IPv4 datagram
										//	(65507 + 20 (IP header) + 8 (ICMP header))
										//	(2^16 - 1)

/*
	(uint8_t *)(buf)	reinterprets the ptr as a byte ptr, necessary because buf could 
						be void * or char *

	[0]					the first byte of the diagram

	& 0x0F				binary mask which isolates the 4 least significant bits

	=>	IP_HDR_SIZE(buf) gives the offset where the ICMP packet starts
*/
# define IP_HDR_SIZE(buf)	((((uint8_t *)(buf))[0] & 0x0F) * 4)