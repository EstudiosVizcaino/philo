/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   routine.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cvizcain <cvizcain@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/16 18:23:19 by cvizcain          #+#    #+#             */
/*   Updated: 2026/03/16 22:05:56 by cvizcain         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/**
 * @file routine.c
 * @brief Philosopher thread routine: eat, sleep, think.
 *
 * Each philosopher thread runs philo_routine(). The routine waits
 * for the shared start barrier, then loops through eat–sleep–think
 * until the simulation ends. Forks are acquired in an order that
 * prevents deadlock: even-ID philosophers take the right fork first.
 * For odd N, the last philosopher (id==N) is staggered like the even
 * group to prevent the odd-N cascade.
 */

#include "philo.h"

/**
 * @brief Spin until data->ready is set (start barrier).
 *
 * All philosopher threads park here until start_threads() records
 * start_time and sets the ready flag under meal_mutex, ensuring every
 * thread begins with the same timing reference.
 *
 * @param philo  The philosopher waiting for the barrier.
 */
static void	wait_for_ready(t_philo *philo)
{
	while (1)
	{
		pthread_mutex_lock(&philo->data->meal_mutex);
		if (philo->data->ready)
		{
			pthread_mutex_unlock(&philo->data->meal_mutex);
			break ;
		}
		pthread_mutex_unlock(&philo->data->meal_mutex);
		usleep(100);
	}
}

/**
 * @brief Acquire both forks in deadlock-safe order.
 *
 * Even-ID philosophers take the right fork before the left; odd-ID
 * philosophers take the left fork before the right. This asymmetry
 * prevents a circular wait when all philosophers try to eat at once.
 *
 * @param philo  The philosopher that wants to eat.
 */
static void	grab_forks(t_philo *philo)
{
	if (philo->id % 2 == 0)
	{
		pthread_mutex_lock(philo->right_fork);
		print_status(philo, "has taken a fork");
		pthread_mutex_lock(philo->left_fork);
		print_status(philo, "has taken a fork");
	}
	else
	{
		pthread_mutex_lock(philo->left_fork);
		print_status(philo, "has taken a fork");
		pthread_mutex_lock(philo->right_fork);
		print_status(philo, "has taken a fork");
	}
}

/**
 * @brief Perform one full eating cycle.
 *
 * Acquires both forks via grab_forks(), then under meal_mutex records
 * last_meal_time, increments meals_eaten, and increments
 * finished_eating when must_eat is reached. Releases meal_mutex
 * before printing and sleeping, then releases both forks.
 *
 * @param philo  The philosopher that is eating.
 */
static void	philo_eat(t_philo *philo)
{
	t_data	*data;

	data = philo->data;
	grab_forks(philo);
	pthread_mutex_lock(&data->meal_mutex);
	philo->last_meal_time = get_time();
	philo->meals_eaten++;
	if (data->must_eat != -1 && philo->meals_eaten == data->must_eat)
		data->finished_eating++;
	pthread_mutex_unlock(&data->meal_mutex);
	print_status(philo, "is eating");
	ft_usleep(data->time_to_eat);
	pthread_mutex_unlock(philo->left_fork);
	pthread_mutex_unlock(philo->right_fork);
}

/**
 * @brief Perform the sleep and think phase after eating.
 *
 * Sleeps for time_to_sleep milliseconds, prints "is thinking", then
 * for odd philosopher counts adds a think_time delay to keep the
 * cycle strictly shorter than time_to_die.
 *
 * @param philo  The philosopher that just finished eating.
 */
static void	philo_rest(t_philo *philo)
{
	t_data	*data;

	data = philo->data;
	print_status(philo, "is sleeping");
	ft_usleep(data->time_to_sleep);
	print_status(philo, "is thinking");
	if (data->num_philos % 2 != 0)
		ft_usleep(data->think_time);
}

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
 *
 * Overachiever fix: the loop exits as soon as meals_eaten reaches
 * must_eat, releasing both forks immediately so that neighbours that
 * have not yet finished are not blocked unnecessarily.
 *
 * The single-philosopher edge case is handled separately.
 *
 * @param arg  Pointer to the philosopher's t_philo struct.
 * @return Always NULL.
 */
void	*philo_routine(void *arg)
{
	t_philo	*philo;
	t_data	*data;

	philo = (t_philo *)arg;
	data = philo->data;
	wait_for_ready(philo);
	if (data->num_philos == 1)
	{
		pthread_mutex_lock(philo->left_fork);
		print_status(philo, "has taken a fork");
		while (!is_dead(data))
			usleep(1000);
		pthread_mutex_unlock(philo->left_fork);
		return (NULL);
	}
	if (philo->id % 2 == 0)
		ft_usleep(data->time_to_eat);
	while (!is_dead(data))
	{
		philo_eat(philo);
		if (is_dead(data))
			break ;
		pthread_mutex_lock(&data->meal_mutex);
		if (data->must_eat != -1 && philo->meals_eaten >= data->must_eat)
		{
			pthread_mutex_unlock(&data->meal_mutex);
			break ;
		}
		pthread_mutex_unlock(&data->meal_mutex);
		philo_rest(philo);
	}
	return (NULL);
}
