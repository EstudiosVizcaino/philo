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

#include "philo.h"

static int	check_death(t_data *data, int i)
{
	long long	elapsed;

	pthread_mutex_lock(&data->meal_mutex);
	elapsed = get_time() - data->philos[i].last_meal_time;
	if (elapsed > data->time_to_die)
	{
		data->dead = 1;
		pthread_mutex_unlock(&data->meal_mutex);
		pthread_mutex_lock(&data->print_mutex);
		printf("%lld %d died\n",
			get_time() - data->start_time, data->philos[i].id);
		pthread_mutex_unlock(&data->print_mutex);
		return (1);
	}
	pthread_mutex_unlock(&data->meal_mutex);
	return (0);
}

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

void	*monitor_routine(void *arg)
{
	t_data	*data;
	int		i;

	data = (t_data *)arg;
	wait_for_ready_monitor(data);
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
		usleep(1000);
	}
	return (NULL);
}
