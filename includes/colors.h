/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   colors.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: annabrag <annabrag@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/15 18:20:39 by annabrag          #+#    #+#             */
/*   Updated: 2026/08/15 18:24:15 by annabrag         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

/**************************\
 *	Modes
\**************************/

# define NC			"\033[0m"
# define BOLD		"\033[1m"
# define ITAL		"\033[3m"
# define UNDERLINE	"\033[4m"

/**************************\
 *	Standard colors
\**************************/

# define BLACK		"\033[30m"
# define GRAY		"\033[90m"
# define RED		"\033[31m"
# define GREEN		"\033[32m"
# define YELLOW		"\033[33m"
# define SAND		"\033[38;2;231;200;160m"
# define DARK_SAND	"\033[38;2;212;163;115m"
# define ORANGE		"\033[38;5;208m"
# define BLUE		"\033[34m"
# define PURPLE		"\033[35m"
# define HOT_PINK	"\033[38;2;255;105;180m"
# define PINK		"\033[38;2;255;182;193m"
# define CYAN		"\033[36m"
# define WHITE		"\033[37m"

# define L_GRAY		"\033[38;2;173;165;165m"
# define L_GRAY2	"\033[38;2;211;211;211m"

/**************************\
 *	Pastel colors
\**************************/

# define P_GREEN	"\033[38;2;173;235;179m"
# define P_DGREEN	"\033[38;2;152;168;105m"
# define P_BLUE		"\033[38;2;179;235;242m"
# define P_YELLOW	"\033[38;2;255;234;150m"
# define P_PURPLE	"\033[38;2;211;211;255m"
# define P_ORANGE	"\033[38;2;255;178;127m"