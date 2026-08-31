/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   socket.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: annabrag <annabrag@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/31 15:13:20 by annabrag          #+#    #+#             */
/*   Updated: 2026/08/31 16:13:58 by annabrag         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_ping.h"

int	socket_init( t_ping *ping )
{
	ping->sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (ping->sockfd == -1)
		return (err_msg("Unable to create raw socket"));
	// sans privileges : -1 ou errno == EPERM
	ping->id = getpid() & 0xFFFF;
	return (EXIT_SUCCESS);
}

int	resolve( t_ping* ping )
{
	memset(&ping->dest, 0, sizeof(ping->dest));
	ping->dest.sin_family = AF_INET;
	if (inet_pton(AF_INET, "127.0.0.1", &ping->dest.sin_addr) != 1)
		return (EXIT_FAILURE);
	return (EXIT_SUCCESS);
}