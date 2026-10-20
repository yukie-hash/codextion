/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cleanup.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: a81909 <a81909@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/21 20:48:37 by a81909            #+#    #+#             */
/*   Updated: 2026/10/18 17:59:07 by a81909           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	free_arrays(t_simulation *simulator)
{
	free(simulator->coders);
	free(simulator->threads);
	free(simulator->dongle_free);
	free(simulator->dongle_cooldown_until);
}

static void	join_threads(t_simulation *simulator)
{
	long	i;

	i = 0;
	while (i < simulator->created_coders)
	{
		pthread_join(simulator->threads[i], NULL);
		i++;
	}
	if (simulator->monitor_started)
		pthread_join(simulator->monitor_thread, NULL);
}

void	cleanup_simulation(t_simulation *simulator)
{
	join_threads(simulator);
	pthread_mutex_destroy(&simulator->log_mutex);
	pthread_mutex_destroy(&simulator->dongle_mutex);
	pthread_cond_destroy(&simulator->dongle_cond);
	free(simulator->coders);
	free(simulator->threads);
	free(simulator->dongle_free);
	free(simulator->dongle_cooldown_until);
}
