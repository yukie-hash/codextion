/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: a81909 <a81909@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/21 20:50:32 by yhamada           #+#    #+#             */
/*   Updated: 2026/09/30 03:23:00 by a81909           ###   ########.fr       */
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

typedef enum e_scheduler
{
	scheduler_fifo,
	scheduler_edf
}	t_scheduler;

typedef struct s_args
{
	long		number_of_coders;
	long		time_to_burnout;
	long		time_to_compile;
	long		time_to_debug;
	long		time_to_refactor;
	long		compiles_required;
	long		dongle_cooldown;
	t_scheduler	scheduler_type;
}	t_args;

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
	struct s_simulation	*simulator;
}	t_coder;

typedef struct s_simulation
{
	t_args			args;
	t_coder			*coders;
	pthread_t		*threads;
	pthread_mutex_t	log_mutex;
	long			start_time_ms;
}	t_simulation;

int		parse_args(int argc, char **argv, t_args *args);
int		fill_numeric_fields(char **argv, t_args *args);
int		parse_scheduler(char *str, t_args *args);
void	*coder_routine(void *arg);
int		init_simulation(t_simulation *simulator, t_args *args);
int		get_current_time_ms(void);
long	get_elapsed_ms(t_simulation *simulator);
void	log_event(t_simulation *simulator, int coder_id, char *message);
void	cleanup_simulation(t_simulation *simulator);

#endif
