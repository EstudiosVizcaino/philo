/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cvizcain <cvizcain@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/16 18:26:02 by cvizcain          #+#    #+#             */
/*   Updated: 2026/03/17 15:09:57 by cvizcain         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/**
 * @file utils.c
 * @brief Low-level utility functions shared across the simulation.
 *
 * Provides the timing primitives (get_time, ft_usleep), the thread-safe
 * status printer (print_status), the simulation-end predicate (is_dead),
 * and the string-to-integer converter (ft_atoi). These helpers are used
 * by all other source files and must not acquire mutexes in an order that
 * conflicts with the global lock order (print_mutex before meal_mutex).
 */

#include "philo.h"

/**
 * @brief Return the current wall-clock time in milliseconds.
 *
 * Wraps gettimeofday() and converts the result to a single long long
 * value representing milliseconds since the Unix epoch.
 *
 * @return Milliseconds since the Unix epoch.
 */
long long	get_time(void)
{
	struct timeval	tv;

	gettimeofday(&tv, NULL);
	return ((tv.tv_sec * 1000) + (tv.tv_usec / 1000));
}

/**
 * @brief Sleep for at least @p ms milliseconds using a busy-wait loop.
 *
 * Calls usleep() for (ms - 5) ms to do most of the waiting in the
 * kernel, then busy-polls get_time() in 100 µs increments to achieve
 * millisecond-level precision. Negative or zero values return immediately.
 *
 * @param ms  Duration to sleep in milliseconds.
 */
void	ft_usleep(long long ms)
{
	long long	start;

	start = get_time();
	if (ms > 5)
		usleep((ms - 5) * 1000);
	while ((get_time() - start) < ms)
		usleep(100);
}

/**
 * @brief Print a philosopher state-change message if the simulation
 *        is still running.
 *
 * Acquires print_mutex then meal_mutex (in that order) to read the
 * stop flags and compute the elapsed timestamp atomically, then
 * releases meal_mutex before calling printf and finally releases
 * print_mutex. Prints nothing when dead or all_ate is already set.
 *
 * @param philo  Philosopher whose state changed.
 * @param msg    State string, e.g. "is eating".
 */
void	print_status(t_philo *philo, char *msg)
{
	t_data		*data;
	long long	time;

	data = philo->data;
	pthread_mutex_lock(&data->print_mutex);
	pthread_mutex_lock(&data->meal_mutex);
	if (!data->dead && !data->all_ate)
	{
		time = get_time() - data->start_time;
		printf("%lld %d %s\n", time, philo->id, msg);
	}
	pthread_mutex_unlock(&data->meal_mutex);
	pthread_mutex_unlock(&data->print_mutex);
}

/**
 * @brief Check whether the simulation has ended.
 *
 * Acquires meal_mutex to safely read t_data::dead and t_data::all_ate,
 * then returns their logical OR. Used by philosopher threads to decide
 * whether to continue the eat-sleep-think loop.
 *
 * @param data  Shared simulation data.
 * @return 1 if @c dead or @c all_ate is set, 0 otherwise.
 */
int	is_dead(t_data *data)
{
	int	result;

	pthread_mutex_lock(&data->meal_mutex);
	result = data->dead || data->all_ate;
	pthread_mutex_unlock(&data->meal_mutex);
	return (result);
}

/**
 * @brief Convert a non-negative decimal string to an int.
 *
 * Reads ASCII digit characters until a non-digit or NUL is found.
 * Leading whitespace and sign characters are not handled. The result
 * is truncated to int; the caller is responsible for range validation.
 *
 * @param str  NUL-terminated string of ASCII digits.
 * @return Parsed integer value (truncated to int).
 */
int	ft_atoi(const char *str)
{
	long long	result;
	int			i;

	result = 0;
	i = 0;
	while (str[i] >= '0' && str[i] <= '9')
	{
		result = result * 10 + (str[i] - '0');
		i++;
	}
	return ((int)result);
}
