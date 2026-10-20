/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: a81909 <a81909@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/21 20:49:01 by a81909            #+#    #+#             */
/*   Updated: 2026/10/18 18:04:44 by a81909           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	start_coders(t_simulation *simulator)
{
	int	i;

	i = 0;
	while (i < simulator->args.number_of_coders)
	{
		if (
			pthread_create(&simulator->threads[i], NULL,
				coder_routine, &simulator->coders[i]) != 0)
			return (0);
		simulator->created_coders++;
		i++;
	}
	return (1);
}

static int	start_threads(t_simulation *simulator)
{
	if (!start_coders(simulator))
		return (0);
	if (pthread_create(&simulator->monitor_thread, NULL,
			monitor_routine, simulator) != 0)
		return (0);
	simulator->monitor_started = 1;
	return (1);
}

int	main(int argc, char **argv)
{
	t_args			args;
	t_simulation	simulator;

	if (!parse_args(argc, argv, &args))
		return (1);
	if (!init_simulation(&simulator, &args))
	{
		fprintf(stderr, "Error: memory allocation failed\n");
		return (1);
	}
	if (!start_threads(&simulator))
	{
		fprintf(stderr, "Error: failed to create a thread\n");
		request_stop(&simulator);
		cleanup_simulation(&simulator);
		return (1);
	}
	cleanup_simulation(&simulator);
	return (0);
}
