/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init_bonus.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cvizcain <cvizcain@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/17 10:09:41 by cvizcain          #+#    #+#             */
/*   Updated: 2026/03/17 10:09:41 by cvizcain         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo_bonus.h"

/**
 * @brief Unlink any stale instance and open a new named semaphore.
 *
 * Calling @c sem_unlink first ensures a clean state even if a
 * previous run crashed without cleaning up.
 *
 * @param name  Null-terminated POSIX semaphore name (e.g. "/foo").
 * @param val   Initial value for the new semaphore.
 * @return      Pointer to the opened semaphore, or NULL on error.
 */
static sem_t	*open_sem(char *name, unsigned int val)
{
	sem_t	*sem;

	sem_unlink(name);
	sem = sem_open(name, O_CREAT, 0644, val);
	if (sem == SEM_FAILED)
		return (NULL);
	return (sem);
}

/**
 * @brief Allocate and initialise the pids and philos arrays.
 *
 * Allocates @c num_philos entries for @c data->pids (zeroed so that
 * any un-forked slot stays 0 and is skipped by @c kill_all) and for
 * @c data->philos.  Then opens one per-philosopher named semaphore
 * named @c /philo_prot_<id> with initial value 1 (binary mutex).
 *
 * @param data  Pointer to partially-initialised simulation data.
 * @return      1 on success, 0 if any allocation or semaphore fails.
 */
static int	alloc_arrays(t_data_bonus *data)
{
	int		i;
	char	name[32];

	data->pids = malloc(sizeof(pid_t) * data->num_philos);
	if (!data->pids)
		return (0);
	memset(data->pids, 0, sizeof(pid_t) * data->num_philos);
	data->philos = malloc(sizeof(t_philo_bonus) * data->num_philos);
	if (!data->philos)
		return (0);
	memset(data->philos, 0, sizeof(t_philo_bonus) * data->num_philos);
	i = 0;
	while (i < data->num_philos)
	{
		data->philos[i].id = i + 1;
		data->philos[i].data = data;
		ft_build_name(name, "/philo_prot_", i + 1);
		data->philos[i].protect = open_sem(name, 1);
		if (!data->philos[i].protect)
			return (0);
		i++;
	}
	return (1);
}

/**
 * @brief Initialise all simulation data from command-line arguments.
 *
 * Parses @p argv, computes @c think_time as half the spare budget
 * between @c time_to_die and one full eat+sleep cycle, opens the
 * four global semaphores (forks, print, dead, meal), then calls
 * @c alloc_arrays.  @c cleanup is called automatically on failure.
 *
 * @param data  Output parameter: simulation data to populate.
 * @param argc  Argument count (4 required + 1 optional).
 * @param argv  Argument vector: n t_die t_eat t_sleep [n_eat].
 * @return      1 on success, 0 on any initialisation error.
 */
int	init_data(t_data_bonus *data, int argc, char **argv)
{
	long long	budget;

	memset(data, 0, sizeof(t_data_bonus));
	data->num_philos = ft_atoi(argv[1]);
	data->time_to_die = ft_atoi(argv[2]);
	data->time_to_eat = ft_atoi(argv[3]);
	data->time_to_sleep = ft_atoi(argv[4]);
	if (argc == 6)
		data->must_eat = ft_atoi(argv[5]);
	else
		data->must_eat = -1;
	budget = data->time_to_die - data->time_to_eat - data->time_to_sleep;
	if (budget > 0)
		data->think_time = budget / 2;
	data->forks = open_sem("/philo_forks", (unsigned int)data->num_philos);
	data->print_sem = open_sem("/philo_print", 1);
	data->dead_sem = open_sem("/philo_dead", 0);
	data->meal_sem = open_sem("/philo_meal", 0);
	if (!data->forks || !data->print_sem || !data->dead_sem
		|| !data->meal_sem || !alloc_arrays(data))
	{
		cleanup(data);
		return (0);
	}
	return (1);
}

/**
 * @brief Close and unlink the per-philosopher protect semaphores.
 *
 * Iterates every slot in @c data->philos, closes the semaphore
 * handle if non-NULL, then unlinks the named semaphore so it is
 * removed from the system.
 *
 * @param data  Pointer to simulation data with philos array.
 */
static void	cleanup_sems(t_data_bonus *data)
{
	int		i;
	char	name[32];

	i = 0;
	while (data->philos && i < data->num_philos)
	{
		if (data->philos[i].protect)
			sem_close(data->philos[i].protect);
		ft_build_name(name, "/philo_prot_", i + 1);
		sem_unlink(name);
		i++;
	}
}

/**
 * @brief Release all resources allocated by @c init_data.
 *
 * Closes the four global semaphore handles, unlinks their names,
 * delegates per-philosopher semaphore teardown to @c cleanup_sems,
 * then frees @c data->philos and @c data->pids.
 *
 * @param data  Pointer to simulation data to tear down.
 */
void	cleanup(t_data_bonus *data)
{
	if (data->forks)
		sem_close(data->forks);
	if (data->print_sem)
		sem_close(data->print_sem);
	if (data->dead_sem)
		sem_close(data->dead_sem);
	if (data->meal_sem)
		sem_close(data->meal_sem);
	sem_unlink("/philo_forks");
	sem_unlink("/philo_print");
	sem_unlink("/philo_dead");
	sem_unlink("/philo_meal");
	cleanup_sems(data);
	free(data->philos);
	free(data->pids);
}
