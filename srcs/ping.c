/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ping.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: annabrag <annabrag@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/11 17:33:03 by annabrag          #+#    #+#             */
/*   Updated: 2026/09/22 03:54:04 by annabrag         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/ft_ping.h"

int	ping_init( t_ping *p, int argc, char **argv )
{
	int	ret;

	p->prog_name = argv[0];
	p->opts.size = DEFAULT_PAYLOAD_SIZE;
	p->opts.linger = DEFAULT_LINGER;

	ret = parse_options(p, argc, argv);
	if (ret != EXIT_SUCCESS)
		return (ret);

	if (create_socket(p) != EXIT_SUCCESS)
		return (RUNTIME_FAILURE);
	if (resolve_target(p) != EXIT_SUCCESS)
	{
		close(p->sockfd);
		return (RUNTIME_FAILURE);
	}
	return (EXIT_SUCCESS);
}

/*
	Builds an Echo Request from the current sequence number and hands it
	to the kernel.

	The sequence number is bumped only once the datagram is on the wire,
	so build_echo_request() always stamps the packet it is building, and
	a send that fails does not leave a hole in the sequence.
*/
static int	_send_echo_request( t_ping *p )
{
	uint8_t	buf[ICMP_HDR_SIZE + MAX_PAYLOAD_SIZE];
	size_t	icmp_size;
	ssize_t	bytes_sent;

	icmp_size = build_echo_request(p, buf);
	bytes_sent = sendto(p->sockfd, buf, icmp_size, 0,
					(struct sockaddr *)&p->dest, sizeof(p->dest));
	if (bytes_sent == -1)
		return (err_msg("Unable to send packet", EXIT_FAILURE));

	p->stats.transmitted++;
	p->seq++;
	return (EXIT_SUCCESS);
}

/*
	Consumes exactly one datagram, which poll() reported as ready.

	A raw ICMP socket receives every ICMP datagram the host gets, not just
	ours, so everything that is not an Echo Reply carrying our id is dropped:

		- too short to hold an IP + ICMP header	-> truncated/garbage
		- type != ICMP_ECHOREPLY				-> someone else's traffic
		- id != p->id						-> another ping process
*/
static void	_handle_reply( t_ping *p )
{
	uint8_t			buf[MAX_DATAGRAM_SIZE + 1];
	ssize_t			datagram_size;
	struct icmphdr	*icmp_hdr;
	unsigned int	ip_hdr_size;

	datagram_size = recv_packet(p, buf, sizeof(buf));
	if (datagram_size == -1)
		return ;

	ip_hdr_size = IP_HDR_SIZE(buf);
	if (datagram_size < (ssize_t)(ip_hdr_size + ICMP_HDR_SIZE))
		return ;

	icmp_hdr = (struct icmphdr *)(buf + ip_hdr_size);
	if (icmp_hdr->type != ICMP_ECHOREPLY)
		return ;
	if (ntohs(icmp_hdr->un.echo.id) != p->id)
		return ;

	p->stats.received++;
	display_reply(p, buf, datagram_size);
}

static void	_set_deadline( struct timespec *deadline, time_t seconds )
{
	clock_gettime(CLOCK_MONOTONIC, deadline);
	deadline->tv_sec += seconds;
}

/*
	Rounded up to the next millisecond: truncating would wake poll() just
	before the deadline, and the loop would spin with a 0 ms timeout
*/
static int	_ms_until( const struct timespec *deadline )
{
	struct timespec	now;
	long long		remaining_ns;

	clock_gettime(CLOCK_MONOTONIC, &now);

	remaining_ns = (deadline->tv_sec - now.tv_sec) * 1000000000LL
		+ (deadline->tv_nsec - now.tv_nsec);
	if (remaining_ns <= 0)
		return (0);

	return ((int)((remaining_ns + 999999) / 1000000));
}

//	opts.count == 0 means unlimited; -c counts requests sent, not replies
static bool	_count_reached( const t_ping *p, size_t transmitted )
{
	return (p->opts.count && transmitted >= p->opts.count);
}

/*
	Runs each time the poll() deadline expires. Same state machine as the
	reference: send every interval until -c is reached, then wait one more
	period of `linger` seconds for late replies, then stop. Without that
	drain phase, the reply to the last request would be lost.

	Returns false when the loop must end.
*/
static bool	_on_deadline( t_ping *p, struct timespec *deadline, bool *draining )
{
	if (!_count_reached(p, p->stats.transmitted))
	{
		if (_send_echo_request(p) != EXIT_SUCCESS)
			return (false);
		_set_deadline(deadline, DEFAULT_INTERVAL);
		return (true);
	}

	if (*draining)
		return (false);
	*draining = true;

	_set_deadline(deadline, p->opts.linger);
	return (true);
}

void	ping_loop( t_ping *p )
{
	struct pollfd	pfd;
	struct timespec	deadline;
	bool			draining;
	int				ready;

	pfd.fd = p->sockfd;
	pfd.events = POLLIN;
	draining = false;

	_set_deadline(&deadline, 0);

	while (!g_stop)
	{
		ready = poll(&pfd, 1, _ms_until(&deadline));
		if (ready == -1)
		{
			if (errno != EINTR)
				return ((void)err_msg("poll failed", EXIT_FAILURE));
			continue ;
		}
		if (ready == 0)
		{
			if (!_on_deadline(p, &deadline, &draining))
				return ;
			continue ;
		}
		_handle_reply(p);
	}
}
