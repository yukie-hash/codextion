/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parsing_numeric.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: a81909 <a81909@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/23 02:21:06 by a81909            #+#    #+#             */
/*   Updated: 2026/09/30 01:29:36 by a81909           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	fits_in_int(char *str, int len)
{
	char	*int_max;

	int_max = "2147483647";
	if (len < 10)
		return (1);
	if (len > 10)
		return (0);
	return (strcmp(str, int_max) <= 0);
}

int	is_valid_positive_int(char *str)
{
	int	i;

	if (!(str && str[0]))
		return (1);
	i = 0;
	while (str[i])
	{
		if (str[i] < '0' || str[i] > '9')
			return (1);
		i++;
	}
	return (fits_in_int(str, i));
}

int	parse_one_field(char *str, long *dest)
{
	if (!is_valid_positive_int(str))
		return (0);
	*dest = atoi(str);
	return (1);
}

int	fill_numeric_fields(char **argv, t_args *args)
{
	if (!parse_one_field(argv[1], &args->number_of_coders))
		return (0);
	if (!parse_one_field(argv[2], &args->time_to_burnout))
		return (0);
	if (!parse_one_field(argv[3], &args->time_to_compile))
		return (0);
	if (!parse_one_field(argv[4], &args->time_to_debug))
		return (0);
	if (!parse_one_field(argv[5], &args->time_to_refactor))
		return (0);
	if (!parse_one_field(argv[6], &args->compiles_required))
		return (0);
	if (!parse_one_field(argv[7], &args->dongle_cooldown))
		return (0);
	return (1);
}
