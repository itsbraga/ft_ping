/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   packet.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: annabrag <annabrag@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/26 19:01:23 by annabrag          #+#    #+#             */
/*   Updated: 2026/08/26 21:10:44 by annabrag         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_ping.h"

uint16_t	packet_checksum( const void *data, size_t len );

size_t		packet_build( t_ping *ping, uint8_t *buf );