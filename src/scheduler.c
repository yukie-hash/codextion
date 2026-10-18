/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   scheduler.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: a81909 <a81909@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/21 20:49:11 by a81909            #+#    #+#             */
/*   Updated: 2026/10/18 17:29:16 by a81909           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static long	request_key(t_simulation *simulator, t_coder *coder)
{
	long	key;

	if (simulator->args.scheduler_type == scheduler_edf)
	{
		key = coder->last_compile_start_ms + simulator->args.time_to_burnout;
		return (key);
	}
	key = simulator->next_ticket_number;
	simulator->next_ticket_number++;
	return (key);
}

void	enqueue_request(t_simulation *simulator, t_coder *coder)
{
	long	key;

	key = request_key(simulator, coder);
	heap_push(&simulator->dongle_queues[coder->left_dongle], key, coder->id);
	if (coder->left_dongle != coder->right_dongle)
		heap_push(&simulator->dongle_queues[coder->right_dongle],
			key, coder->id);
}

void	dequeue_request(t_simulation *sim, t_coder *coder)
{
	heap_remove(&sim->dongle_queues[coder->left_dongle], coder->id);
	if (coder->left_dongle != coder->right_dongle)
		heap_remove(&sim->dongle_queues[coder->right_dongle], coder->id);
}

int	has_rival(t_simulation *simulator, t_heap *queue, t_ticket mine_ticket)
{
	int		i;
	long	now;
	t_coder	*other;

	now = get_current_time_ms();
	i = 0;
	while (i < queue->size)
	{
		other = &simulator->coders[queue->tickets[i].coder_id - 1];
		if (ticket_comes_first(&queue->tickets[i], &mine_ticket)
			&& dongle_ready(simulator, other->left_dongle, now)
			&& dongle_ready(simulator, other->right_dongle, now))
			return (1);
		i++;
	}
	return (0);
}

int	is_my_turn(t_simulation *simulator, t_coder *coder)
{
	t_heap		*left_heap;
	t_heap		*right_heap;
	t_ticket	mine_ticket;

	left_heap = &simulator->dongle_queues[coder->left_dongle];
	right_heap = &simulator->dongle_queues[coder->right_dongle];
	mine_ticket = left_heap->tickets[find_ticket(left_heap, coder->id)];
	return (!has_rival(simulator, left_heap, mine_ticket)
		&& !has_rival(simulator, right_heap, mine_ticket));
}
