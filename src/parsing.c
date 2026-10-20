/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parsing.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: a81909 <a81909@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/21 20:49:06 by a81909            #+#    #+#             */
/*   Updated: 2026/09/30 01:29:09 by a81909           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	parse_scheduler(char *str, t_args *args)
{
	if (strcmp(str, "fifo") == 0)
	{
		args->scheduler_type = scheduler_fifo;
		return (1);
	}
	if (strcmp(str, "edf") == 0)
	{
		args->scheduler_type = scheduler_edf;
		return (1);
	}
	return (0);
}

static void	print_usage(void)
{
	fprintf(stderr, "Usage: ./codexion number_of_coders time_to_burnout "
		"time_to_compile time_to_debug time_to_refactor "
		"number_of_compiles_required dongle_cooldown scheduler\n"
		"  scheduler: fifo or edf\n");
}

int	parse_args(int argc, char **argv, t_args *args)
{
	if (argc != 9)
	{
		fprintf(stderr, "Error: expected 8 arguments, got %d\n", argc - 1);
		print_usage();
		return (0);
	}
	if (!fill_numeric_fields(argv, args))
	{
		print_usage();
		return (0);
	}
	if (!parse_scheduler(argv[8], args))
	{
		fprintf(stderr, "Error: scheduler must be \"fifo\" or \"edf\" "
			"(got \"%s\")\n", argv[8]);
		print_usage();
		return (0);
	}
	return (1);
}
