/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   stop.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: a81909 <a81909@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/11 03:07:41 by a81909            #+#    #+#             */
/*   Updated: 2026/10/18 17:54:18 by a81909           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	is_stopped(t_simulation *simulator)
{
	int	stopped;

	pthread_mutex_lock(&simulator->dongle_mutex);
	stopped = simulator->stop;
	pthread_mutex_unlock(&simulator->dongle_mutex);
	return (stopped);
}

void	request_stop(t_simulation *sim)
{
	pthread_mutex_lock(&sim->dongle_mutex);
	sim->stop = 1;
	pthread_cond_broadcast(&sim->dongle_cond);
	pthread_mutex_unlock(&sim->dongle_mutex);
}

int	sleep_ms(t_simulation *sim, long ms)
{
	long	end_time_ms;
	long	left;

	end_time_ms = get_current_time_ms() + ms;
	left = ms;
	while (left > 0 && !is_stopped(sim))
	{
		if (left > SLEEP_SLICE_MS)
			left = SLEEP_SLICE_MS;
		usleep(left * 1000);
		left = end_time_ms - get_current_time_ms();
	}
	return (left <= 0);
}
