#pragma once

/**************************\
 * Libraries
\**************************/

# include <unistd.h>
# include <stdlib.h>
# include <stdio.h>
# include <string.h>
# include <errno.h>
# include <signal.h>
# include <stdint.h>

# include <sys/time.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <netinet/ip.h>
# include <netinet/ip_icmp.h>
# include <arpa/inet.h>
# include <netdb.h>
# include <poll.h>

# include "defines.h"
# include "colors.h"

/**************************\
 * Structs
\**************************/

typedef struct s_options
{
	int		verbose;	//	-v
	int		count;		//	-c, 0 = unlimited
	int		ttl;		//	--ttl, 0 = system default
	int		size;		//	-s payload size
	int		timeout;	//	-w, 0 = none
	int		linger;		//	-W, 0 = none
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
	char				ip_str[INET_ADDRSTRLEN];	//	target's IP
	struct sockaddr_in	dest;						//	target's addr -- destination
	int					sockfd;
	uint16_t			id;			//	my id
	uint16_t			seq;		//	current Sequence Number
	t_stats				stats;
}	t_ping;

/**************************\
 * Signals
\**************************/

extern volatile sig_atomic_t	g_stop;

/**************************\
 * Prototypes
\**************************/

// tools/helpers.c
short		err_msg( char *reason );

// srcs/packet.c
uint16_t	packet_checksum( const void *data, size_t len );
size_t		packet_build( t_ping *ping, uint8_t *buf );

// srcs/socket.c
int			socket_init( t_ping *ping );
int			resolve( t_ping* ping );