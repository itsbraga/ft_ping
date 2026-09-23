/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   args.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: panther <panther@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/22 02:39:08 by annabrag          #+#    #+#             */
/*   Updated: 2026/09/22 23:17:12 by panther          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_ping.h"

static size_t	_parse_number( const char *prog, const char *arg, size_t max,
					bool allow_zero )
{
	char			*endptr;
	unsigned long	n;

	if (arg[0] == '-')
		display_invalid_value(prog, arg, arg);
	else
	{
		errno = 0;
		n = strtoul(arg, &endptr, 10);
		if (*endptr != '\0' || endptr == arg)
			display_invalid_value(prog, arg, endptr);
		else if (errno == ERANGE || (max != 0 && n > max))
			display_value_too_big(prog, arg);
		else if (n == 0 && !allow_zero)
			display_value_too_small(prog, arg);
		else
			return (n);
	}
	exit(EXIT_USAGE);
}

/*
	The return value tells which kind of error, optopt which option caused
	it: a short one is reported by its letter, a long one leaves optopt at
	zero and is read back from the command line instead
*/
static void	_reject_option( const char *prog, int opt, char **argv )
{
	if (opt == ':')
		display_missing_argument(prog, optopt);
	else if (optopt)
		display_invalid_option(prog, optopt);
	else
		display_unrecognized_option(prog, argv[optind - 1]);
	exit(EXIT_USAGE);
}

static void	_apply_option( t_options *o, int opt, char **argv )
{
	switch (opt)
	{
		case '?':
			display_help(argv[0]);
			exit(EXIT_SUCCESS);
		case 'v':
			o->verbose = true;
			break;
		case 'c':
			o->count = _parse_number(argv[0], optarg, 0, true);
			break;
		case 's':
			o->size = _parse_number(argv[0], optarg, MAX_PAYLOAD_SIZE, true);
			break;
		case 'w':
			o->timeout = _parse_number(argv[0], optarg, INT_MAX, false);
			break;
		case 'W':
			o->linger = _parse_number(argv[0], optarg, INT_MAX, false);
			break;
		case OPT_TTL:
			o->ttl = _parse_number(argv[0], optarg, MAX_TTL, false);
			break;
		default:
			_reject_option(argv[0], opt, argv);
	}
}

/*
	GNU getopt moves operands after the options, so the host is always
	at optind, wherever it was typed on the command line
*/
static char	*_host_operand( int argc, char **argv )
{
	if (optind >= argc)
	{
		display_missing_host(argv[0]);
		exit(EXIT_USAGE);
	}
	return (argv[optind]);
}

char	*parse_options( t_options *o, int argc, char **argv )
{
	static const struct option	long_opts[] = {
		{"help", no_argument, NULL, '?'},
		{"verbose", no_argument, NULL, 'v'},
		{"count", required_argument, NULL, 'c'},
		{"size", required_argument, NULL, 's'},
		{"ttl", required_argument, NULL, OPT_TTL},
		{"timeout", required_argument, NULL, 'w'},
		{"linger", required_argument, NULL, 'W'},
		{NULL, 0, NULL, 0}
	};
	int							opt;

	opterr = 0;
	while ((opt = getopt_long(argc, argv, ":?vc:s:w:W:", long_opts, NULL)) != -1)
		_apply_option(o, opt, argv);
	return (_host_operand(argc, argv));
}
