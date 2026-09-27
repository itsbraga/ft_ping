/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: annabrag <annabrag@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/22 16:20:08 by annabrag          #+#    #+#             */
/*   Updated: 2026/09/27 23:08:51 by annabrag         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_ping.h"

static void	_set_default_options( t_options *o )
{
	o->size = DEFAULT_PAYLOAD_SIZE;
	o->linger = DEFAULT_LINGER;
}

static void	_set_default_stats( t_stats *s )
{
	s->rtt_min = DBL_MAX;
}

void	ping_init( t_ping *p )
{
	memset(p, 0, sizeof(t_ping));
	p->id = getpid() & 0xFFFF;
	_set_default_options(&p->opts);
	_set_default_stats(&p->stats);
}

int	ping_setup( t_ping *p, int argc, char **argv )
{
	p->target = parse_options(&p->opts, argc, argv);
	if (create_socket(p) != EXIT_SUCCESS)
		return (RUNTIME_FAILURE);
	apply_socket_options(p);
	if (resolve_target(p) != EXIT_SUCCESS)
	{
		close(p->sockfd);
		return (RUNTIME_FAILURE);
	}
	return (EXIT_SUCCESS);
}