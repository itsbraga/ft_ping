/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ping.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: annabrag <annabrag@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/11 17:33:03 by annabrag          #+#    #+#             */
/*   Updated: 2026/10/12 00:22:53 by annabrag         ###   ########.fr       */
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
	if (sendto(p->sockfd, buf, icmp_size, 0, (struct sockaddr *)&p->dest,
				sizeof(p->dest)) == -1)
	{
		errno_msg("sending packet");
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

//	Every request got an answer, an Echo Reply or an ICMP error alike
static bool	_all_answered( const t_ping *p )
{
	return (p->opts.count && p->stats.answered >= p->opts.count);
}

/*
	Validates a received datagram and splits it into its headers.
	Every length is checked before the byte it guards is read: the
	buffer comes from the network and nothing in it can be trusted.
	A failed read (size == -1) is rejected by the first test
*/
static bool	_parse_packet( const uint8_t *buf, ssize_t size, t_packet *pkt )
{
	size_t	ip_hdr_size;

	if (size < (ssize_t)sizeof(struct ip))
		return (false);
	ip_hdr_size = IP_HDR_SIZE(buf);
	if (ip_hdr_size < sizeof(struct ip) || (size_t)size < ip_hdr_size + ICMP_HDR_SIZE)
		return (false);
	pkt->ip = (const struct ip *)buf;
	pkt->icmp = (const struct icmphdr *)(buf + ip_hdr_size);
	pkt->icmp_size = (size_t)size - ip_hdr_size;
	return (true);
}

static bool	_is_our_reply( const t_ping *p, const t_packet *pkt )
{
	return (pkt->icmp->type == ICMP_ECHOREPLY && ntohs(pkt->icmp->un.echo.id) == p->id);
}

static bool	_is_handled_error( const struct icmphdr *icmp )
{
	if (icmp->type == ICMP_DEST_UNREACH)
		return (true);
	return (icmp->type == ICMP_TIME_EXCEEDED && icmp->code == ICMP_EXC_TTL);
}

/*
	Checks whether an ICMP error (Dest Unreachable (3) or Time Exceeded (11))
	was triggered by one of our Echo Requests. The error payload carries
	the original IP header followed by the first 8 bytes of the original
	ICMP datagram, which is enough to read type and id
*/
static bool	_is_our_error( const t_ping *p, const t_packet *pkt )
{
	const struct ip			*quoted;
	const struct icmphdr	*orig;
	size_t					ihl;

	if (!_is_handled_error(pkt->icmp))
		return (false);
	if (pkt->icmp_size < ICMP_HDR_SIZE + sizeof(struct ip))
		return (false);
	quoted = (const struct ip *)((const uint8_t *)pkt->icmp + ICMP_HDR_SIZE);
	ihl = IP_HDR_SIZE(quoted);
	//	error header + quoted IP header + quoted Echo Request header
	if (ihl < sizeof(struct ip) || pkt->icmp_size < ICMP_HDR_SIZE + ihl + ICMP_HDR_SIZE)
		return (false);
	if (quoted->ip_p != IPPROTO_ICMP || quoted->ip_dst.s_addr != p->dest.sin_addr.s_addr)
		return (false);
	orig = (const struct icmphdr *)((const uint8_t *)quoted + ihl);
	return (orig->type == ICMP_ECHO && ntohs(orig->un.echo.id) == p->id);
}

/*
	Measures the round-trip time when the reply carries our timestamp,
	records it, and prints the reply line. The timestamp is copied out
	rather than cast in place: its offset gives no alignment guarantee
*/
static void	_report_reply( t_ping *p, const t_packet *pkt )
{
	struct timeval	sent_at;
	double			rtt;
	const double	*shown_rtt;

	shown_rtt = NULL;
	if (pkt->icmp_size >= ICMP_HDR_SIZE + sizeof(sent_at))
	{
		memcpy(&sent_at, (const uint8_t *)pkt->icmp + ICMP_HDR_SIZE, sizeof(sent_at));
		rtt = compute_rtt(&sent_at);
		stats_update(&p->stats, rtt);
		shown_rtt = &rtt;
	}
	display_reply(pkt, shown_rtt);
}

/*
	Consumes exactly one datagram, which poll() reported as ready.
	A raw ICMP socket receives every ICMP datagram the host gets, so
	anything that is neither our Echo Reply nor an error about one of
	out requests is dropped. Both count as an answer for -c
*/
static t_step	_handle_reply( t_ping *p )
{
	uint8_t		buf[MAX_DATAGRAM_SIZE + 1];
	t_packet	pkt;
	ssize_t		size;

	size = recv_packet(p, buf, sizeof(buf), &pkt.from);
	if (!_parse_packet(buf, size, &pkt))
		return (STEP_CONTINUE);
	if (_is_our_reply(p, &pkt))
	{
		p->stats.received++;
		_report_reply(p, &pkt);
	}
	else if (_is_our_error(p, &pkt))
		display_icmp_error(&pkt, p->opts.verbose);
	else
		return (STEP_CONTINUE);
	p->stats.answered++;
	if (_all_answered(p))
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

/*
	Waits for whichever comes first, a datagram or the next scheduled
	action, and handles it. -w is checked after each event, never waited
	for: a run in its linger phase only ends early when a datagram arrives
*/
static t_step	_handle_next_event( t_ping *p, long long *next_action_ms,
					long long deadline_ms )
{
	struct pollfd	pfd;
	int				ready;
	t_step			step;

	pfd = (struct pollfd){.fd = p->sockfd, .events = POLLIN};
	ready = poll(&pfd, 1, _ms_until(*next_action_ms));
	if (ready < 0)
	{
		if (errno == EINTR)
			return (STEP_CONTINUE);
		errno_msg("poll");
		return (STEP_FAILED);
	}
	if (ready > 0)
		step = _handle_reply(p);
	else
		step = _do_next_action(p, next_action_ms);
	if (step == STEP_CONTINUE && p->opts.timeout && _now_ms() >= deadline_ms)
		return (STEP_DONE);
	return (step);
}

static int	_ping_loop( t_ping *p )
{
	long long	next_action_ms;
	long long	deadline_ms;
	t_step		step;

	next_action_ms = _now_ms();
	deadline_ms = 0;
	if (p->opts.timeout)
		deadline_ms = next_action_ms + p->opts.timeout * 1000;
	step = STEP_CONTINUE;
	while (!g_stop && step == STEP_CONTINUE)
		step = _handle_next_event(p, &next_action_ms, deadline_ms);
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
