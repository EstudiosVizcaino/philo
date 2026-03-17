/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   philo_bonus.h                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cvizcain <cvizcain@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/17 10:09:41 by cvizcain          #+#    #+#             */
/*   Updated: 2026/03/17 10:09:41 by cvizcain         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PHILO_BONUS_H
# define PHILO_BONUS_H

/**
 * @file philo_bonus.h
 * @brief Types, includes, and prototypes for the bonus dining
 *        philosophers simulation (process-based, POSIX semaphores).
 *
 * Each philosopher runs as an independent child process. All
 * inter-process coordination uses POSIX named semaphores. A
 * per-philosopher semaphore (@c protect) guards the fields
 * @c last_meal_time and @c meals_eaten against concurrent access
 * from the process-local death-monitor thread.
 */

# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <unistd.h>
# include <sys/time.h>
# include <pthread.h>
# include <semaphore.h>
# include <fcntl.h>
# include <sys/wait.h>
# include <signal.h>

typedef struct s_data_bonus	t_data_bonus;

/**
 * @brief State owned by a single philosopher child process.
 *
 * Each child process receives its own copy of this struct via
 * @c fork(). The fields @c last_meal_time and @c meals_eaten are
 * shared with the process-local death-monitor thread and must
 * always be accessed while holding @c protect.
 */
typedef struct s_philo_bonus
{
	int				id;            /**< 1-based philosopher identifier. */
	int				meals_eaten;   /**< Completed meals. Guarded by protect. */
	long long		last_meal_time;/**< Timestamp (ms) of last meal. Guarded by protect. */
	sem_t			*protect;      /**< Guards last_meal_time and meals_eaten. */
	t_data_bonus	*data;         /**< Pointer to shared simulation data. */
}	t_philo_bonus;

/**
 * @brief Simulation-wide parameters and shared resource handles.
 *
 * Initialised once in the parent process before @c fork(). Because
 * philosophers are separate processes, each child inherits a copy.
 * All coordination relies on the named semaphores stored here.
 */
typedef struct s_data_bonus
{
	int				num_philos; /**< Total number of philosophers. */
	long long		time_to_die; /**< Starvation deadline in ms. */
	long long		time_to_eat; /**< Duration of one meal in ms. */
	long long		time_to_sleep; /**< Duration of one sleep in ms. */
	int				must_eat;   /**< Required meals per philo (-1 = unlimited). */
	long long		start_time; /**< Simulation start timestamp in ms. */
	long long		think_time; /**< Extra think delay for odd-count tables. */
	sem_t			*forks;     /**< Counting semaphore: available forks. */
	sem_t			*print_sem; /**< Mutex semaphore protecting stdout. */
	sem_t			*dead_sem;  /**< Posted once to trigger program shutdown. */
	sem_t			*meal_sem;  /**< Posted by each philo when must_eat reached. */
	pthread_t		meal_thread;/**< Parent-side thread running meal_monitor. */
	pid_t			*pids;      /**< Array of child process IDs (num_philos). */
	t_philo_bonus	*philos;    /**< Array of philosopher state structs. */
}	t_data_bonus;

/* init_bonus.c */

/**
 * @brief Initialise all simulation data from command-line arguments.
 *
 * Parses @p argv into @p data, computes @c think_time, opens all
 * global named semaphores, and allocates the @c pids and @c philos
 * arrays via @c alloc_arrays.  On any failure @c cleanup is called
 * and 0 is returned.
 *
 * @param data  Pointer to the uninitialised data structure.
 * @param argc  Argument count as received by @c main.
 * @param argv  Argument vector as received by @c main.
 * @return      1 on success, 0 on failure.
 */
int			init_data(t_data_bonus *data, int argc, char **argv);

/**
 * @brief Release all semaphore handles, unlink names, and free heap.
 *
 * Closes and unlinks the four global semaphores, then iterates over
 * every philosopher entry to close and unlink its per-philo protect
 * semaphore.  Finally frees @c data->philos and @c data->pids.
 *
 * @param data  Pointer to the data structure to tear down.
 */
void		cleanup(t_data_bonus *data);

/* routine_bonus.c */

/**
 * @brief Child-process entry point: run one philosopher's lifecycle.
 *
 * Spawns a detached death-monitor thread, staggers even-numbered
 * philosophers by @c time_to_eat ms to reduce contention, then
 * loops indefinitely calling @c philo_eat and @c philo_rest.
 *
 * @param philo  Pointer to this philosopher's state struct.
 */
void		run_philo(t_philo_bonus *philo);

/* monitor_bonus.c */

/**
 * @brief Per-process thread: detect philosopher starvation.
 *
 * Polls @c last_meal_time once per millisecond under @c protect.
 * If the elapsed time exceeds @c time_to_die, prints the death
 * message, posts @c dead_sem, and calls @c exit(1).
 *
 * @param arg  Pointer to the philosopher's @c t_philo_bonus struct.
 * @return     Never returns in the starvation path; NULL otherwise.
 */
void		*death_monitor(void *arg);

/**
 * @brief Parent-process thread: detect that all philosophers ate.
 *
 * When @c must_eat is set, waits for @c num_philos posts on
 * @c meal_sem (one per philosopher reaching the meal quota), then
 * posts @c dead_sem to trigger an orderly shutdown.  Returns
 * immediately if @c must_eat is -1.
 *
 * @param arg  Pointer to the shared @c t_data_bonus struct.
 * @return     NULL in all cases.
 */
void		*meal_monitor(void *arg);

/* utils_bonus.c */

/**
 * @brief Return current wall-clock time in milliseconds.
 *
 * @return  Milliseconds since the Unix epoch.
 */
long long	get_time(void);

/**
 * @brief High-precision sleep for @p ms milliseconds.
 *
 * Busy-waits in 500-microsecond increments using @c usleep(500)
 * to reduce OS scheduling jitter compared to a single long sleep.
 *
 * @param ms  Duration to sleep in milliseconds.
 */
void		ft_usleep(long long ms);

/**
 * @brief Print a timestamped philosopher status line to stdout.
 *
 * Acquires @c print_sem before writing to prevent interleaving with
 * output from other processes, then releases it.  The timestamp is
 * relative to @c data->start_time.
 *
 * @param philo  Philosopher whose status is being printed.
 * @param msg    Null-terminated status string to print.
 */
void		print_status(t_philo_bonus *philo, char *msg);

/**
 * @brief Convert a non-negative decimal ASCII string to an @c int.
 *
 * Reads digits until a non-digit is found.  No sign handling and
 * no overflow checking — @c check_args must validate input first.
 *
 * @param str  Null-terminated string of decimal digits.
 * @return     Integer value of the string.
 */
int			ft_atoi(const char *str);

/**
 * @brief Build a named-semaphore path by appending an integer to
 *        a base string.
 *
 * Writes "<base><id>" into @p dst, where @p id is formatted without
 * leading zeros (up to 3 digits).  @p dst must be large enough to
 * hold the result.
 *
 * @param dst   Destination buffer (caller-allocated).
 * @param base  Base string prefix (e.g. "/philo_prot_").
 * @param id    Integer suffix to append.
 */
void		ft_build_name(char *dst, char *base, int id);

/* main_bonus.c */

/**
 * @brief Send SIGKILL to every live philosopher child process.
 *
 * Iterates @c data->pids and calls @c kill(pid, SIGKILL) for each
 * entry where @c pid > 0, guarding against un-forked zero entries.
 *
 * @param data  Pointer to simulation data containing the pid array.
 */
void		kill_all(t_data_bonus *data);

#endif
