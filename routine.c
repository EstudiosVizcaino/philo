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

#include "philo.h"

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

static void	philo_eat(t_philo *philo)
{
	t_data		*data;
	long long	elapsed;

	data = philo->data;
	grab_forks(philo);
	pthread_mutex_lock(&data->meal_mutex);
	philo->last_meal_time = get_time();
	philo->meals_eaten++;
	pthread_mutex_unlock(&data->meal_mutex);
	print_status(philo, "has taken a fork");
	print_status(philo, "has taken a fork");
	print_status(philo, "is eating");
	elapsed = get_time() - philo->last_meal_time;
	if (elapsed < data->time_to_eat)
		ft_usleep(data->time_to_eat - elapsed);
	pthread_mutex_unlock(philo->left_fork);
	pthread_mutex_unlock(philo->right_fork);
}

static void	philo_rest(t_philo *philo)
{
	t_data		*data;
	long long	start;
	long long	think;

	data = philo->data;
	start = get_time();
	print_status(philo, "is sleeping");
	if (get_time() - start < data->time_to_sleep)
		ft_usleep(data->time_to_sleep - (get_time() - start));
	print_status(philo, "is thinking");
	if (data->num_philos % 2 != 0)
	{
		think = (data->time_to_die - data->time_to_eat
				- data->time_to_sleep) / 2;
		if (think > 0)
			ft_usleep(think);
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
