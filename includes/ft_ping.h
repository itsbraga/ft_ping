/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_ping.h                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: annabrag <annabrag@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/15 19:00:43 by annabrag          #+#    #+#             */
/*   Updated: 2026/09/22 04:00:54 by annabrag         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

/**************************\
 *	LIBRARIES
\**************************/

# include <unistd.h>
# include <stdlib.h>
# include <stdio.h>
# include <string.h>
# include <errno.h>
# include <signal.h>
# include <stdint.h>
# include <stdbool.h>

# include <sys/time.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <netinet/ip.h>
# include <netinet/ip_icmp.h>
# include <arpa/inet.h>
# include <netdb.h>
# include <poll.h>
# include <time.h>
# include <getopt.h>

# include "defines.h"
# include "colors.h"

/**************************\
 *	ENUMS
\**************************/

typedef enum e_parse
{
	PARSE_OK,
	PARSE_HELP,
	PARSE_USAGE_ERROR,
	PARSE_VALUE_ERROR,
}	t_parse;

typedef enum e_opt_id
{
	OPT_VERBOSE,
	OPT_HELP,
	OPT_COUNT,
	OPT_SIZE,
	OPT_TTL,
	OPT_TIMEOUT,
	OPT_LINGER
}	t_opt_id;

/**************************\
 *	STRUCTS
\**************************/

typedef struct s_options
{
	bool	verbose;	//	-v
	size_t	count;		//	-c, 0 = unlimited
	size_t	size;		//	-s
	size_t	ttl;		//	--ttl, 0 = system default
	size_t	timeout;	//	-w, 0 = none
	size_t	linger;		//	-W, DEFAULT_LINGER unless overridden
}	t_options;

typedef struct s_stats
{
	size_t	transmitted;	//	Echo Requests sent
	size_t	received;		//	Echo Replies received
	double	rtt_min;		//	minimum round-trip time
	double	rtt_max;		//	maximum round-trip time
	double	rtt_sum;		//	sum of all times, for doing average
	double	rtt_sum_sq;		//	sum of all times squared, for stddev
}	t_stats;

typedef struct s_ping
{
	const char			*prog_name;
	t_options			opts;
	char				*target;
	char				hostname[NI_MAXHOST];	//	canonical name, as printed
	char				target_ip_str[INET_ADDRSTRLEN];
	struct sockaddr_in	dest;	//	target's addr -- destination
	int					sockfd;
	uint16_t			id;		//	my id
	uint16_t			seq;	//	next Echo Request's Sequence Number
	t_stats				stats;
}	t_ping;

/**************************\
 *	SIGNALS
\**************************/

extern volatile sig_atomic_t	g_stop;

/**************************\
 *	PROTOTYPES
\**************************/

//	srcs/tools/helpers.c
int			err_msg( char *reason, int exit_code );
const char	*short_name( const char *prog_name );

//	srcs/packet.c
size_t		build_echo_request( t_ping *p, uint8_t *buf );

//	srcs/socket.c
int			create_socket( t_ping *p );
int			resolve_target( t_ping *p );
ssize_t		recv_packet( t_ping *p, uint8_t *buf, size_t buf_size );

//	srcs/stats.c
double		compute_rtt( const struct timeval *sent_at );
void		stats_update( t_stats *s, double rtt );

//	srcs/display.c
void		display_missing_host( const char *prog_name );
void		display_invalid_option( const char *prog_name, int opt );
void		display_unrecognized_option( const char *prog_name, const char *arg );
void		display_missing_argument( const char *prog_name, int opt );
void		display_invalid_value( const char *prog_name, const char *value,
				const char *near );
void		display_unknown_host( const char *prog_name );
void		display_target_info( t_ping *p );
void		display_reply( t_ping *p, const uint8_t* buf, ssize_t datagram_size );
void		display_stats( const char *host, const t_stats *s, size_t payload_size );

//	srcs/options.c
void		print_help( const char *prog_name );

//	srcs/args.c
int			parse_options( t_ping *p, int argc, char **argv );

//	srcs/ping.c
int			ping_init( t_ping *p, int argc, char **argv );
void		ping_loop( t_ping *p );