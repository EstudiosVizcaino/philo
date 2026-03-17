/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cvizcain <cvizcain@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/16 18:26:02 by cvizcain          #+#    #+#             */
/*   Updated: 2026/03/17 15:09:57 by cvizcain         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

long long	get_time(void)
{
	struct timeval	tv;

	gettimeofday(&tv, NULL);
	return ((tv.tv_sec * 1000) + (tv.tv_usec / 1000));
}

void	ft_usleep(long long ms)
{
	long long	start;

	start = get_time();
	if (ms > 5)
		usleep((ms - 5) * 1000);
	while ((get_time() - start) < ms)
		usleep(100);
}

void	print_status(t_philo *philo, char *msg)
{
	t_data		*data;
	long long	time;

	data = philo->data;
	pthread_mutex_lock(&data->print_mutex);
	pthread_mutex_lock(&data->meal_mutex);
	if (!data->dead && !data->all_ate)
	{
		time = get_time() - data->start_time;
		printf("%lld %d %s\n", time, philo->id, msg);
	}
	pthread_mutex_unlock(&data->meal_mutex);
	pthread_mutex_unlock(&data->print_mutex);
}

int	is_dead(t_data *data)
{
	int	result;

	pthread_mutex_lock(&data->meal_mutex);
	result = data->dead || data->all_ate;
	pthread_mutex_unlock(&data->meal_mutex);
	return (result);
}

int	ft_atoi(const char *str)
{
	long long	result;
	int			i;

	result = 0;
	i = 0;
	while (str[i] >= '0' && str[i] <= '9')
	{
		result = result * 10 + (str[i] - '0');
		i++;
	}
	return ((int)result);
}
