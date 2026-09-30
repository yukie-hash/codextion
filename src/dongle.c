/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: a81909 <a81909@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/21 20:48:51 by a81909            #+#    #+#             */
/*   Updated: 2026/09/30 14:36:40 by a81909           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	dongles_available(t_simulation *simulator, t_coder *coder)
{
	return (simulator->dongle_free[coder->left_dongle]
		&& simulator->dongle_free[coder->right_dongle]);
}

void	take_dongles(t_coder *coder)
{
	t_simulation	*simulator;

	simulator = coder->simulator;
	pthread_mutex_lock(&simulator->dongle_mutex);
	while (!dongles_available(simulator, coder))
		pthread_cond_wait(&simulator->dongle_cond, &simulator->dongle_mutex);
	simulator->dongle_free[coder->left_dongle] = 0;
	simulator->dongle_free[coder->right_dongle] = 0;
	pthread_mutex_unlock(&simulator->dongle_mutex);
	log_event(simulator, coder->id, "has taken a dongle");
	log_event(simulator, coder->id, "has taken a dongle");
}

void	release_dongles(t_coder *coder)
{
	t_simulation	*simulator;

	simulator = coder->simulator;
	pthread_mutex_lock(&simulator->dongle_mutex);
	simulator->dongle_free[coder->left_dongle] = 1;
	simulator->dongle_free[coder->right_dongle] = 1;
	pthread_cond_broadcast(&simulator->dongle_cond);
	pthread_mutex_unlock(&simulator->dongle_mutex);
}
