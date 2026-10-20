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

static int	fits_in_int(char *str, int len)
{
	while (len > 1 && *str == '0')
	{
		str++;
		len--;
	}
	if (len < 10)
		return (1);
	if (len > 10)
		return (0);
	return (strcmp(str, "2147483647") <= 0);
}

static int	is_valid_number(char *str)
{
	int	i;

	if (!str || !str[0])
		return (0);
	i = 0;
	while (str[i])
	{
		if (str[i] < '0' || str[i] > '9')
			return (0);
		i++;
	}
	return (fits_in_int(str, i));
}

static char	*arg_name(int index)
{
	if (index == 1)
		return ("number_of_coders");
	if (index == 2)
		return ("time_to_burnout");
	if (index == 3)
		return ("time_to_compile");
	if (index == 4)
		return ("time_to_debug");
	if (index == 5)
		return ("time_to_refactor");
	if (index == 6)
		return ("number_of_compiles_required");
	return ("dongle_cooldown");
}

static int	parse_one_field(char **argv, int index, long *dest)
{
	if (!is_valid_number(argv[index]))
	{
		fprintf(stderr, "Error: %s must be an integer between 0 and "
			"2147483647 (got \"%s\")\n", arg_name(index), argv[index]);
		return (0);
	}
	*dest = atoi(argv[index]);
	return (1);
}

int	fill_numeric_fields(char **argv, t_args *args)
{
	if (!parse_one_field(argv, 1, &args->number_of_coders)
		|| !parse_one_field(argv, 2, &args->time_to_burnout)
		|| !parse_one_field(argv, 3, &args->time_to_compile)
		|| !parse_one_field(argv, 4, &args->time_to_debug)
		|| !parse_one_field(argv, 5, &args->time_to_refactor)
		|| !parse_one_field(argv, 6, &args->compiles_required)
		|| !parse_one_field(argv, 7, &args->dongle_cooldown))
		return (0);
	if (args->number_of_coders < 1)
	{
		fprintf(stderr, "Error: number_of_coders must be at least 1\n");
		return (0);
	}
	return (1);
}
