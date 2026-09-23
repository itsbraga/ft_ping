/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ping.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: panther <panther@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/11 17:33:03 by annabrag          #+#    #+#             */
/*   Updated: 2026/09/22 23:44:07 by panther          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_ping.h"

/*
	Builds an Echo Request from the current sequence number and hands it
	to the kernel.

	The sequence number is bumped only once the datagram is on the wire,
	so a send that fails leaves no hole in the sequence.
*/
static int	_send_echo_request( t_ping *p )
{
	uint8_t	buf[ICMP_HDR_SIZE + MAX_PAYLOAD_SIZE];
	size_t	icmp_size;

	icmp_size = build_echo_request(p, buf);
	if (sendto(p->sockfd, buf, icmp_size, 0,
					(struct sockaddr *)&p->dest, sizeof(p->dest)) == -1)
	{
		errno_msg(p->prog_name, "sending packet");
		return (RUNTIME_FAILURE);
	}
	p->stats.transmitted++;
	p->seq++;
	return (EXIT_SUCCESS);
}

//	count == 0 means unlimited: neither limit is ever reached
static bool	_all_sent( const t_ping *p )
{
	return (p->opts.count && p->stats.transmitted >= p->opts.count);
}

static bool	_all_received( const t_ping *p )
{
	return (p->opts.count && p->stats.received >= p->opts.count);
}

/*
	Returns the ICMP header of a well-formed datagram, NULL otherwise.
	Every length is checked before the byte it guards is read: the
	buffer comes from the network and nothing in it can be trusted
*/
static const struct icmphdr	*_icmp_header( const uint8_t *buf, ssize_t size )
{
	size_t	ip_hdr_size;

	if (size < (ssize_t)sizeof(struct ip))
		return (NULL);
	ip_hdr_size = IP_HDR_SIZE(buf);
	if (ip_hdr_size < sizeof(struct ip) || (size_t)size < ip_hdr_size + ICMP_HDR_SIZE)
		return (NULL);
	return ((const struct icmphdr *)(buf + ip_hdr_size));
}

/*
	Measures the round-trip time when the reply carries our timestamp,
	records it, and prints the reply line. The timestamp is copied out
	rather than cast in place: its offset gives no alignment guarantee
*/
static void	_report_reply( t_ping *p, const uint8_t *buf, size_t icmp_size,
				const struct sockaddr_in *from )
{
	const struct icmphdr	*icmp_hdr;
	struct timeval			sent_at;
	double					rtt;
	const double			*shown_rtt;
	uint16_t				curr_seq;

	icmp_hdr = (const struct icmphdr *)(buf + IP_HDR_SIZE(buf));
	shown_rtt = NULL;
	if (icmp_size >= ICMP_HDR_SIZE + sizeof(sent_at))
	{
		memcpy(&sent_at, (const uint8_t *)icmp_hdr + ICMP_HDR_SIZE, sizeof(sent_at));
		rtt = compute_rtt(&sent_at);
		stats_update(&p->stats, rtt);
		shown_rtt = &rtt;
	}
	curr_seq = ntohs(icmp_hdr->un.echo.sequence);
	display_reply(from, icmp_size, curr_seq, ((const struct ip *)buf)->ip_ttl, shown_rtt);
}

/*
	Consumes exactly one datagram, which poll() reported as ready.
	A raw ICMP socket receives every ICMP datagram the host gets, so
	anything that is not a well-formed Echo Reply carrying our id is
	dropped. The run ends as soon as every expected reply is in
*/
static t_step	_handle_reply( t_ping *p )
{
	uint8_t					buf[MAX_DATAGRAM_SIZE + 1];
	struct sockaddr_in		from;
	ssize_t					size;
	const struct icmphdr	*icmp_hdr;

	size = recv_packet(p, buf, sizeof(buf), &from);
	if (size < 0)
		return (STEP_CONTINUE);
	icmp_hdr = _icmp_header(buf, size);
	if (!icmp_hdr || icmp_hdr->type != ICMP_ECHOREPLY || ntohs(icmp_hdr->un.echo.id) != p->id)
		return (STEP_CONTINUE);
	p->stats.received++;
	_report_reply(p, buf, (size_t)size - IP_HDR_SIZE(buf), &from);
	if (_all_received(p))
		return (STEP_DONE);
	return (STEP_CONTINUE);
}

static long long	_now_ms( void )
{
	struct timespec	now;

	clock_gettime(CLOCK_MONOTONIC, &now);
	return (now.tv_sec * 1000LL + now.tv_nsec / 1000000);
}

/*
	Time left before target_ms, in the int that poll() expects.
	Past targets give 0; a very long -W is capped instead of overflowing
*/
static int	_ms_until( long long target_ms )
{
	long long	left;

	left = target_ms - _now_ms();
	if (left < 0)
		return (0);
	if (left > INT_MAX)
		return (INT_MAX);
	return ((int)left);
}

/*
	Called when the next scheduled action is due. Sends a request and
	schedules the next one a second later. After the last request, the
	next action is the end of the run, pushed back by `linger` seconds
	so that late replies can still arrive
*/
static t_step	_do_next_action( t_ping *p, long long *next_action_ms )
{
	if (_all_sent(p))
		return (STEP_DONE);
	if (_send_echo_request(p) != EXIT_SUCCESS)
		return (STEP_FAILED);
	*next_action_ms = _now_ms() + DEFAULT_INTERVAL_MS;
	if (_all_sent(p))
		*next_action_ms += p->opts.linger * 1000;
	return (STEP_CONTINUE);
}

static t_step	_handle_next_event( t_ping *p, long long *next_action_ms )
{
	struct pollfd	pfd;
	int				ready;

	pfd = (struct pollfd){.fd = p->sockfd, .events = POLLIN};
	ready = poll(&pfd, 1, _ms_until(*next_action_ms));
	if (ready > 0)
		return (_handle_reply(p));
	if (ready == 0)
		return (_do_next_action(p, next_action_ms));
	if (errno == EINTR)
		return (STEP_CONTINUE);
	errno_msg(p->prog_name, "poll");
	return (STEP_FAILED);
}

static int	_ping_loop( t_ping *p )
{
	long long	next_action_ms;
	t_step		step;

	next_action_ms = _now_ms();
	step = STEP_CONTINUE;
	while (!g_stop && step == STEP_CONTINUE)
		step = _handle_next_event(p, &next_action_ms);
	if (step == STEP_FAILED)
		return (RUNTIME_FAILURE);
	return (EXIT_SUCCESS);
}

int	ping_run( t_ping *p )
{
	display_target_info(p);
	//	A send failure ends the run without statistics (cf. refs/send_denied.cat)
	if (_ping_loop(p) != EXIT_SUCCESS)
		return (RUNTIME_FAILURE);
	display_stats(p->hostname, &p->stats, p->opts.size);
	if (p->stats.received == 0)
		return (RUNTIME_FAILURE);
	return (EXIT_SUCCESS);
}
