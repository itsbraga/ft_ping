/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   helpers.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: annabrag <annabrag@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/31 15:49:58 by annabrag          #+#    #+#             */
/*   Updated: 2026/08/31 15:55:08 by annabrag         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_ping.h"

int	err_msg( char *reason, int exit_code )
{
	printf("%s%s\n", ERR_PREFIX, reason);
	return (exit_code);
}

void	debug_hexdump( const uint8_t *buf, size_t len )
{
	size_t	i;

	i = 0;
	while (i < len)
	{
		if (i % 16 == 0)
			printf("\n0x%04zx:  ", i);
		printf("%02x", buf[i]);
		if (i % 2 == 1)
			printf(" ");
		i++;
	}
	printf("\n");
}