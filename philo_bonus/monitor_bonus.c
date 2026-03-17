/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor_bonus.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cvizcain <cvizcain@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/17 10:09:41 by cvizcain          #+#    #+#             */
/*   Updated: 2026/03/17 10:09:41 by cvizcain         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo_bonus.h"

void	*death_monitor(void *arg)
{
	t_philo_bonus	*philo;
	t_data_bonus	*data;
	long long		elapsed;

	philo = (t_philo_bonus *)arg;
	data = philo->data;
	while (1)
	{
		usleep(1000);
		sem_wait(philo->protect);
		elapsed = get_time() - philo->last_meal_time;
		sem_post(philo->protect);
		if (elapsed > data->time_to_die)
		{
			sem_wait(data->print_sem);
			printf("%lld %d died\n",
				get_time() - data->start_time, philo->id);
			sem_post(data->dead_sem);
			exit(1);
		}
	}
	return (NULL);
}

void	*meal_monitor(void *arg)
{
	t_data_bonus	*data;
	int				i;

	data = (t_data_bonus *)arg;
	if (data->must_eat == -1)
		return (NULL);
	i = 0;
	while (i < data->num_philos)
	{
		sem_wait(data->meal_sem);
		i++;
	}
	sem_post(data->dead_sem);
	return (NULL);
}
