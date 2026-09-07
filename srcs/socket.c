/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   socket.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: annabrag <annabrag@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/31 15:13:20 by annabrag          #+#    #+#             */
/*   Updated: 2026/09/07 17:24:54 by annabrag         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_ping.h"

int	socket_init( t_ping *ping )
{
	ping->sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (ping->sockfd == -1)
		return (err_msg(strerror(errno), -1));

	ping->id = getpid() & 0xFFFF;
	return (EXIT_SUCCESS);
}

int	resolve( t_ping* ping )
{
	memset(&ping->dest, 0, sizeof(ping->dest));
	ping->dest.sin_family = AF_INET;
	ping->target = "127.0.0.1";

	if (inet_pton(AF_INET, ping->target, &ping->dest.sin_addr) != 1)
		return (EXIT_FAILURE);

	inet_ntop(AF_INET, &ping->dest.sin_addr, ping->ip_str, sizeof(ping->ip_str));

	return (EXIT_SUCCESS);
}

ssize_t	socket_recv( t_ping *ping, uint8_t *buf, size_t size )
{
	struct sockaddr_in	from;
	socklen_t			from_len;
	ssize_t				received;

	from_len = sizeof(from);

	received = recvfrom(ping->sockfd, buf, size, 0,
						(struct sockaddr *)&from, &from_len);
	if (received == -1)
		return (err_msg("Unable to receive packet", -1));

	return (received);
}