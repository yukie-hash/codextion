/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder_routine.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: a81909 <a81909@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/21 20:48:48 by a81909            #+#    #+#             */
/*   Updated: 2026/09/30 13:45:30 by a81909           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	do_compile(t_coder *coder)
{
	take_dongles(coder);
	log_event(coder->simulator, coder->id, "is compiling");
	usleep(coder->simulator->args.time_to_compile * 1000);
	release_dongles(coder);
}

static void	do_debug(t_coder *coder)
{
	log_event(coder->simulator, coder->id, "is debugging");
	usleep(coder->simulator->args.time_to_compile * 1000);
}

static void	do_refactor(t_coder *coder)
{
	log_event(coder->simulator, coder->id, "is refactoring");
	usleep(coder->simulator->args.time_to_compile * 1000);
}

void	*coder_routine(void *arg)
{
	t_coder	*coder;

	coder = (t_coder *)arg;
	while (coder->compile_count < coder->simulator->args.compiles_required)
	{
		do_compile(coder);
		coder->compile_count++;
		do_debug(coder);
		do_refactor(coder);
	}
	return (NULL);
}
