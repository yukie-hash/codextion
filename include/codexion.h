/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: a81909 <a81909@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/21 20:50:32 by yhamada           #+#    #+#             */
/*   Updated: 2026/09/21 20:58:49 by a81909           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CODEXION_H
# define CODEXION_H

# include <pthread.h>
# include <sys/time.h>
# include <stdlib.h>
# include <unistd.h>
# include <stdio.h>
# include <string.h>

typedef struct s_dongle
{
	int				id;
	int				in_use;
	long			cooldown_until_ms;
	pthread_mutex_t	mutex;
}	t_dongle;

typedef struct s_coder
{
	int					id;
	int					compile_count;
	long				last_compile_start_ms;
	int					burned_out;
	t_dongle			*left;
	t_dongle			*right;
	struct s_simulation	*sim;
}	t_coder;

typedef struct s_simulation
{
	int				number_of_coders;
	long			time_to_burnout;
	long			time_to_compile;
	long			time_to_debug;
	long			time_to_refactor;
	int				compiles_required;
	long			dongle_cooldown;
	int				scheduler_type;
	t_coder			*coders;
	t_dongle		*dongles;
	pthread_mutex_t	log_mutex;
	pthread_mutex_t	state_mutex;
	pthread_cond_t	state_cond;
	int				simulation_over;
	long			start_time_ms;
}	t_simulation;

#endif
