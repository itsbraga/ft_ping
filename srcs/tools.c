/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   tools.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: annabrag <annabrag@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/31 15:49:58 by annabrag          #+#    #+#             */
/*   Updated: 2026/09/27 22:57:52 by annabrag         ###   ########.fr       */
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
	static const char	*msg[] = {
		"Destination Net Unreachable",
		"Destination Host Unreachable",
		"Destination Protocol Unreachable",
		"Destination Port Unreachable",
		"Frag needed and DF set",
		"Source Route Failed",
		"Destination Net Unknown",
		"Destination Host Unknown",
		"Source Host Isolated",
		"Destination Net Prohibited",
		"Destination Host Prohibited",
		"Destination Net Unreachable for TOS",
		"Destination Host Unreachable for TOS",
		"Communication Administratively Prohibited",
		"Host Precedence Violation",
		"Precedence Cutoff in Effect",
	};

	if (code < sizeof(msg) / sizeof(msg[0]))
		return (msg[code]);
	return ("Dest Unreachable, Unknown Code");
}