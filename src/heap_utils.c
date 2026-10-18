/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap_utils.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: a81909 <a81909@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/18 12:00:00 by a81909            #+#    #+#             */
/*   Updated: 2026/10/18 12:00:00 by a81909           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	find_ticket(t_heap *heap, int coder_id)
{
	int	i;

	i = 0;
	while (i < heap->size)
	{
		if (heap->tickets[i].coder_id == coder_id)
			return (i);
		i++;
	}
	return (-1);
}

void	heap_remove(t_heap *heap, int coder_id)
{
	int	i;

	i = find_ticket(heap, coder_id);
	if (i < 0)
		return ;
	heap->size--;
	heap->tickets[i] = heap->tickets[heap->size];
	if (i < heap->size)
	{
		heap_sift_up(heap, i);
		heap_sift_down(heap, i);
	}
}

int	heap_top_id(t_heap *heap)
{
	if (heap->size == 0)
		return (-1);
	return (heap->tickets[0].coder_id);
}
