/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor_bonus.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cvizcain <cvizcain@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/17 10:09:41 by cvizcain          #+#    #+#             */
/*   Updated: 2026/03/17 10:09:41 by cvizcain         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo_bonus.h"

/**
 * @brief Thread function: monitor one philosopher for starvation.
 *
 * Runs inside each child process.  Sleeps 1 ms between checks, then
 * reads @c last_meal_time under @c protect.  If the elapsed time
 * exceeds @c time_to_die, acquires @c print_sem (held forever so no
 * other process prints after the death line), posts @c dead_sem to
 * wake the parent, and calls @c exit(1).
 *
 * @param arg  Pointer to the philosopher's @c t_philo_bonus struct.
 * @return     Never returns in the starvation path; NULL otherwise.
 */
void	*death_monitor(void *arg)
{
	t_philo_bonus	*philo;
	t_data_bonus	*data;
	long long		elapsed;

	philo = (t_philo_bonus *)arg;
	data = philo->data;
	while (1)
	{
		usleep(1000);
		sem_wait(philo->protect);
		elapsed = get_time() - philo->last_meal_time;
		sem_post(philo->protect);
		if (elapsed > data->time_to_die)
		{
			sem_wait(data->print_sem);
			printf("%lld %d died\n",
				get_time() - data->start_time, philo->id);
			sem_post(data->dead_sem);
			exit(1);
		}
	}
	return (NULL);
}

/**
 * @brief Thread function: detect that all philosophers met their quota.
 *
 * Runs in the parent process.  If @c must_eat is -1 the thread
 * returns immediately.  Otherwise it waits for exactly @c num_philos
 * posts on @c meal_sem (each child posts once when @c meals_eaten
 * equals @c must_eat), then posts @c dead_sem to trigger shutdown.
 *
 * @param arg  Pointer to the shared @c t_data_bonus struct.
 * @return     NULL in all cases.
 */
void	*meal_monitor(void *arg)
{
	t_data_bonus	*data;
	int				i;

	data = (t_data_bonus *)arg;
	if (data->must_eat == -1)
		return (NULL);
	i = 0;
	while (i < data->num_philos)
	{
		sem_wait(data->meal_sem);
		i++;
	}
	sem_post(data->dead_sem);
	return (NULL);
}
