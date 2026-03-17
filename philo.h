/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   philo.h                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cvizcain <cvizcain@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/16 18:22:34 by cvizcain          #+#    #+#             */
/*   Updated: 2026/03/16 22:06:32 by cvizcain         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PHILO_H
# define PHILO_H

# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <unistd.h>
# include <sys/time.h>
# include <pthread.h>

typedef struct s_data	t_data;

typedef struct s_philo
{
	int				id;
	int				meals_eaten;
	long long		last_meal_time;
	pthread_t		thread;
	pthread_mutex_t	*left_fork;
	pthread_mutex_t	*right_fork;
	t_data			*data;
}	t_philo;

typedef struct s_data
{
	int				num_philos;
	long long		time_to_die;
	long long		time_to_eat;
	long long		time_to_sleep;
	int				must_eat;
	int				dead;
	int				all_ate;
	int				ready;
	int				finished_eating;
	long long		start_time;
	long long		think_time;
	pthread_mutex_t	*forks;
	pthread_mutex_t	print_mutex;
	pthread_mutex_t	meal_mutex;
	pthread_t		monitor;
	t_philo			*philos;
}	t_data;

/* init.c */
int			init_data(t_data *data, int argc, char **argv);
void		cleanup(t_data *data);

/* routine.c */
void		*philo_routine(void *arg);

/* monitor.c */
void		*monitor_routine(void *arg);

/* utils.c */
long long	get_time(void);
void		ft_usleep(long long ms);
void		print_status(t_philo *philo, char *msg);
int			is_dead(t_data *data);
int			ft_atoi(const char *str);

#endif
