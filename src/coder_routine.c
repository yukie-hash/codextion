/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder_routine.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: a81909 <a81909@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/21 20:48:48 by a81909            #+#    #+#             */
/*   Updated: 2026/10/16 15:33:08 by a81909           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	do_compile(t_coder *coder)
{
	int	done;

	if (!take_dongles(coder))
		return (0);
	log_event(coder->simulator, coder->id, "is compiling");
	done = sleep_ms(coder->simulator, coder->simulator->args.time_to_compile);
	release_dongles(coder);
	return (done);
}

static int	do_debug(t_coder *coder)
{
	log_event(coder->simulator, coder->id, "is debugging");
	return (sleep_ms(coder->simulator, coder->simulator->args.time_to_debug));
}

static int	do_refactor(t_coder *coder)
{
	log_event(coder->simulator, coder->id, "is refactoring");
	return (sleep_ms(coder->simulator, coder->simulator->args.time_to_refactor));
}

static void	lone_coder(t_coder *coder)
{
	t_simulation	*simulator;

	simulator = coder->simulator;
	log_event(coder->simulator, coder->id, "has taken a dongle");
	pthread_mutex_lock(&simulator->dongle_mutex);
	while (!simulator->stop)
		pthread_cond_wait(&simulator->dongle_cond, &simulator->dongle_mutex);
	pthread_mutex_unlock(&simulator->dongle_mutex);
}

void	*coder_routine(void *arg)
{
	t_coder	*coder;

	coder = (t_coder *)arg;
	if (coder->left_dongle == coder->right_dongle)
	{
		lone_coder(coder);
		return (NULL);
	}
	while (!is_stopped(coder->simulator)
		&& coder->compile_count < coder->simulator->args.compiles_required)
	{
		if (!do_compile(coder) || !do_debug(coder) || !do_refactor(coder))
			return (NULL);
	}
	return (NULL);
}
