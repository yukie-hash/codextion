/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: a81909 <a81909@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/21 20:48:51 by a81909            #+#    #+#             */
/*   Updated: 2026/10/06 14:57:26 by a81909           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	dongle_ready(t_simulation *simulator, int dongle_index, long now)
{
	return (simulator->dongle_free[dongle_index]
		&& simulator->dongle_cooldown_until[dongle_index] <= now);
}

static int	can_take(t_simulation *simulator, t_coder *coder)
{
	long	now;

	now = get_current_time_ms();
	return (dongle_ready(simulator, coder->left_dongle, now)
		&& dongle_ready(simulator, coder->right_dongle, now)
		&& is_my_turn(simulator, coder));
}

static void	wait_for_dangles(t_simulation *simulator, t_coder *coder)
{
	long			cooldown_end;
	struct timespec	ts;

	cooldown_end = earliest_cooldown_end_ms(simulator, coder);
	if (cooldown_end == -1)
	{
		pthread_cond_wait(&simulator->dongle_cond, &simulator->dongle_mutex);
	}
	else
	{
		ms_to_timespec(cooldown_end, &ts);
		pthread_cond_timedwait(&simulator->dongle_cond, &simulator->dongle_mutex, &ts);
	}
}

void	take_dongles(t_coder *coder)
{
	t_simulation	*simulator;

	simulator = coder->simulator;
	pthread_mutex_lock(&simulator->dongle_mutex);
	enqueue_request(simulator, coder);
	while (!can_take(simulator, coder))
		wait_for_dangles(simulator, coder);
	dequeue_request(simulator, coder);
	simulator->dongle_free[coder->left_dongle] = 0;
	simulator->dongle_free[coder->right_dongle] = 0;
	pthread_mutex_unlock(&simulator->dongle_mutex);
	log_event(simulator, coder->id, "has taken a dongle");
	log_event(simulator, coder->id, "has taken a dongle");
}

void	release_dongles(t_coder *coder)
{
	t_simulation	*simulator;
	long			until;

	simulator = coder->simulator;
	until = get_current_time_ms() + simulator->args.dongle_cooldown;
	pthread_mutex_lock(&simulator->dongle_mutex);
	simulator->dongle_free[coder->left_dongle] = 1;
	simulator->dongle_free[coder->right_dongle] = 1;
	simulator->dongle_cooldown_until[coder->left_dongle] = until;
	simulator->dongle_cooldown_until[coder->right_dongle] = until;
	pthread_cond_broadcast(&simulator->dongle_cond);
	pthread_mutex_unlock(&simulator->dongle_mutex);
}
