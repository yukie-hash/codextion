/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: a81909 <a81909@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/21 20:49:04 by a81909            #+#    #+#             */
/*   Updated: 2026/10/11 03:31:44 by a81909           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	find_burned_out(t_simulation *simulator)
{
	long	i;
	long	now;
	t_coder	*coder;

	now = get_current_time_ms();
	i = 0;
	while (i < simulator->args.number_of_coders)
	{
		coder = &simulator->coders[i];
		if (coder->compile_count < simulator->args.compiles_required
			&& now - coder->last_compile_start_ms
			> simulator->args.time_to_burnout)
			return (coder->id);
		i++;
	}
	return (0);
}

static int	all_finished(t_simulation *simulator)
{
	long	i;

	i = 0;
	while (i < simulator->args.number_of_coders)
	{
		if (simulator->coders[i].compile_count
			< simulator->args.compiles_required)
			return (0);
		i++;
	}
	return (1);
}

static int	check_end_condition(t_simulation *simulator)
{
	int	result;

	pthread_mutex_lock(&simulator->dongle_mutex);
	result = find_burned_out(simulator);
	if (result == 0 && all_finished(simulator))
		result = -1;
	if (result != 0)
	{
		simulator->stop = 1;
		pthread_cond_broadcast(&simulator->dongle_cond);
	}
	pthread_mutex_unlock(&simulator->dongle_mutex);
	return (result);
}

void	*monitor_routine(void *arg)
{
	t_simulation	*simulator;
	int				result;

	simulator = (t_simulation *)arg;
	result = check_end_condition(simulator);
	while (result == 0)
	{
		usleep(MONITOR_INTERVAL_US);
		result = check_end_condition(simulator);
	}
	if (result > 0)
		log_burnout(simulator, result);
	return (NULL);
}
