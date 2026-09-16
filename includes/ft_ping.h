#pragma once

/**************************\
 * LIBRARIES
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
 * STRUCTS
\**************************/

typedef struct s_options
{
	unsigned int	verbose;	//	-v
	unsigned int	count;		//	-c, 0 = unlimited
	unsigned int	ttl;		//	--ttl, 0 = system default
	unsigned int	size;		//	-s payload size
	unsigned int	timeout;	//	-w, 0 = none
	unsigned int	linger;		//	-W, DEFAULT_LINGER unless overridden
}	t_options;

typedef struct s_stats
{
	int		sent;
	int		received;
	double	min;
	double	max;
	double	sum;
	double	sum_sq;
}	t_stats;

typedef struct s_ping
{
	t_options			opts;
	char				*prog_name;
	char				*target;
	char				hostname[NI_MAXHOST];		//	canonical name, as printed
	char				ip_str[INET_ADDRSTRLEN];	//	target's IP
	struct sockaddr_in	dest;						//	target's addr -- destination
	int					sockfd;
	uint16_t			id;			//	my id
	uint16_t			seq;		//	current Sequence Number
	t_stats				stats;
}	t_ping;

/**************************\
 * SIGNALS
\**************************/

extern volatile sig_atomic_t	g_stop;

/**************************\
 * PROTOTYPES
\**************************/

// tools/helpers.c
int			err_msg( char *reason, int exit_code );
void		debug_hexdump( const uint8_t *buf, size_t len );

// srcs/packet.c
size_t		build_echo_request( t_ping *p, uint8_t *buf );

// srcs/socket.c
int			create_socket( t_ping *p );
int			resolve_target( t_ping *p );
ssize_t		recv_packet( t_ping *p, uint8_t *buf, size_t size );

// srcs/display.c
void		display_help( const char *prog_name );
void		display_missing_host( const char *prog_name );
void		display_invalid_option( const char *prog_name, int opt );
void		display_unrecognized_option( const char *prog_name, const char *arg );
void		display_missing_argument( const char *prog_name, int opt );
void		display_invalid_value( const char *prog_name, const char *value,
				const char *near );
void		display_unknown_host( const char *prog_name );
void		display_target_info( t_ping *p );
void		display_reply( t_ping *p, const uint8_t* buf, ssize_t received );

// srcs/ping.c
int			init_ping( t_ping *p, int argc, char **argv );
void		ping_loop( t_ping *p );