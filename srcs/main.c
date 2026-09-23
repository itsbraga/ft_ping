/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: annabrag <annabrag@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/15 18:13:06 by annabrag          #+#    #+#             */
/*   Updated: 2026/09/22 17:56:18 by annabrag         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_ping.h"

volatile sig_atomic_t	g_stop = 0;

static void	_handle_sigint( int sig __attribute__((unused)) )
{
	g_stop = 1;
}

int	main( int argc, char **argv )
{
	t_ping	p;
	int		ret;

	if (argc < 2)
		fprintf(stderr, "%s %s\n", ERR_PREFIX, "Not enough arguments");
	setvbuf(stdout, NULL, _IOLBF, 0);
	ping_init(&p, argv);
	ping_setup(&p, argc, argv);
	signal(SIGINT, _handle_sigint);
	ret = ping_run(&p);
	close(p.sockfd);
	return (ret);
}
