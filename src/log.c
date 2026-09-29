/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   log.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: a81909 <a81909@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/21 20:48:59 by a81909            #+#    #+#             */
/*   Updated: 2026/09/30 03:09:52 by a81909           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	log_event(t_simulation *simulator, int coder_id, char *message)
{
	pthread_mutex_lock(&simulator->log_mutex);
	printf("%ld %d %s\n", get_elapsed_ms(simulator), coder_id, message);
	pthread_mutex_unlock(&simulator->log_mutex);
}
