/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   scheduler_rival.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: a81909 <a81909@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/20 15:50:05 by a81909            #+#    #+#             */
/*   Updated: 2026/10/20 15:50:54 by a81909           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	rival_goes_first(t_simulation *simulator, t_coder *rival, long now)
{
	long	rival_start_time;
	long	rival_deadline;

	if (dongle_ready(simulator, rival->left_dongle, now)
		&& dongle_ready(simulator, rival->right_dongle, now))
		return (1);
	rival_start_time = now + simulator->args.time_to_compile
		+ simulator->args.dongle_cooldown;
	rival_deadline = rival->last_compile_start_ms
		+ simulator->args.time_to_burnout;
	return (rival_start_time + DEADLINE_MARGIN_MS > rival_deadline);
}

int	has_rival(t_simulation *simulator, t_heap *queue, t_ticket mine_ticket)
{
	int		i;
	long	now;
	t_coder	*rival;

	now = get_current_time_ms();
	i = 0;
	while (i < queue->size)
	{
		rival = &simulator->coders[queue->tickets[i].coder_id - 1];
		if (ticket_comes_first(&queue->tickets[i], &mine_ticket)
			&& rival_goes_first(simulator, rival, now))
			return (1);
		i++;
	}
	return (0);
}
