/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cleanup.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: a81909 <a81909@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/21 20:48:37 by a81909            #+#    #+#             */
/*   Updated: 2026/10/06 03:30:02 by a81909           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	cleanup_simulation(t_simulation *simulator)
{
	long	i;

	i = 0;
	while (i < simulator->args.number_of_coders)
	{
		pthread_join(simulator->threads[i], NULL);
		i++;
	}
	pthread_mutex_destroy(&simulator->log_mutex);
	pthread_mutex_destroy(&simulator->dongle_mutex);
	pthread_cond_destroy(&simulator->dongle_cond);
	free(simulator->coders);
	free(simulator->threads);
	free(simulator->dongle_free);
	free(sim->dongle_cooldown_until); 
}
