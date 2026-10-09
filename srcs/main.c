/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: annabrag <annabrag@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/15 18:13:06 by annabrag          #+#    #+#             */
/*   Updated: 2026/10/09 17:00:37 by annabrag         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_ping.h"

volatile sig_atomic_t	g_stop = 0;

static void	_handle_sigint( int sig __attribute__((unused)) )
{
	g_stop = 1;
}

static void	_setup_signals( void )
{
	struct sigaction	sa;

	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = _handle_sigint;
	sigaction(SIGINT, &sa, NULL);
}

int	main( int argc, char **argv )
{
	t_ping	p;
	int		ret;

	setvbuf(stdout, NULL, _IOLBF, 0);
	ping_init(&p);
	if (ping_setup(&p, argc, argv) != EXIT_SUCCESS)
		return (RUNTIME_FAILURE);
	_setup_signals();
	ret = ping_run(&p);
	close(p.sockfd);
	return (ret);
}
