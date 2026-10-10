/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   stop.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: a81909 <a81909@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/11 03:07:41 by a81909            #+#    #+#             */
/*   Updated: 2026/10/11 03:13:17 by a81909           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	is_stopped(t_simulation *simulator)
{
	int	stopped;

	pthread_mutex_lock(&simulator->dongle_mutex);
	stopped = simulator->stop;
	pthread_mutex_unlock(&simulator->dongle_mutex);
	return (stopped);
}
