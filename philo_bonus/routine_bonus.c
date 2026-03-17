/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   routine_bonus.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cvizcain <cvizcain@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/17 10:09:41 by cvizcain          #+#    #+#             */
/*   Updated: 2026/03/17 10:09:41 by cvizcain         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo_bonus.h"

/**
 * @brief Philosopher eating phase: acquire forks, eat, release forks.
 *
 * Waits on the global @c forks semaphore twice (one fork per wait),
 * then locks @c protect to update @c last_meal_time and increment
 * @c meals_eaten atomically with respect to the death-monitor thread.
 * After sleeping for @c time_to_eat ms the two fork tokens are
 * returned and, if the meal quota is reached, @c meal_sem is posted.
 *
 * @param philo  Pointer to the philosopher executing this phase.
 */
static void	philo_eat(t_philo_bonus *philo)
{
	t_data_bonus	*data;

	data = philo->data;
	sem_wait(data->forks);
	print_status(philo, "has taken a fork");
	sem_wait(data->forks);
	print_status(philo, "has taken a fork");
	sem_wait(philo->protect);
	philo->last_meal_time = get_time();
	philo->meals_eaten++;
	sem_post(philo->protect);
	print_status(philo, "is eating");
	ft_usleep(data->time_to_eat);
	sem_post(data->forks);
	sem_post(data->forks);
	if (data->must_eat != -1 && philo->meals_eaten == data->must_eat)
		sem_post(data->meal_sem);
}

/**
 * @brief Philosopher rest phase: sleep, then think (with stagger).
 *
 * Prints and waits for @c time_to_sleep ms, then prints "is thinking".
 * For odd-count tables an additional @c think_time delay is inserted
 * to spread the philosophers' cycles and prevent starvation.
 *
 * @param philo  Pointer to the philosopher executing this phase.
 */
static void	philo_rest(t_philo_bonus *philo)
{
	t_data_bonus	*data;

	data = philo->data;
	print_status(philo, "is sleeping");
	ft_usleep(data->time_to_sleep);
	print_status(philo, "is thinking");
	if (data->num_philos % 2 != 0)
		ft_usleep(data->think_time);
}

/**
 * @brief Entry point for a philosopher child process.
 *
 * Creates and detaches a death-monitor thread that will call
 * @c exit(1) if this philosopher starves.  Even-numbered
 * philosophers stagger by @c time_to_eat ms to reduce initial
 * fork contention.  Then loops @c philo_eat / @c philo_rest
 * indefinitely; the loop is broken externally by SIGKILL.
 *
 * @param philo  Pointer to this philosopher's state struct.
 */
void	run_philo(t_philo_bonus *philo)
{
	pthread_t	death_thread;

	if (pthread_create(&death_thread, NULL, death_monitor, philo))
		exit(1);
	pthread_detach(death_thread);
	if (philo->id % 2 == 0)
		ft_usleep(philo->data->time_to_eat);
	while (1)
	{
		philo_eat(philo);
		philo_rest(philo);
	}
}
