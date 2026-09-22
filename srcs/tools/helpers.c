/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   helpers.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: annabrag <annabrag@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/31 15:49:58 by annabrag          #+#    #+#             */
/*   Updated: 2026/09/22 17:45:43 by annabrag         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_ping.h"

void	errno_msg( const char *prog_name, const char *context )
{
	fprintf(stderr, "%s: %s: %s\n", prog_name, context, strerror(errno));
}

void	err_msg( const char *prog_name, const char *detail )
{
	fprintf(stderr, "%s: %s\n", prog_name, detail);
}

const char	*short_name( const char *prog_name )
{
	const char	*slash;

	slash = strrchr(prog_name, '/');
	return (slash ? slash + 1 : prog_name);
}