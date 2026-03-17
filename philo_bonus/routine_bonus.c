/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   routine_bonus.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cvizcain <cvizcain@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/17 10:09:41 by cvizcain          #+#    #+#             */
/*   Updated: 2026/03/17 10:09:41 by cvizcain         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo_bonus.h"

static void	philo_eat(t_philo_bonus *philo)
{
	t_data_bonus	*data;

	data = philo->data;
	sem_wait(data->forks);
	print_status(philo, "has taken a fork");
	sem_wait(data->forks);
	print_status(philo, "has taken a fork");
	sem_wait(philo->protect);
	philo->last_meal_time = get_time();
	philo->meals_eaten++;
	sem_post(philo->protect);
	print_status(philo, "is eating");
	ft_usleep(data->time_to_eat);
	sem_post(data->forks);
	sem_post(data->forks);
	if (data->must_eat != -1 && philo->meals_eaten == data->must_eat)
		sem_post(data->meal_sem);
}

static void	philo_rest(t_philo_bonus *philo)
{
	t_data_bonus	*data;

	data = philo->data;
	print_status(philo, "is sleeping");
	ft_usleep(data->time_to_sleep);
	print_status(philo, "is thinking");
	if (data->num_philos % 2 != 0)
		ft_usleep(data->think_time);
}

void	run_philo(t_philo_bonus *philo)
{
	pthread_t	death_thread;

	if (pthread_create(&death_thread, NULL, death_monitor, philo))
		exit(1);
	pthread_detach(death_thread);
	if (philo->id % 2 == 0)
		ft_usleep(philo->data->time_to_eat);
	while (1)
	{
		philo_eat(philo);
		philo_rest(philo);
	}
}
