/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cvizcain <cvizcain@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/16 18:22:29 by cvizcain          #+#    #+#             */
/*   Updated: 2026/03/16 22:05:48 by cvizcain         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

static int	is_valid_arg(char *str)
{
	int			i;
	long long	val;

	i = 0;
	if (!str[i])
		return (0);
	while (str[i])
	{
		if (str[i] < '0' || str[i] > '9')
			return (0);
		i++;
	}
	val = 0;
	i = 0;
	while (str[i])
	{
		val = val * 10 + (str[i] - '0');
		if (val > 2147483647)
			return (0);
		i++;
	}
	return (1);
}

static int	check_args(int argc, char **argv)
{
	int	i;

	if (argc < 5 || argc > 6)
	{
		write(2, "Usage: ./philo n t_die t_eat t_sleep [n_eat]\n", 45);
		return (0);
	}
	i = 1;
	while (i < argc)
	{
		if (!is_valid_arg(argv[i]) || (i < 5 && ft_atoi(argv[i]) <= 0))
		{
			write(2, "Error: invalid argument\n", 24);
			return (0);
		}
		i++;
	}
	return (1);
}

static int	abort_threads(t_data *data, int count)
{
	pthread_mutex_lock(&data->meal_mutex);
	data->dead = 1;
	data->ready = 1;
	pthread_mutex_unlock(&data->meal_mutex);
	while (--count >= 0)
		pthread_join(data->philos[count].thread, NULL);
	return (0);
}

static int	start_threads(t_data *data)
{
	int	i;

	i = -1;
	while (++i < data->num_philos)
	{
		if (pthread_create(&data->philos[i].thread, NULL,
				philo_routine, &data->philos[i]))
			return (abort_threads(data, i));
	}
	if (pthread_create(&data->monitor, NULL, monitor_routine, data))
		return (abort_threads(data, data->num_philos));
	pthread_mutex_lock(&data->meal_mutex);
	data->start_time = get_time();
	i = -1;
	while (++i < data->num_philos)
		data->philos[i].last_meal_time = data->start_time;
	data->ready = 1;
	pthread_mutex_unlock(&data->meal_mutex);
	return (1);
}

int	main(int argc, char **argv)
{
	t_data	data;
	int		i;

	if (!check_args(argc, argv))
		return (1);
	if (argc == 6 && ft_atoi(argv[5]) == 0)
		return (0);
	if (!init_data(&data, argc, argv))
		return (1);
	if (!start_threads(&data))
	{
		cleanup(&data);
		return (1);
	}
	i = -1;
	while (++i < data.num_philos)
		pthread_join(data.philos[i].thread, NULL);
	pthread_join(data.monitor, NULL);
	cleanup(&data);
	return (0);
}
