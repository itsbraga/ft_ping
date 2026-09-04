/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: annabrag <annabrag@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/15 18:13:06 by annabrag          #+#    #+#             */
/*   Updated: 2026/08/31 16:22:34 by annabrag         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_ping.h"

// static void	_set_program_name( t_ping *ping, char *name )
// {
// 	ping->prog_name = name;
// }

// int	main( int argc, char **argv )
// {
// 	if (argc != 2)
// 		return (err_args("Invalid amount of arguments.", EXIT_FAILURE) );

// 	t_ping	ping;

// 	_set_program_name(&ping, argv[0]);
// 	return (EXIT_SUCCESS);
// }

int	main( void )
{
	t_ping			ping = {0};
	size_t			packet_size;
	uint8_t			buf[ICMP_HDR_SIZE + MAX_PAYLOAD];
	ssize_t			sent, received;
	uint8_t			recv_buf[MAX_PACKET_SIZE + 1];
	struct icmphdr	*icmp;
	unsigned int	hdr_len;

	ping.opts.size = DEFAULT_PAYLOAD;

	if (socket_init(&ping) != EXIT_SUCCESS || resolve(&ping) != EXIT_SUCCESS)
		return (EXIT_FAILURE);

	packet_size = packet_build(&ping, buf);

	sent = sendto(ping.sockfd, buf, packet_size, 0,
					(struct sockaddr *)&ping.dest, sizeof(ping.dest));
	if (sent == -1)
		return (err_msg("Unable to send packet", -1));

	while (true)
	{
		received = socket_recv(&ping, recv_buf, sizeof(recv_buf));
		if (received == -1)
			return (EXIT_FAILURE);

		hdr_len = IP_HDR_LEN(recv_buf);
		if (received < (ssize_t)(hdr_len + ICMP_HDR_SIZE))
			continue ;

		icmp = (struct icmphdr *)(recv_buf + hdr_len);
		if (icmp->type != ICMP_ECHOREPLY)
			continue ;
		if (ntohs(icmp->un.echo.id) != ping.id)
			continue ;

		break ;
	}

	printf("received %zd bytes, IP header = %u bytes\n", received, IP_HDR_LEN(recv_buf));
	debug_hexdump(recv_buf, received);

	close(ping.sockfd);
	return (EXIT_SUCCESS);
}