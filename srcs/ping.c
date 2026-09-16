/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ping.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: annabrag <annabrag@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/11 17:33:03 by annabrag          #+#    #+#             */
/*   Updated: 2026/09/16 20:00:15 by annabrag         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_ping.h"

static int	_parse_count( t_ping *p, const char *arg )
{
	char	*endptr;

	p->opts.count = strtoul(arg, &endptr, 10);
	if (*endptr != '\0')
	{
		display_invalid_value(p->prog_name, arg, endptr);
		return (RUNTIME_FAILURE);
	}
	return (EXIT_SUCCESS);
}

static bool	_is_help_request( int opt )
{
	return (opt == OPT_HELP || (opt == '?' && optopt == '?'));
}

static int	_reject_option( t_ping *p, int opt, char **argv )
{
	if (opt == ':')
		display_missing_argument(p->prog_name, optopt);
	else if (optopt)
		display_invalid_option(p->prog_name, optopt);
	else
		display_unrecognized_option(p->prog_name, argv[optind - 1]);
	return (ERR_USAGE);
}

static int	_parse_options( t_ping *p, int argc, char **argv )
{
	static const struct option	long_opts[] = {
		{"help", no_argument, NULL, OPT_HELP},
		{0, 0, 0, 0}
	};
	int							opt;

	opterr = 0;
	while ((opt = getopt_long(argc, argv, ":vc:", long_opts, NULL)) != -1)
	{
		//	like argp, help wins immediately and nothing has been allocated yet
		if (_is_help_request(opt))
		{
			display_help(p->prog_name);
			exit(EXIT_SUCCESS);
		}
		else if (opt == 'v')
			p->opts.verbose = 1;
		else if (opt == 'c')
		{
			if (_parse_count(p, optarg) != EXIT_SUCCESS)
				return (RUNTIME_FAILURE);
		}
		else
			return (_reject_option(p, opt, argv));
	}
	if (optind >= argc)
	{
		display_missing_host(p->prog_name);
		return (ERR_USAGE);
	}
	p->target = argv[optind];
	return (EXIT_SUCCESS);
}

int	init_ping( t_ping *p, int argc, char **argv )
{
	int	ret;

	p->prog_name = argv[0];
	p->opts.size = DEFAULT_PAYLOAD;
	p->opts.linger = DEFAULT_LINGER;

	ret = _parse_options(p, argc, argv);
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
	uint8_t	buf[ICMP_HDR_SIZE + MAX_PAYLOAD];
	size_t	packet_size;
	ssize_t	sent;

	packet_size = build_echo_request(p, buf);
	sent = sendto(p->sockfd, buf, packet_size, 0,
					(struct sockaddr *)&p->dest, sizeof(p->dest));
	if (sent == -1)
		return (err_msg("Unable to send packet", EXIT_FAILURE));

	p->stats.sent++;
	p->seq++;
	return (EXIT_SUCCESS);
}

/*
	Consumes exactly one datagram, which poll() reported as ready.

	A raw ICMP socket receives every ICMP datagram the host gets, not just
	ours, so everything that is not an Echo Reply carrying our id is dropped:

		- too short to hold an IP + ICMP header	-> truncated/garbage
		- type != ICMP_ECHOREPLY				-> someone else's traffic
		- id != ping->id						-> another ping process
*/
static void	_handle_reply( t_ping *p )
{
	uint8_t			buf[MAX_PACKET_SIZE + 1];
	ssize_t			received;
	struct icmphdr	*icmp;
	unsigned int	hdr_len;

	received = recv_packet(p, buf, sizeof(buf));
	if (received == -1)
		return ;

	hdr_len = IP_HDR_LEN(buf);
	if (received < (ssize_t)(hdr_len + ICMP_HDR_SIZE))
		return ;

	icmp = (struct icmphdr *)(buf + hdr_len);
	if (icmp->type != ICMP_ECHOREPLY)
		return ;
	if (ntohs(icmp->un.echo.id) != p->id)
		return ;

	p->stats.received++;
	display_reply(p, buf, received);
}

static void	_set_deadline( struct timespec *deadline, time_t seconds )
{
	clock_gettime(CLOCK_MONOTONIC, deadline);
	deadline->tv_sec += seconds;
}

/*
	Rounded up to the next millisecond: truncating would wake poll() just
	before the deadline, and the loop would spin with a 0 ms timeout.
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
static bool	_count_reached( const t_ping *p, int done )
{
	return (p->opts.count && (unsigned int)done >= p->opts.count);
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
	if (!_count_reached(p, p->stats.sent))
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

/*
	The poll() timeout carries all the timing, so the interval is measured
	from one send to the next, whatever the RTT, and a silent target never
	blocks the program.
*/
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
