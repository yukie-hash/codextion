/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: a81909 <a81909@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/21 20:48:56 by a81909            #+#    #+#             */
/*   Updated: 2026/09/30 02:59:03 by a81909           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	allocate_arrays(t_simulation *simulator, long number_of_coders)
{
	simulator->coders = malloc(sizeof(t_coder) * number_of_coders);
	if (!simulator->coders)
		return (0);
	simulator->threads = malloc(sizeof(t_coder) * number_of_coders);
	if (!simulator->threads)
	{
		free(simulator->coders);
		return (0);
	}
	return (1);
}

void	init_coders(t_simulation *simulator)
{
	long	i;

	i = 0;
	while (i < simulator->args.number_of_coders)
	{
		simulator->coders[i].id = i + 1;
		simulator->coders[i].compile_count = 0;
		simulator->coders[i].simulator = simulator;
		i++;
	}
}

int	init_simulation(t_simulation *simulator, t_args *args)
{
	simulator->args = *args;
	if (!allocate_arrays(simulator, args->number_of_coders))
		return (0);
	pthread_mutex_init(&simulator->log_mutex, NULL);
	simulator->start_time_ms = get_current_time_ms();  //start時刻ここから？
	if (!simulator->start_time_ms)
		return (0);
	init_coders(simulator);
	return (1);
}
