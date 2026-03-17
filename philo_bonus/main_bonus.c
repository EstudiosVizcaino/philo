/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main_bonus.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cvizcain <cvizcain@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/17 10:09:41 by cvizcain          #+#    #+#             */
/*   Updated: 2026/03/17 10:09:41 by cvizcain         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo_bonus.h"

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
		write(2, "Usage: ./philo_bonus n t_die t_eat t_sleep [n_eat]\n", 51);
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

static void	start_processes(t_data_bonus *data)
{
	int	i;

	data->start_time = get_time();
	i = 0;
	while (i < data->num_philos)
	{
		data->philos[i].last_meal_time = data->start_time;
		data->pids[i] = fork();
		if (data->pids[i] < 0)
		{
			kill_all(data);
			return ;
		}
		if (data->pids[i] == 0)
		{
			run_philo(&data->philos[i]);
			exit(0);
		}
		i++;
	}
	sem_wait(data->dead_sem);
	kill_all(data);
}

int	main(int argc, char **argv)
{
	t_data_bonus	data;
	int				i;

	if (!check_args(argc, argv))
		return (1);
	if (argc == 6 && ft_atoi(argv[5]) == 0)
		return (0);
	if (!init_data(&data, argc, argv))
		return (1);
	pthread_create(&data.meal_thread, NULL, meal_monitor, &data);
	start_processes(&data);
	i = 0;
	while (i < data.num_philos)
	{
		waitpid(data.pids[i], NULL, 0);
		sem_post(data.meal_sem);
		i++;
	}
	pthread_join(data.meal_thread, NULL);
	cleanup(&data);
	return (0);
}
