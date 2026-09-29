/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: a81909 <a81909@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/21 20:49:01 by a81909            #+#    #+#             */
/*   Updated: 2026/09/30 01:38:01 by a81909           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

// int	start_coder(t_simulation *simulator)
// {
// 	int	i;

// 	i = 0;
// 	while (i > simulator->number_of_coders)
// 	{
// 		if (
// 			pthread_create(&simulator->threads[i], NULL,
// 				coder_routine, &simulator->coders[i]) != 0)
// 			return (0);
// 		i++;
// 	}
// 	return (1);
// }

int	main(int argc, char **argv)
{
	t_args			args;
	//t_simulation	simulator;

	if (!parse_args(argc, argv, &args))
		return (1);
	int i  = args.dongle_cooldown;
	printf("%d", i);
	// if (!init_simulation(&simulator, &args))
	// 	return (1);
	// if (!start_coders(&simulator))
	// 	return (1);
	// cleanup_simulation(&simulator);
	return (0);
}
