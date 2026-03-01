/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: philo <philo@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/28 00:00:00 by philo             #+#    #+#             */
/*   Updated: 2026/02/28 00:00:00 by philo            ###   ########.fr       */
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
	int	i;
	int	count;

	if (data->must_eat == -1)
		return (0);
	count = 0;
	i = 0;
	pthread_mutex_lock(&data->meal_mutex);
	while (i < data->num_philos)
	{
		if (data->philos[i].meals_eaten >= data->must_eat)
			count++;
		i++;
	}
	if (count == data->num_philos)
		data->all_ate = 1;
	pthread_mutex_unlock(&data->meal_mutex);
	if (data->all_ate)
		return (1);
	return (0);
}

void	*monitor_routine(void *arg)
{
	t_data	*data;
	int		i;

	data = (t_data *)arg;
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
