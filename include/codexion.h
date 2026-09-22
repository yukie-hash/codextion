/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: a81909 <a81909@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/21 20:50:32 by yhamada           #+#    #+#             */
/*   Updated: 2026/09/23 02:05:32 by a81909           ###   ########.fr       */
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
	long			number_of_coders;
	long			time_to_burnout;
	long			time_to_compile;
	long			time_to_debug;
	long			time_to_refactor;
	long			compiles_required;
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

typedef enum e_scheduler
{
	scheduler_fifo,
	scheduler_edf
}	t_scheduler;

int	parse_args(int argc, char **argv, t_simulation *sim);
int	fill_numeric_fields(char **argv, t_simulation *simulator);
int	parse_scheduler(char *str, t_simulation *simulator);

#endif
