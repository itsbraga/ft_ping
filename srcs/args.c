/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   args.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: annabrag <annabrag@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/22 02:39:08 by annabrag          #+#    #+#             */
/*   Updated: 2026/09/22 17:45:47 by annabrag         ###   ########.fr       */
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
	return (EXIT_USAGE);
}

int	parse_options( t_ping *p, int argc, char **argv )
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
		return (EXIT_USAGE);
	}
	p->target = argv[optind];
	return (EXIT_SUCCESS);
}