/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cvizcain <cvizcain@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/16 18:22:36 by cvizcain          #+#    #+#             */
/*   Updated: 2026/03/17 15:09:26 by cvizcain         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

static void	wait_for_ready_monitor(t_data *data)
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

static int	check_death(t_data *data, int i, long long time)
{
	if (time - data->philos[i].last_meal_time > data->time_to_die)
	{
		data->dead = 1;
		pthread_mutex_unlock(&data->meal_mutex);
		pthread_mutex_lock(&data->print_mutex);
		printf("%lld %d died\n", time - data->start_time,
			data->philos[i].id);
		pthread_mutex_unlock(&data->print_mutex);
		return (1);
	}
	return (0);
}

static int	check_philos(t_data *data)
{
	int			i;
	int			finished;
	long long	time;

	i = 0;
	finished = 0;
	pthread_mutex_lock(&data->meal_mutex);
	time = get_time();
	while (i < data->num_philos)
	{
		if (data->must_eat != -1
			&& data->philos[i].meals_eaten >= data->must_eat)
			finished++;
		else if (check_death(data, i, time))
			return (1);
		i++;
	}
	if (data->must_eat != -1 && finished == data->num_philos)
	{
		data->all_ate = 1;
		pthread_mutex_unlock(&data->meal_mutex);
		return (1);
	}
	pthread_mutex_unlock(&data->meal_mutex);
	return (0);
}

void	*monitor_routine(void *arg)
{
	t_data	*data;

	data = (t_data *)arg;
	wait_for_ready_monitor(data);
	while (1)
	{
		if (check_philos(data))
			break ;
		usleep(1000);
	}
	return (NULL);
}
