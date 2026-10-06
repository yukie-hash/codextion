/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: a81909 <a81909@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/21 20:48:54 by a81909            #+#    #+#             */
/*   Updated: 2026/10/06 22:55:07 by a81909           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	request_less(t_ticket *a, t_ticket *b)
{
	if (a->key != b->key)
		return (a->key < b->key);
	// edf用。keyが同じだったらコーダーidが小さい方を優先する。
	return (a->coder_id < b->coder_id);
}

static void	swap_request(t_ticket *a, t_ticket *b)
{
	t_ticket	tmp;

	tmp = *a;
	*a = *b;
	*b = tmp;
}

void	heap_shift_up(t_heap *heap, int i)
{
	int	parent;

	while (i > 0)
	{
		parent = (i - 1) / 2;
		if (!request_less(&heap->tickets[i], &heap->tickets[parent]))
			return ;
		swap_request(&heap->tickets[i], &heap->tickets[parent]);
		i = parent;
	}
}

void	heap_shift_down(t_heap *heap, int current)
{
	int	min_index;
	int	left_child;
	int	right_child;

	while (1)
	{
		min_index = current;
		left_child = 2 * current + 1;
		right_child = 2 * current + 2;
		if (left_child < heap->size
			&& request_less(&heap->tickets[left_child],
				&heap->tickets[min_index]))
			min_index = left_child;
		if (right_child < heap->size
			&& request_less(&heap->tickets[right_child],
				&heap->tickets[min_index]))
			min_index = right_child;
		if (min_index == current)
			return ;
		swap_request(&heap->tickets[current], &heap->tickets[min_index]);
		current = min_index;
	}
}

int	heap_push(t_heap *heap, long key, int coder_id)
{
	if (heap->size >= heap->capacity)
		return (0);
	heap->tickets[heap->size].key = key;
	heap->tickets[heap->size].coder_id = coder_id;
	heap->size++;
	heap_shift_up(heap, heap->size - 1);
	return (1);
}

void	heap_pop(t_heap *heap)
{
	if (heap->size == 0)
		return ;
	heap->size--;
	heap->tickets[0] = heap->tickets[heap->size];
	heap_shift_down(heap, 0);
}

int	heap_top_id(t_heap *heap)
{
	if (heap->size == 0)
		return (-1);
	return (heap->tickets[0].coder_id);
}
