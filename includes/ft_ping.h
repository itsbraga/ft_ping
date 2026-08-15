#pragma once

/**************************\
 * Defines
\**************************/

# define ERR_PREFIX "\033[1m\033[31mError: \033[0m"

/**************************\
 * Libraries
\**************************/

# include <unistd.h>
# include <stdio.h>
# include <sys/epoll.h>
# include <netinet/in.h>

# include "colors.h"

/**************************\
 * Structs
\**************************/

typedef struct s_info
{
}				t_info;

/**************************\
 * Defines
\**************************/

# define SUCCESS 0
# define FAILURE 1