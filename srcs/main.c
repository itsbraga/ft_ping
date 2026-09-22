/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: annabrag <annabrag@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/15 18:13:06 by annabrag          #+#    #+#             */
/*   Updated: 2026/09/22 03:34:32 by annabrag         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/ft_ping.h"

volatile sig_atomic_t	g_stop = 0;

static void	_handle_sigint( int sig __attribute__((unused)) )
{
	g_stop = 1;
}

int	main( int argc, char **argv )
{
	t_ping	p = {0};
	int		exit_code;

	exit_code = ping_init(&p, argc, argv);
	if (exit_code != EXIT_SUCCESS)
		return (exit_code);

	signal(SIGINT, _handle_sigint);
	display_target_info(&p);
	ping_loop(&p);
	display_stats(p.hostname, &p.stats, p.opts.size);

	close(p.sockfd);
	return (p.stats.received > 0 ? EXIT_SUCCESS : RUNTIME_FAILURE);
}
