/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   defines.h                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: annabrag <annabrag@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/25 19:17:10 by annabrag          #+#    #+#             */
/*   Updated: 2026/08/27 17:07:54 by annabrag         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

/**************************\
 * Debug macros
\**************************/

# define ERR_PREFIX	"\033[1m\033[31mError: \033[0m"

/**************************\
 * Exit codes
\**************************/

# define SUCCESS 			0
# define ERR_USAGE			64
# define RUNTIME_FAILURE	1	//	total loss, unknown host

/**************************\
 * Defaults
\**************************/

# define DEFAULT_TTL		64

/**************************\
 * Packet layout
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

# define ICMP_HDR_SIZE		8
# define DEFAULT_PAYLOAD	56
# define MAX_PAYLOAD		65507	//	maximum user data size allowed inside an ICMP Echo Request
									//	packet over IPv4
# define MAX_PACKET_SIZE	65535	//	maximum total length of an IPv4 packet
									//	(65507 + 20 (IP header) + 8 (ICMP header))
									//	(2^16 - 1)

/*
	(uint8_t *)(buf)	reinterprets the ptr as a byte ptr, necessary because buf could 
						be void * or char *

	[0]					the first byte of the diagram

	& 0x0F				binary mask which isolates the 4 least significant bits

	=>	IP_HDR_LEN(buf) gives the offset where ICMP starts
*/
# define IP_HDR_LEN(buf)	((((uint8_t *)(buf))[0] & 0x0F) * 4)