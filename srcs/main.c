/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: annabrag <annabrag@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/15 18:13:06 by annabrag          #+#    #+#             */
/*   Updated: 2026/08/15 18:28:10 by annabrag         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_ping.h"

short	err_args()
{
	printf("%s%s\n", ERR_PREFIX "Invalid amount of arguments.");
	return (FAILURE);
}

int	main( int argc, char **argv )
{
	if (argc != 2)
		return (err_args());

	return (SUCCESS);
}