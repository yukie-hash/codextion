/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   scheduler_init.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: a81909 <a81909@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/21 20:49:09 by a81909            #+#    #+#             */
/*   Updated: 2026/10/06 16:04:51 by a81909           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	heap_destroy(t_heap *heap)
{
	free(heap->tickets);
	heap->tickets = NULL;
	heap->size = 0;
}

void	destroy_dongle_queues(t_heap *queues, long count)
{
	long	i;

	i = 0;
	while (i < count)
	{
		heap_destroy(&queues[i]);
		i++;
	}
	free(queues);
}

static int	heap_init(t_heap *heap, int capacity)
{
	heap->tickets = malloc(sizeof(t_ticket) * capacity);
	if (!heap->tickets)
		return (0);
	heap->size = 0;
	heap->capacity = capacity;
	return (1);
}

int	init_dongle_queues(t_simulation *simulator, long number_of_coders)
{
	long	i;

	simulator->dongle_queues = malloc(sizeof(t_heap) * number_of_coders);
	if (!simulator->dongle_queues)
		return (0);
	i = 0;
	while (i < number_of_coders)
	{
		if (!heap_init(&simulator->dongle_queues[i], QUEUE_CAPACITY))
		{
			destroy_dongle_queues(simulator->dongle_queues, i);
			return (0);
		}
		i++;
	}
	return (1);
}
