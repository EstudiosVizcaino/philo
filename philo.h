/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   philo.h                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cvizcain <cvizcain@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/16 18:22:34 by cvizcain          #+#    #+#             */
/*   Updated: 2026/03/17 18:12:00 by cvizcain         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/**
 * @file philo.h
 * @brief Shared types and public function prototypes for philo.
 *
 * Defines the two core data structures (t_philo and t_data) and
 * the function signatures implemented across the .c source files.
 * All mutable shared state is protected by the mutexes declared
 * inside t_data; see individual function documentation for the
 * required lock order.
 */
#ifndef PHILO_H
# define PHILO_H

# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <unistd.h>
# include <sys/time.h>
# include <pthread.h>

typedef struct s_data	t_data;

/**
 * @brief Per-philosopher state and POSIX thread handle.
 *
 * One t_philo instance is allocated per philosopher. The fields
 * @c last_meal_time and @c meals_eaten are mutable shared state
 * and must be read or written while holding t_data::meal_mutex.
 * The fork pointers are set once during initialisation and never
 * changed afterwards.
 */
typedef struct s_philo
{
	int				id;
	int				meals_eaten;
	long long		last_meal_time;
	pthread_t		thread;
	pthread_mutex_t	*left_fork;
	pthread_mutex_t	*right_fork;
	t_data			*data;
}	t_philo;

/**
 * @brief Simulation-wide configuration and shared state.
 *
 * Holds the parsed argument values, all synchronisation primitives,
 * and the philosopher array. Fields @c dead, @c all_ate, @c ready,
 * @c finished_eating, and @c start_time, as well as the per-philo
 * fields @c last_meal_time and @c meals_eaten, are all protected by
 * @c meal_mutex. @c print_mutex serialises stdout writes.
 * Lock order: acquire @c print_mutex before @c meal_mutex.
 */
typedef struct s_data
{
	int				num_philos;
	long long		time_to_die;
	long long		time_to_eat;
	long long		time_to_sleep;
	int				must_eat;
	int				dead;
	int				all_ate;
	int				ready;
	int				finished_eating;
	long long		start_time;
	long long		think_time;
	pthread_mutex_t	*forks;
	pthread_mutex_t	print_mutex;
	pthread_mutex_t	meal_mutex;
	pthread_t		monitor;
	t_philo			*philos;
}	t_data;

/* init.c */

/**
 * @brief Initialise all simulation data from command-line arguments.
 *
 * Parses @p argv, computes derived timing values, and allocates and
 * initialises all mutexes and the philosopher array.
 *
 * @param data  Pointer to the uninitialised t_data structure.
 * @param argc  Argument count (5 or 6).
 * @param argv  Argument vector beginning at @c argv[1].
 * @return 1 on success, 0 on any allocation or mutex error.
 */
int			init_data(t_data *data, int argc, char **argv);

/**
 * @brief Release all resources owned by @p data.
 *
 * Destroys every fork mutex, frees the fork and philosopher arrays,
 * and destroys the print and meal mutexes. Safe to call after a
 * partial initialisation (checks for NULL pointers).
 *
 * @param data  Pointer to the t_data structure to clean up.
 */
void		cleanup(t_data *data);

/* routine.c */

/**
 * @brief Entry point for each philosopher thread.
 *
 * Spins until the start barrier is released, then enters the
 * eat–sleep–think loop. Even-ID philosophers are staggered by
 * time_to_eat milliseconds at startup. This delay is chosen so that
 * the even group begins trying to pick up forks exactly when the odd
 * group has finished eating and released them, preventing the initial
 * contention burst that would otherwise cause starvation with large N
 * and a tight time_to_die/cycle ratio.
 * The single-philosopher edge case is handled separately.
 *
 * @param arg  Pointer to the philosopher's t_philo struct.
 * @return Always NULL.
 */
void		*philo_routine(void *arg);

/* monitor.c */

/**
 * @brief Spin until the start barrier (data->ready) is set.
 *
 * All threads park here until start_threads() records start_time
 * and sets the ready flag under meal_mutex. Used by both philosopher
 * threads and the monitor thread as the shared start barrier.
 *
 * @param data  Shared simulation data.
 */
void		wait_ready(t_data *data);

/**
 * @brief Entry point for the dedicated monitor thread.
 *
 * Waits for the start barrier, then polls all philosophers once per
 * millisecond. Sets t_data::dead and prints the death message if any
 * philosopher exceeds time_to_die. Sets t_data::all_ate when every
 * philosopher has finished must_eat meals.
 *
 * @param arg  Pointer to the shared t_data struct.
 * @return Always NULL.
 */
void		*monitor_routine(void *arg);

/* utils.c */

/**
 * @brief Return the current wall-clock time in milliseconds.
 *
 * @return Milliseconds since the Unix epoch.
 */
long long	get_time(void);

/**
 * @brief Sleep for exactly @p ms milliseconds using a busy-wait loop.
 *
 * Standard @c usleep has poor precision; this function polls
 * get_time() in 500 µs increments to achieve millisecond accuracy.
 *
 * @param ms  Duration to sleep in milliseconds.
 */
void		ft_usleep(long long ms);

/**
 * @brief Print a philosopher state-change message if the simulation
 *        is still running.
 *
 * Acquires print_mutex then meal_mutex (in that order) to read the
 * stop flags and compute the elapsed timestamp atomically, then
 * releases meal_mutex before calling printf and finally releases
 * print_mutex.
 *
 * @param philo  Philosopher whose state changed.
 * @param msg    State string, e.g. "is eating".
 */
void		print_status(t_philo *philo, char *msg);

/**
 * @brief Check whether the simulation has ended.
 *
 * @param data  Shared simulation data.
 * @return 1 if @c dead or @c all_ate is set, 0 otherwise.
 */
int			is_dead(t_data *data);

/**
 * @brief Convert a non-negative decimal string to an int.
 *
 * Reads digits until a non-digit character (or NUL) is encountered.
 * Negative signs and leading whitespace are not supported.
 *
 * @param str  NUL-terminated string of ASCII digits.
 * @return Parsed integer value (truncated to int).
 */
int			ft_atoi(const char *str);

#endif
