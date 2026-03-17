/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   routine.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cvizcain <cvizcain@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/16 18:23:19 by cvizcain          #+#    #+#             */
/*   Updated: 2026/03/17 16:25:21 by cvizcain         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/**
 * @file routine.c
 * @brief Philosopher thread routine: eat, sleep, think.
 *
 * Each philosopher thread runs philo_routine(). The routine waits
 * for the shared start barrier, then loops through eat-sleep-think
 * until the simulation ends. Forks are acquired in an order that
 * prevents deadlock: even-ID philosophers take the right fork first.
 */

#include "philo.h"

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
 * Acquires both forks, records last_meal_time and increments
 * meals_eaten under meal_mutex, then eats and releases forks.
 * Returns 1 if the simulation is over (dead or must_eat reached)
 * so the caller can exit the loop without a separate check.
 *
 * @param philo  The philosopher that is eating.
 * @return 1 if the loop should stop, 0 otherwise.
 */
static int	philo_eat(t_philo *philo)
{
	t_data	*data;
	int		done;

	data = philo->data;
	grab_forks(philo);
	pthread_mutex_lock(&data->meal_mutex);
	philo->last_meal_time = get_time();
	philo->meals_eaten++;
	done = (data->must_eat != -1 && philo->meals_eaten >= data->must_eat);
	if (done)
		data->finished_eating++;
	pthread_mutex_unlock(&data->meal_mutex);
	print_status(philo, "is eating");
	ft_usleep(data->time_to_eat);
	pthread_mutex_unlock(philo->left_fork);
	pthread_mutex_unlock(philo->right_fork);
	return (is_dead(data) || done);
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
 * @brief Handle the single-philosopher edge case.
 *
 * With only one philosopher and one fork, eating is impossible.
 * The philosopher picks up the single fork and waits until the
 * monitor detects the inevitable death.
 *
 * @param philo  The sole philosopher.
 */
static void	philo_solo(t_philo *philo)
{
	pthread_mutex_lock(philo->left_fork);
	print_status(philo, "has taken a fork");
	while (!is_dead(philo->data))
		usleep(1000);
	pthread_mutex_unlock(philo->left_fork);
}

/**
 * @brief Entry point for each philosopher thread.
 *
 * Spins until the start barrier is released, then enters the
 * eat-sleep-think loop. Even-ID philosophers are staggered by
 * time_to_eat milliseconds at startup so the even group begins
 * grabbing forks when the odd group has already released them.
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
	wait_ready(data);
	if (data->num_philos == 1)
	{
		philo_solo(philo);
		return (NULL);
	}
	if (philo->id % 2 == 0)
		ft_usleep(data->time_to_eat);
	while (!is_dead(data))
	{
		if (philo_eat(philo))
			break ;
		philo_rest(philo);
	}
	return (NULL);
}
