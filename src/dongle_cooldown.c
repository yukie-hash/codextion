/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle_cooldown.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: a81909 <a81909@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/06 01:31:52 by a81909            #+#    #+#             */
/*   Updated: 2026/10/06 02:06:24 by a81909           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static long	cooldown_end_ms(t_simulation *simulator, int dongle_idx, long now)
{
	if (simulator->dongle_free[dongle_idx]
		&& simulator->dongle_cooldown_until[dongle_idx] > now)
		return (simulator->dongle_cooldown_until[dongle_idx]);
	return (-1);
}

long	earliest_cooldown_end_ms(t_simulation *simulator, t_coder *coder)
{
	long	now;
	long	left_deadline_ms;
	long	right_deadline_ms;

	now = get_current_time_ms();
	left_deadline_ms = cooldown_end_ms(simulator, coder->left_dongle, now);
	right_deadline_ms = cooldown_end_ms(simulator, coder->right_dongle, now);
	if (left_deadline_ms == -1)
		return (right_deadline_ms);
	if (right_deadline_ms == -1)
		return (left_deadline_ms);
	if (right_deadline_ms > left_deadline_ms)
		return (left_deadline_ms);
	return (right_deadline_ms);
}
