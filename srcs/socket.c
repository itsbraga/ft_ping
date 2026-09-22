/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   socket.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: annabrag <annabrag@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/31 15:13:20 by annabrag          #+#    #+#             */
/*   Updated: 2026/09/22 03:39:56 by annabrag         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/ft_ping.h"

int	create_socket( t_ping *p )
{
	p->sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (p->sockfd == -1)
		return (err_msg(strerror(errno), -1));
	
	p->id = getpid() & 0xFFFF;
	setuid(getuid());
	return (EXIT_SUCCESS);
}

/*
	AI_CANONNAME: the reference prints the canonical name, not the argument,
	on its PING line (www.github.com -> github.com)
*/
int	resolve_target( t_ping *p )
{
	struct addrinfo	hints;
	struct addrinfo	*res;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_flags = AI_CANONNAME;

	if (getaddrinfo(p->target, NULL, &hints, &res) != 0)
	{
		display_unknown_host(p->prog_name);
		return (RUNTIME_FAILURE);
	}

	memcpy(&p->dest, res->ai_addr, sizeof(p->dest));
	snprintf(p->hostname, sizeof(p->hostname), "%s",
		res->ai_canonname ? res->ai_canonname : p->target);

	freeaddrinfo(res);

	inet_ntop(AF_INET, &p->dest.sin_addr, p->target_ip_str, sizeof(p->target_ip_str));
	return (EXIT_SUCCESS);
}

ssize_t	recv_packet( t_ping *p, uint8_t *buf, size_t buf_size )
{
	struct sockaddr_in	from;
	socklen_t			from_len;
	ssize_t				datagram_size;

	from_len = sizeof(from);
	//	MSG_DONTWAIT: poll() already said data is there, but a readiness
	//	report is only a hint, the loop must never block here
	datagram_size = recvfrom(p->sockfd, buf, buf_size, MSG_DONTWAIT,
						(struct sockaddr *)&from, &from_len);
	if (datagram_size == -1)
	{
		if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)
			return (-1);
		return (err_msg("Unable to receive packet", -1));
	}
	return (datagram_size);
}