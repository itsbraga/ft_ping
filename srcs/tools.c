/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   tools.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: annabrag <annabrag@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/31 15:49:58 by annabrag          #+#    #+#             */
/*   Updated: 2026/10/12 00:21:02 by annabrag         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_ping.h"

void	errno_msg( const char *context )
{
	fprintf(stderr, "%s: %s: %s\n", PROG_NAME, context, strerror(errno));
}

void	err_msg( const char *detail )
{
	fprintf(stderr, "%s: %s\n", PROG_NAME, detail);
}

const char	*unreach_str( uint8_t code )
{
	switch (code)
	{
		case ICMP_NET_UNREACH:
			return ("Destination Net Unreachable");
		case ICMP_HOST_UNREACH:
			return ("Destination Host Unreachable");
		case ICMP_PROT_UNREACH:
			return ("Destination Protocol Unreachable");
		case ICMP_PORT_UNREACH:
			return ("Destination Port Unreachable");
		case ICMP_PKT_FILTERED:
			return ("Packet Filtered");
		default:
			return (NULL);
	}
}