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

short	err_msg( char *reason )
{
	printf( "%s%s\n", ERR_PREFIX, reason );
	return ( EXIT_FAILURE );
}
