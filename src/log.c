/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   log.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: a81909 <a81909@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/21 20:48:59 by a81909            #+#    #+#             */
/*   Updated: 2026/10/11 02:57:37 by a81909           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	log_event(t_simulation *simulator, int coder_id, char *message)
{
	pthread_mutex_lock(&simulator->log_mutex);
	if (!simulator->log_closed)
		printf("%ld %d %s\n", get_elapsed_ms(simulator), coder_id, message);
	pthread_mutex_unlock(&simulator->log_mutex);
}

void	log_burnout(t_simulation *simulator, int coder_id)
{
	pthread_mutex_lock(&simulator->log_mutex);
	if (!simulator->log_closed)
		printf("%ld %d burned out\n", get_elapsed_ms(simulator), coder_id);
	simulator->log_closed = 1;
	pthread_mutex_unlock(&simulator->log_mutex);
}