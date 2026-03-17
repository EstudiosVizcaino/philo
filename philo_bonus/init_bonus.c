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

static void	ft_build_name(char *dst, char *base, int id)
{
	int		i;
	int		j;
	char	tmp[4];

	i = 0;
	j = 0;
	while (base[i])
		dst[j++] = base[i++];
	i = 0;
	if (id >= 100)
		tmp[i++] = '0' + (id / 100);
	if (id >= 10)
		tmp[i++] = '0' + ((id / 10) % 10);
	tmp[i++] = '0' + (id % 10);
	tmp[i] = '\0';
	i = 0;
	while (tmp[i])
		dst[j++] = tmp[i++];
	dst[j] = '\0';
}

static sem_t	*open_sem(char *name, unsigned int val)
{
	sem_t	*sem;

	sem_unlink(name);
	sem = sem_open(name, O_CREAT, 0644, val);
	if (sem == SEM_FAILED)
		return (NULL);
	return (sem);
}

static int	alloc_arrays(t_data_bonus *data)
{
	int		i;
	char	name[32];

	data->pids = malloc(sizeof(pid_t) * data->num_philos);
	if (!data->pids)
		return (0);
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

void	cleanup(t_data_bonus *data)
{
	int		i = 0;
	char	name[32];

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
	while (data->philos && i < data->num_philos)
	{
		if (data->philos[i].protect)
			sem_close(data->philos[i].protect);
		ft_build_name(name, "/philo_prot_", i + 1);
		sem_unlink(name);
		i++;
	}
	free(data->philos);
	free(data->pids);
}
