/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   socket.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: annabrag <annabrag@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/31 15:13:20 by annabrag          #+#    #+#             */
/*   Updated: 2026/09/22 17:57:08 by annabrag         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/ft_ping.h"

int	create_socket( t_ping *p )
{
	p->sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (p->sockfd < 0)
	{
		if (errno == EPERM || errno == EACCES)
		{
			errno = 0;

			/*
				At least Linux can allow subprivileged users to send ICMP
				packets formally encapsulated and built as a datagram socket,
				but then the identity number is set by the kernel itself
			*/
			p->sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_ICMP);
			if (p->sockfd < 0)
			{
				if (errno == EPERM || errno == EACCES)
					err_msg(p->prog_name, "Lacking privilege for icmp socket");
				else
				 	err_msg(p->prog_name, strerror(errno));
				return (RUNTIME_FAILURE);
			}
		}
		else
		{
			errno_msg(p->prog_name, "socket");
			return (RUNTIME_FAILURE);
		}
	}
	if (setuid(getuid()) != 0)
	{
		errno_msg(p->prog_name, "setuid");
		close(p->sockfd);
		return (RUNTIME_FAILURE);
	}
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
		err_msg(p->prog_name, "unknown host");
		return (RUNTIME_FAILURE);
	}
	memcpy(&p->dest, res->ai_addr, sizeof(p->dest));
	snprintf(p->hostname, sizeof(p->hostname), "%s",
		res->ai_canonname ? res->ai_canonname : p->target);

	freeaddrinfo(res);
	inet_ntop(AF_INET, &p->dest.sin_addr, p->target_ip_str, sizeof(p->target_ip_str));
	return (EXIT_SUCCESS);
}

ssize_t	recv_packet( t_ping *p, uint8_t *buf, size_t buf_size, struct sockaddr_in *from )
{
	socklen_t	from_len;
	ssize_t		size;

	from_len = sizeof(*from);
	size = recvfrom(p->sockfd, buf, buf_size, MSG_DONTWAIT,
						(struct sockaddr *)from, &from_len);
	if (size == -1 && errno != EINTR && errno != EAGAIN && errno != EWOULDBLOCK)
		errno_msg(p->prog_name, "recvfrom");
	return (size);
}