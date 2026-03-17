/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cvizcain <cvizcain@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/16 18:22:36 by cvizcain          #+#    #+#             */
/*   Updated: 2026/03/16 22:05:52 by cvizcain         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/**
 * @file monitor.c
 * @brief Dedicated monitor thread: death detection and meal-count check.
 *
 * The monitor thread polls multiple times per millisecond. check_death() locks
 * meal_mutex to read last_meal_time and sets the dead flag, then
 * prints the death message outside meal_mutex (print_mutex only).
 * check_all_ate() sets all_ate once finished_eating reaches num_philos.
 * wait_ready() is the shared start barrier used by both monitor and
 * philosopher threads.
 */

#include "philo.h"

/**
 * @brief Check whether philosopher @p i has exceeded time_to_die.
 *
 * Acquires meal_mutex to read last_meal_time and time_to_die. If the
 * elapsed time is greater than or equal to time_to_die, sets data->dead,
 * releases meal_mutex, acquires print_mutex, and prints the death
 * message, then releases print_mutex.
 *
 * @param data  Shared simulation data.
 * @param i     Zero-based index of the philosopher to check.
 * @return 1 if the philosopher died, 0 otherwise.
 */
static int	check_death(t_data *data, int i)
{
	long long	elapsed;
	long long	timestamp;

	pthread_mutex_lock(&data->meal_mutex);
	elapsed = get_time() - data->philos[i].last_meal_time;
	if (elapsed >= data->time_to_die)
	{
		data->dead = 1;
		timestamp = get_time() - data->start_time;
		pthread_mutex_unlock(&data->meal_mutex);
		pthread_mutex_lock(&data->print_mutex);
		printf("%lld %d died\n", timestamp, data->philos[i].id);
		pthread_mutex_unlock(&data->print_mutex);
		return (1);
	}
	pthread_mutex_unlock(&data->meal_mutex);
	return (0);
}

/**
 * @brief Check whether every philosopher has completed must_eat meals.
 *
 * Returns 0 immediately if must_eat is -1 (no limit). Otherwise
 * acquires meal_mutex and checks finished_eating. If all N
 * philosophers are done, sets all_ate and returns 1.
 *
 * @param data  Shared simulation data.
 * @return 1 if all philosophers have eaten enough, 0 otherwise.
 */
static int	check_all_ate(t_data *data)
{
	int	finished;

	if (data->must_eat == -1)
		return (0);
	finished = 0;
	pthread_mutex_lock(&data->meal_mutex);
	if (data->finished_eating == data->num_philos)
	{
		data->all_ate = 1;
		finished = 1;
	}
	pthread_mutex_unlock(&data->meal_mutex);
	return (finished);
}

/**
 * @brief Spin until the start barrier (data->ready) is set.
 *
 * All threads park here until start_threads() records start_time
 * and sets the ready flag under meal_mutex, ensuring every thread
 * begins with the same timing reference.
 *
 * @param data  Shared simulation data.
 */
void	wait_ready(t_data *data)
{
	while (1)
	{
		pthread_mutex_lock(&data->meal_mutex);
		if (data->ready)
		{
			pthread_mutex_unlock(&data->meal_mutex);
			break ;
		}
		pthread_mutex_unlock(&data->meal_mutex);
		usleep(100);
	}
}

void	*monitor_routine(void *arg)
{
	t_data	*data;
	int		i;

	data = (t_data *)arg;
	wait_ready(data);
	while (1)
	{
		i = 0;
		while (i < data->num_philos)
		{
			if (check_death(data, i))
				return (NULL);
			i++;
		}
		if (check_all_ate(data))
			return (NULL);
		usleep(100);
	}
	return (NULL);
}
