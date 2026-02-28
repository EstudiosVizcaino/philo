/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: philo <philo@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/28 00:00:00 by philo             #+#    #+#             */
/*   Updated: 2026/02/28 00:00:00 by philo            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

static int	is_valid_arg(char *str)
{
	int	i;

	i = 0;
	if (!str[i])
		return (0);
	while (str[i])
	{
		if (str[i] < '0' || str[i] > '9')
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
		if (!is_valid_arg(argv[i]) || ft_atoi(argv[i]) <= 0)
		{
			write(2, "Error: invalid argument\n", 24);
			return (0);
		}
		i++;
	}
	return (1);
}

static int	start_threads(t_data *data)
{
	pthread_t	monitor;
	int			i;

	i = 0;
	data->start_time = get_time();
	while (i < data->num_philos)
	{
		data->philos[i].last_meal_time = data->start_time;
		if (pthread_create(&data->philos[i].thread, NULL,
				philo_routine, &data->philos[i]))
			return (0);
		i++;
	}
	if (pthread_create(&monitor, NULL, monitor_routine, data))
		return (0);
	pthread_detach(monitor);
	return (1);
}

static void	join_threads(t_data *data)
{
	int	i;

	i = 0;
	while (i < data->num_philos)
	{
		pthread_join(data->philos[i].thread, NULL);
		i++;
	}
}

int	main(int argc, char **argv)
{
	t_data	data;

	if (!check_args(argc, argv))
		return (1);
	if (!init_data(&data, argc, argv))
		return (1);
	if (!start_threads(&data))
	{
		cleanup(&data);
		return (1);
	}
	join_threads(&data);
	cleanup(&data);
	return (0);
}
