/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   helpers.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: annabrag <annabrag@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/31 15:49:58 by annabrag          #+#    #+#             */
/*   Updated: 2026/09/22 17:55:51 by annabrag         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_ping.h"

void	errno_msg( const char *prog, const char *context )
{
	fprintf(stderr, "%s: %s: %s\n", prog, context, strerror(errno));
}

void	err_msg( const char *prog, const char *detail )
{
	fprintf(stderr, "%s: %s\n", prog, detail);
}

const char	*short_name( const char *prog )
{
	const char	*slash;

	slash = strrchr(prog, '/');
	return (slash ? slash + 1 : prog);
}