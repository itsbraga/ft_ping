/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   helpers.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: annabrag <annabrag@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/31 15:49:58 by annabrag          #+#    #+#             */
/*   Updated: 2026/09/22 02:55:34 by annabrag         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/ft_ping.h"

int	err_msg( char *reason, int exit_code )
{
	fprintf(stderr, "%s%s\n", ERR_PREFIX, reason);
	return (exit_code);
}

const char	*short_name( const char *prog_name )
{
	const char	*slash;

	slash = strrchr(prog_name, '/');
	return (slash ? slash + 1 : prog_name);
}