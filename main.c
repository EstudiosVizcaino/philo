/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cvizcain <cvizcain@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/16 18:22:29 by cvizcain          #+#    #+#             */
/*   Updated: 2026/03/16 22:05:48 by cvizcain         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/**
 * @file main.c
 * @brief Entry point, argument validation, and thread lifecycle.
 *
 * Validates the command-line arguments, delegates initialisation to
 * init_data(), creates all philosopher threads and the monitor thread
 * via start_threads(), joins every thread, and calls cleanup().
 */

#include "philo.h"

/**
 * @brief Check whether @p str represents a valid non-negative integer.
 *
 * The string must consist entirely of ASCII digits and its numeric
 * value must fit in a signed 32-bit integer (≤ 2 147 483 647).
 * Empty strings are rejected.
 *
 * @param str  NUL-terminated string to validate.
 * @return 1 if valid, 0 otherwise.
 */
static int	is_valid_arg(char *str)
{
	int			i;
	long long	val;

	i = 0;
	if (!str[i])
		return (0);
	while (str[i])
	{
		if (str[i] < '0' || str[i] > '9')
			return (0);
		i++;
	}
	val = 0;
	i = 0;
	while (str[i])
	{
		val = val * 10 + (str[i] - '0');
		if (val > 2147483647)
			return (0);
		i++;
	}
	return (1);
}

/**
 * @brief Validate all command-line arguments.
 *
 * Expects 4 or 5 positional arguments (plus the program name).
 * Each argument must pass is_valid_arg() and the first four must be
 * strictly positive. Errors are written to stderr.
 *
 * @param argc  Argument count from main().
 * @param argv  Argument vector from main().
 * @return 1 if all arguments are valid, 0 otherwise.
 */
static int	check_args(int argc, char **argv)
{
	int	i;

	if (argc < 5 || argc > 6)
	{
		write(2, "Usage: ./philo n t_die t_eat t_sleep [n_eat]\n", 45);
		return (0);
	}
	i = 1;
	while (i < argc)
	{
		if (!is_valid_arg(argv[i]) || (i < 5 && ft_atoi(argv[i]) <= 0))
		{
			write(2, "Error: invalid argument\n", 24);
			return (0);
		}
		i++;
	}
	return (1);
}

/**
 * @brief Cancel threads that were already created after a failure.
 *
 * Sets the dead flag and the ready barrier under meal_mutex so that
 * any already-running threads wake up and exit cleanly, then joins
 * the first @p count philosopher threads.
 *
 * @param data   Shared simulation data.
 * @param count  Number of philosopher threads that were created.
 * @return Always 0 (propagated as a start_threads failure code).
 */
static int	abort_threads(t_data *data, int count)
{
	pthread_mutex_lock(&data->meal_mutex);
	data->dead = 1;
	data->ready = 1;
	pthread_mutex_unlock(&data->meal_mutex);
	while (--count >= 0)
		pthread_join(data->philos[count].thread, NULL);
	return (0);
}

/**
 * @brief Create philosopher threads, the monitor thread, and release
 *        the start barrier.
 *
 * All philosopher threads are created first so they can spin on the
 * ready flag. Once all threads exist, start_time is recorded,
 * last_meal_time is initialised for every philosopher, and the ready
 * flag is set — all under meal_mutex — to give every thread an
 * identical reference point.
 *
 * @param data  Shared simulation data.
 * @return 1 on success, 0 if any pthread_create call fails (threads
 *         created before the failure are joined via abort_threads).
 */
static int	start_threads(t_data *data)
{
	int	i;

	i = -1;
	while (++i < data->num_philos)
	{
		if (pthread_create(&data->philos[i].thread, NULL,
				philo_routine, &data->philos[i]))
			return (abort_threads(data, i));
	}
	if (pthread_create(&data->monitor, NULL, monitor_routine, data))
		return (abort_threads(data, data->num_philos));
	pthread_mutex_lock(&data->meal_mutex);
	data->start_time = get_time();
	i = -1;
	while (++i < data->num_philos)
		data->philos[i].last_meal_time = data->start_time;
	data->ready = 1;
	pthread_mutex_unlock(&data->meal_mutex);
	return (1);
}

/**
 * @brief Program entry point.
 *
 * Validates arguments, returns early when n_meals is 0, initialises
 * the simulation, starts all threads, waits for them to finish, and
 * releases all resources.
 *
 * @param argc  Argument count (must be 5 or 6).
 * @param argv  Argument vector.
 * @return 0 on success, 1 on argument or initialisation error.
 */
int	main(int argc, char **argv)
{
	t_data	data;
	int		i;

	if (!check_args(argc, argv))
		return (1);
	if (argc == 6 && ft_atoi(argv[5]) == 0)
		return (0);
	if (!init_data(&data, argc, argv))
		return (1);
	if (!start_threads(&data))
	{
		cleanup(&data);
		return (1);
	}
	i = -1;
	while (++i < data.num_philos)
		pthread_join(data.philos[i].thread, NULL);
	pthread_join(data.monitor, NULL);
	cleanup(&data);
	return (0);
}
