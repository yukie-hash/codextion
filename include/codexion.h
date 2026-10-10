/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: a81909 <a81909@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/21 20:50:32 by yhamada           #+#    #+#             */
/*   Updated: 2026/10/11 03:35:22 by a81909           ###   ########.fr       */
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
	int					left_dongle;
	int					right_dongle;
	t_dongle			*left;
	t_dongle			*right;
	struct s_simulation	*simulator;
}	t_coder;

# define QUEUE_CAPACITY 2
# define MONITOR_INTERVAL_US 1000

typedef struct s_ticket
{
	long	key;
	int		coder_id;
}	t_ticket;

typedef struct s_heap
{
	t_ticket	*tickets;
	int			size;
	int			capacity;
}	t_heap;

typedef struct s_simulation
{
	t_args			args;
	t_coder			*coders;
	pthread_t		*threads;
	pthread_t		monitor_thread;
	pthread_mutex_t	log_mutex;
	pthread_mutex_t	dongle_mutex;
	int				*dongle_free;
	pthread_cond_t	dongle_cond;
	long			start_time_ms;
	long			*dongle_cooldown_until;
	t_heap			*dongle_queues;
	long			next_ticket_number;
	int				stop;
	int				log_closed;
}	t_simulation;

int		parse_args(int argc, char **argv, t_args *args);
int		fill_numeric_fields(char **argv, t_args *args);
int		parse_scheduler(char *str, t_args *args);
void	*coder_routine(void *arg);
int		init_simulation(t_simulation *simulator, t_args *args);
long	get_current_time_ms(void);
long	get_elapsed_ms(t_simulation *simulator);
void	log_event(t_simulation *simulator, int coder_id, char *message);
void	cleanup_simulation(t_simulation *simulator);
int		take_dongles(t_coder *coder);
void	release_dongles(t_coder *coder);
long	earliest_cooldown_end_ms(t_simulation *simulator, t_coder *coder);
void	ms_to_timespec(long ms, struct timespec *ts);
void	free_arrays(t_simulation *simulator);
int		init_dongle_queues(t_simulation *simulator, long number_of_coders);
int		heap_top_id(t_heap *heap);
void	enqueue_request(t_simulation *simulator, t_coder *coder);
void	dequeue_request(t_simulation *simulator, t_coder *coder);
int		is_my_turn(t_simulation *simulator, t_coder *coder);
int		heap_push(t_heap *heap, long key, int coder_id);
void	heap_pop(t_heap *heap);
void	*monitor_routine(void *arg);
void	log_burnout(t_simulation *simulator, int coder_id);
int		is_stopped(t_simulation *simulator);

#endif
