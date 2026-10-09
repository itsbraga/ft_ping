/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   args.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: annabrag <annabrag@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/22 02:39:08 by annabrag          #+#    #+#             */
/*   Updated: 2026/10/09 17:00:15 by annabrag         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_ping.h"

static size_t	_parse_number( const char *value, size_t max, bool allow_zero )
{
	char			*endptr;
	unsigned long	n;

	n = strtoul(value, &endptr, 0);
	if (*endptr != '\0')
		display_invalid_value(value, endptr);
	else if (n == 0 && !allow_zero)
		display_value_too_small(value);
	else if (max != 0 && n > max)
		display_value_too_big(value);
	else
		return (n);
	exit(RUNTIME_FAILURE);
}

static const char	*_long_name( const struct option *long_opts, int val )
{
	while (long_opts->name)
	{
		if (long_opts->val == val)
			return (long_opts->name);
		long_opts++;
	}
	return (NULL);
}

/*
	The return value tells which kind of error, optopt which option caused
	it. A long option is reported by its full name, looked up by value:
	optopt holds the value, and the user may have typed an abbreviation
*/
static void	_reject_option( char **argv, int opt, const struct option *long_opts )
{
	const char	*arg;
	const char	*name;

	arg = argv[optind - 1];
	name = _long_name(long_opts, optopt);
	if (opt == ':' && strncmp(arg, "--", 2) == 0)
		display_long_missing_argument(name);
	else if (opt == ':')
		display_missing_argument(optopt);
	else if (optopt == 0)
		display_unrecognized_option(arg);
	else if (name)
		display_long_extra_argument(name);
	else
		display_invalid_option(optopt);
	exit(EXIT_USAGE);
}

static void	_apply_option( t_options *o, int opt, const struct option *long_opts, char **argv )
{
	if (opt == '?' && optopt == '?')
		opt = OPT_HELP;
	switch (opt)
	{
		case OPT_HELP:
			display_help();
			exit(EXIT_SUCCESS);
		case 'v':
			o->verbose = true;
			break;
		case OPT_USAGE:
			display_usage();
			exit(EXIT_SUCCESS);
		case 'c':
			o->count = _parse_number(optarg, 0, true);
			break;
		case 's':
			o->size = _parse_number(optarg, MAX_PAYLOAD_SIZE, true);
			break;
		case 'w':
			o->timeout = _parse_number(optarg, INT_MAX, false);
			break;
		case 'W':
			o->linger = _parse_number(optarg, INT_MAX, false);
			break;
		case OPT_TTL:
			o->ttl = _parse_number(optarg, MAX_TTL, false);
			break;
		default:
			_reject_option(argv, opt, long_opts);
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
		display_missing_host();
		exit(EXIT_USAGE);
	}
	return (argv[optind]);
}

char	*parse_options( t_options *o, int argc, char **argv )
{
	static const struct option	long_opts[] = {
		{"help", no_argument, NULL, OPT_HELP},
		{"usage", no_argument, NULL, OPT_USAGE},
		{"verbose", no_argument, NULL, 'v'},
		{"count", required_argument, NULL, 'c'},
		{"size", required_argument, NULL, 's'},
		{"ttl", required_argument, NULL, OPT_TTL},
		{"timeout", required_argument, NULL, 'w'},
		{"linger", required_argument, NULL, 'W'},
		{NULL, 0, NULL, 0}
	};
	int	opt;

	opterr = 0;
	while ((opt = getopt_long(argc, argv, ":vc:s:w:W:", long_opts, NULL)) != -1)
		_apply_option(o, opt, long_opts, argv);
	return (_host_operand(argc, argv));
}
