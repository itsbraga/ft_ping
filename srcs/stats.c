/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   stats.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: annabrag <annabrag@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/22 02:06:53 by annabrag          #+#    #+#             */
/*   Updated: 2026/09/22 17:46:09 by annabrag         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_ping.h"

double	compute_rtt( const struct timeval *sent_at )
{
	struct timeval	now;

	gettimeofday(&now, NULL);
	return ((now.tv_sec - sent_at->tv_sec) * 1000.0
		+ (now.tv_usec - sent_at->tv_usec) / 1000.0);
}

//	Called once per timed reply, right after received is incremented.
void	stats_update( t_stats *s, double rtt )
{
	s->rtt_sum += rtt;
	s->rtt_sum_sq += rtt * rtt;
	if (rtt < s->rtt_min)
		s->rtt_min = rtt;
	if (rtt > s->rtt_max)
		s->rtt_max = rtt;
}

double	stats_stddev( const t_stats *s )
{
	double	avg;
	double	variance;

	avg = s->rtt_sum / s->received;
	variance = s->rtt_sum_sq / s->received - avg * avg;
	if (variance < 0.0005)
		return (0.0);
	return (sqrt(variance));
}