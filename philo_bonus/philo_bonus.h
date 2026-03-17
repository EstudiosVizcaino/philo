/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   philo_bonus.h                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cvizcain <cvizcain@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/17 10:09:41 by cvizcain          #+#    #+#             */
/*   Updated: 2026/03/17 10:09:41 by cvizcain         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PHILO_BONUS_H
# define PHILO_BONUS_H

# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <unistd.h>
# include <sys/time.h>
# include <pthread.h>
# include <semaphore.h>
# include <fcntl.h>
# include <sys/wait.h>
# include <signal.h>

typedef struct s_data_bonus	t_data_bonus;

typedef struct s_philo_bonus
{
	int				id;
	int				meals_eaten;
	long long		last_meal_time;
	sem_t			*protect;
	t_data_bonus	*data;
}	t_philo_bonus;

typedef struct s_data_bonus
{
	int				num_philos;
	long long		time_to_die;
	long long		time_to_eat;
	long long		time_to_sleep;
	int				must_eat;
	long long		start_time;
	long long		think_time;
	sem_t			*forks;
	sem_t			*print_sem;
	sem_t			*dead_sem;
	sem_t			*meal_sem;
	pthread_t		meal_thread;
	pid_t			*pids;
	t_philo_bonus	*philos;
}	t_data_bonus;

/* init_bonus.c */
int			init_data(t_data_bonus *data, int argc, char **argv);
void		cleanup(t_data_bonus *data);

/* routine_bonus.c */
void		run_philo(t_philo_bonus *philo);

/* monitor_bonus.c */
void		*death_monitor(void *arg);
void		*meal_monitor(void *arg);

/* utils_bonus.c */
long long	get_time(void);
void		ft_usleep(long long ms);
void		print_status(t_philo_bonus *philo, char *msg);
int			ft_atoi(const char *str);
void		kill_all(t_data_bonus *data);

#endif
