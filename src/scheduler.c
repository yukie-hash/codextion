/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   scheduler.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: a81909 <a81909@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/21 20:49:11 by a81909            #+#    #+#             */
/*   Updated: 2026/10/14 19:56:40 by a81909           ###   ########.fr       */
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
	//  コーダー一人対策用 本来であればバーンアウト
	if (coder->left_dongle != coder->right_dongle)
		heap_push(&simulator->dongle_queues[coder->right_dongle], key, coder->id);
}

void	dequeue_request(t_simulation *sim, t_coder *coder)
{
	heap_pop(&sim->dongle_queues[coder->left_dongle]);
	if (coder->left_dongle != coder->right_dongle)
		heap_pop(&sim->dongle_queues[coder->right_dongle]);
}

int	is_my_turn(t_simulation *simulator, t_coder *coder)
{
	return (heap_top_id(&simulator->dongle_queues[coder->left_dongle])
		== coder->id
		&& heap_top_id(&simulator->dongle_queues[coder->right_dongle])
		== coder->id);
}
