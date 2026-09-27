/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_ping.h                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: annabrag <annabrag@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/15 19:00:43 by annabrag          #+#    #+#             */
/*   Updated: 2026/09/27 23:09:16 by annabrag         ###   ########.fr       */
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
# include <math.h>
# include <limits.h>
# include <float.h>

# include <sys/time.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <netinet/ip.h>
# include <netinet/ip_icmp.h>
# include <arpa/inet.h>
# include <netdb.h>
# include <poll.h>
# include <getopt.h>
# include <time.h>

# include "defines.h"

/**************************\
 *	ENUM
\**************************/

/*
	getopt_long returns the option letter for short options, so a long-only
	option needs an identifier that cannot be mistaken for one: CHAR_MAX + 1
	is the first value past every character
*/
typedef enum e_long_only
{
	OPT_TTL = CHAR_MAX + 1,
	OPT_USAGE
}	t_long_only;

//	What the loop does after handling a timer expiry or a datagram
typedef enum e_step
{
	STEP_CONTINUE,
	STEP_DONE,
	STEP_FAILED,
}	t_step;

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
	t_options			opts;
	char				*target;
	char				hostname[NI_MAXHOST];			//	canonical name, as printed
	char				target_ip_str[INET_ADDRSTRLEN];
	struct sockaddr_in	dest;							//	target's addr -- destination
	int					sockfd;
	uint16_t			id;								//	our id
	uint16_t			seq;							//	next Echo Request's Sequence Number
	t_stats				stats;
}	t_ping;

/**************************\
 *	SIGNALS
\**************************/

extern volatile sig_atomic_t	g_stop;

/**************************\
 *	PROTOTYPES
\**************************/

//	srcs/tools.c
void		errno_msg( const char *context );
void		err_msg( const char *detail );
const char	*unreach_str( uint8_t code );

// srcs/init.c
void		ping_init( t_ping *p );
int			ping_setup( t_ping *p, int argc, char **argv );

//	srcs/args.c
char		*parse_options( t_options *o, int argc, char **argv );

//	srcs/packet.c
size_t		build_echo_request( t_ping *p, uint8_t *buf );

//	srcs/socket.c
int			create_socket( t_ping *p );
void		apply_socket_options( t_ping *p );
int			resolve_target( t_ping *p );
ssize_t		recv_packet( t_ping *p, uint8_t *buf, size_t buf_size,
				struct sockaddr_in *from );

//	srcs/stats.c
double		compute_rtt( const struct timeval *sent_at );
void		stats_update( t_stats *s, double rtt );
double		stats_stddev( const t_stats *s );

//	srcs/display.c
void		display_missing_host( void );
void		display_invalid_option( int arg );
void		display_unrecognized_option( const char *name );
void		display_missing_argument( int arg );
void		display_long_missing_argument( const char *name );
void		display_long_extra_argument( const char *name );
void		display_invalid_value( const char *value, const char *near );
void		display_value_too_small( const char *value );
void		display_value_too_big( const char *value );
void		display_icmp_error( const struct sockaddr_in *from, size_t icmp_size,
				uint8_t type, uint8_t code );
void		display_help( void );
void		display_target_info( t_ping *p );
void		display_reply( const struct sockaddr_in *from, size_t icmp_size,
				uint16_t seq, uint8_t ttl, const double *rtt );
void		display_stats( const char *host, const t_stats *s, size_t payload_size );

//	srcs/ping.c
int			ping_run( t_ping *p );
