/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: a81909 <a81909@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/21 20:48:56 by a81909            #+#    #+#             */
/*   Updated: 2026/10/06 18:32:29 by a81909           ###   ########.fr       */
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

static int	allocate_dongles(t_simulation *simulator, long number_of_coders)
{
	long	i;

	simulator->dongle_free = malloc(sizeof(int) * number_of_coders);
	if (!simulator->dongle_free)
		return (0);
	simulator->dongle_cooldown_until = malloc(sizeof(long) * number_of_coders);
	if (!simulator->dongle_cooldown_until)
	{
		free(simulator->dongle_free);
		return (0);
	}
	memset(simulator->dongle_cooldown_until, 0,
		sizeof(long) * number_of_coders);
	i = 0;
	while (i < number_of_coders)
	{
		simulator->dongle_free[i] = 1;
		i++;
	}
	pthread_mutex_init(&simulator->dongle_mutex, NULL);
	pthread_cond_init(&simulator->dongle_cond, NULL);
	return (1);
}

static void	init_coders(t_simulation *simulator)
{
	long	i;
	long	n;

	n = simulator->args.number_of_coders;
	i = 0;
	while (i < simulator->args.number_of_coders)
	{
		simulator->coders[i].id = i + 1;
		simulator->coders[i].compile_count = 0;
		simulator->coders[i].left_dongle = i;
		simulator->coders[i].right_dongle = (i + 1) % n;
		simulator->coders[i].simulator = simulator;
		i++;
	}
}

int	init_simulation(t_simulation *simulator, t_args *args)
{
	simulator->args = *args;
	if (!allocate_arrays(simulator, args->number_of_coders))
		return (0);
	if (!allocate_dongles(simulator, args->number_of_coders))
	{
		free(simulator->coders);
		free(simulator->threads);
		return (0);
	}
	if (!init_dongle_queues(simulator, args->number_of_coders))
	{
		free_arrays(simulator);
		return (0);
	}
	pthread_mutex_init(&simulator->log_mutex, NULL);
	simulator->next_ticket_number = 0;
	simulator->start_time_ms = get_current_time_ms();  //start時刻ここから？
	if (!simulator->start_time_ms)
		return (0);
	init_coders(simulator);
	return (1);
}
