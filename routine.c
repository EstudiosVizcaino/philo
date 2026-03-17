/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   routine.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cvizcain <cvizcain@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/16 18:23:19 by cvizcain          #+#    #+#             */
/*   Updated: 2026/03/17 15:09:41 by cvizcain         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/**
 * @file routine.c
 * @brief Philosopher thread routine and helper functions.
 *
 * Implements the eat-sleep-think cycle run by every philosopher thread.
 * wait_for_ready() spins on the start barrier before the cycle begins.
 * Fork acquisition uses address ordering to prevent circular-wait
 * deadlocks. The single-philosopher edge case is handled separately
 * inside philo_routine().
 */

#include "philo.h"

/**
 * @brief Spin-wait until the simulation start barrier is released.
 *
 * Polls t_data::ready under meal_mutex in 100 µs increments.
 * Returns as soon as the main thread sets ready = 1.
 *
 * @param philo  The philosopher whose start barrier is being waited on.
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
 * @brief Lock both forks in address order to prevent deadlock.
 *
 * Acquiring mutexes in a consistent global order (lower address first)
 * breaks the circular-wait condition and prevents any two adjacent
 * philosophers from deadlocking on their shared fork.
 *
 * @param philo  The philosopher whose left and right forks must be locked.
 */
static void	grab_forks(t_philo *philo)
{
	if (philo->left_fork < philo->right_fork)
	{
		pthread_mutex_lock(philo->left_fork);
		pthread_mutex_lock(philo->right_fork);
	}
	else
	{
		pthread_mutex_lock(philo->right_fork);
		pthread_mutex_lock(philo->left_fork);
	}
}

/**
 * @brief Execute one eating cycle for @p philo.
 *
 * Grabs both forks (in address order), updates last_meal_time and
 * meals_eaten under meal_mutex, prints the fork and eating messages,
 * sleeps for the remaining time_to_eat, then releases both forks.
 *
 * @param philo  The philosopher who is about to eat.
 */
static void	philo_eat(t_philo *philo)
{
	t_data		*data;
	long long	meal_start;

	data = philo->data;
	grab_forks(philo);
	pthread_mutex_lock(&data->meal_mutex);
	meal_start = get_time();
	philo->last_meal_time = meal_start;
	philo->meals_eaten++;
	pthread_mutex_unlock(&data->meal_mutex);
	print_status(philo, "has taken a fork");
	print_status(philo, "has taken a fork");
	print_status(philo, "is eating");
	if (data->time_to_eat - (get_time() - meal_start) > 0)
		ft_usleep(data->time_to_eat - (get_time() - meal_start));
	pthread_mutex_unlock(philo->left_fork);
	pthread_mutex_unlock(philo->right_fork);
}

/**
 * @brief Execute one sleep-think cycle for @p philo.
 *
 * Sleeps for time_to_sleep milliseconds, then enters the thinking
 * state. For odd philosopher counts, an additional think_time delay
 * staggers the next fork-grab attempt so that the cycle fits inside
 * time_to_die without starvation. Even counts require only a short
 * yield because their symmetric pairing already avoids contention.
 *
 * @param philo  The philosopher who has just finished eating.
 */
static void	philo_rest(t_philo *philo)
{
	t_data		*data;
	long long	start;

	data = philo->data;
	start = get_time();
	print_status(philo, "is sleeping");
	if (get_time() - start < data->time_to_sleep)
		ft_usleep(data->time_to_sleep - (get_time() - start));
	print_status(philo, "is thinking");
	if (data->num_philos % 2 != 0)
	{
		if (data->think_time > 0)
			ft_usleep(data->think_time);
	}
	else
		usleep(500);
}

void	*philo_routine(void *arg)
{
	t_philo	*philo;

	philo = (t_philo *)arg;
	wait_for_ready(philo);
	if (philo->data->num_philos == 1)
	{
		pthread_mutex_lock(philo->left_fork);
		print_status(philo, "has taken a fork");
		while (!is_dead(philo->data))
			usleep(1000);
		pthread_mutex_unlock(philo->left_fork);
		return (NULL);
	}
	if (philo->id % 2 == 0)
		ft_usleep(philo->data->time_to_eat);
	while (!is_dead(philo->data))
	{
		philo_eat(philo);
		if (philo->data->must_eat != -1
			&& philo->meals_eaten >= philo->data->must_eat)
			break ;
		philo_rest(philo);
	}
	return (NULL);
}
