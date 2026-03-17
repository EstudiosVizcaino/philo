/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cvizcain <cvizcain@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/16 18:22:31 by cvizcain          #+#    #+#             */
/*   Updated: 2026/03/16 22:06:06 by cvizcain         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/**
 * @file init.c
 * @brief Simulation initialisation and resource cleanup.
 *
 * Provides init_data() which parses arguments, computes derived
 * timing values, and allocates every mutex and the philosopher array.
 * cleanup() is the symmetric teardown function.
 */

#include "philo.h"

/**
 * @brief Allocate and initialise one mutex per fork.
 *
 * Allocates a contiguous array of @c num_philos mutexes, initialises
 * them one by one, and on failure destroys the already-initialised
 * ones before freeing the array.
 *
 * @param data  Shared simulation data (num_philos must be set).
 * @return 1 on success, 0 on allocation or mutex-init failure.
 */
static int	init_forks(t_data *data)
{
	int	i;

	data->forks = malloc(sizeof(pthread_mutex_t) * data->num_philos);
	if (!data->forks)
		return (0);
	i = 0;
	while (i < data->num_philos)
	{
		if (pthread_mutex_init(&data->forks[i], NULL))
		{
			while (--i >= 0)
				pthread_mutex_destroy(&data->forks[i]);
			free(data->forks);
			data->forks = NULL;
			return (0);
		}
		i++;
	}
	return (1);
}

/**
 * @brief Allocate and initialise the philosopher array.
 *
 * Each philosopher is assigned a 1-based @c id, zero meal counters,
 * left fork (forks[i]) and right fork (forks[(i+1)%n]), and a
 * back-pointer to @p data.
 *
 * @param data  Shared simulation data (forks must already be set up).
 * @return 1 on success, 0 on allocation failure.
 */
static int	init_philos(t_data *data)
{
	int	i;

	data->philos = malloc(sizeof(t_philo) * data->num_philos);
	if (!data->philos)
		return (0);
	i = 0;
	while (i < data->num_philos)
	{
		data->philos[i].id = i + 1;
		data->philos[i].meals_eaten = 0;
		data->philos[i].last_meal_time = 0;
		data->philos[i].left_fork = &data->forks[i];
		data->philos[i].right_fork = &data->forks[(i + 1) % data->num_philos];
		data->philos[i].data = data;
		i++;
	}
	return (1);
}

/**
 * @brief Initialise the print mutex, the meal mutex, and the forks.
 *
 * Initialises mutexes in the order: print_mutex, meal_mutex, then
 * calls init_forks(). Any failure destroys what was already created.
 *
 * @param data  Shared simulation data.
 * @return 1 on success, 0 on any mutex-init failure.
 */
static int	init_mutexes(t_data *data)
{
	if (pthread_mutex_init(&data->print_mutex, NULL))
		return (0);
	if (pthread_mutex_init(&data->meal_mutex, NULL))
	{
		pthread_mutex_destroy(&data->print_mutex);
		return (0);
	}
	if (!init_forks(data))
	{
		pthread_mutex_destroy(&data->print_mutex);
		pthread_mutex_destroy(&data->meal_mutex);
		return (0);
	}
	return (1);
}

/**
 * @brief Initialise all simulation data from command-line arguments.
 *
 * Parses @p argv, computes derived timing values, and allocates and
 * initialises all mutexes and the philosopher array.
 *
 * @param data  Pointer to the uninitialised t_data structure.
 * @param argc  Argument count (5 or 6).
 * @param argv  Argument vector beginning at @c argv[1].
 * @return 1 on success, 0 on any allocation or mutex error.
 */
int	init_data(t_data *data, int argc, char **argv)
{
	long long	time_budget;

	memset(data, 0, sizeof(t_data));
	data->num_philos = ft_atoi(argv[1]);
	data->time_to_die = ft_atoi(argv[2]);
	data->time_to_eat = ft_atoi(argv[3]);
	data->time_to_sleep = ft_atoi(argv[4]);
	if (argc == 6)
		data->must_eat = ft_atoi(argv[5]);
	else
		data->must_eat = -1;
	time_budget = data->time_to_die - data->time_to_eat - data->time_to_sleep;
	if (time_budget > 0)
		data->think_time = time_budget / 2;
	else
		data->think_time = 0;
	if (!init_mutexes(data))
		return (0);
	if (!init_philos(data))
	{
		cleanup(data);
		return (0);
	}
	return (1);
}

/**
 * @brief Release all resources owned by @p data.
 *
 * Destroys every fork mutex, frees the fork and philosopher arrays,
 * and destroys the print and meal mutexes. Safe to call after a
 * partial initialisation (checks for NULL pointers).
 *
 * @param data  Pointer to the t_data structure to clean up.
 */
void	cleanup(t_data *data)
{
	int	i;

	i = 0;
	if (data->forks)
	{
		while (i < data->num_philos)
		{
			pthread_mutex_destroy(&data->forks[i]);
			i++;
		}
		free(data->forks);
	}
	if (data->philos)
		free(data->philos);
	pthread_mutex_destroy(&data->print_mutex);
	pthread_mutex_destroy(&data->meal_mutex);
}
