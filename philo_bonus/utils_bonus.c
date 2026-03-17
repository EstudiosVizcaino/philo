/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils_bonus.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cvizcain <cvizcain@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/17 10:09:41 by cvizcain          #+#    #+#             */
/*   Updated: 2026/03/17 10:09:41 by cvizcain         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo_bonus.h"

long long	get_time(void)
{
	struct timeval	tv;

	gettimeofday(&tv, NULL);
	return ((tv.tv_sec * 1000) + (tv.tv_usec / 1000));
}

void	ft_usleep(long long ms)
{
	long long	start;

	start = get_time();
	while ((get_time() - start) < ms)
		usleep(500);
}

void	print_status(t_philo_bonus *philo, char *msg)
{
	long long	time;

	sem_wait(philo->data->print_sem);
	time = get_time() - philo->data->start_time;
	printf("%lld %d %s\n", time, philo->id, msg);
	sem_post(philo->data->print_sem);
}

int	ft_atoi(const char *str)
{
	long long	result;
	int			i;

	result = 0;
	i = 0;
	while (str[i] >= '0' && str[i] <= '9')
	{
		result = result * 10 + (str[i] - '0');
		i++;
	}
	return ((int)result);
}

void	ft_build_name(char *dst, char *base, int id)
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
