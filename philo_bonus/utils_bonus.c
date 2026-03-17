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

/**
 * @brief Return the current wall-clock time in milliseconds.
 *
 * Uses @c gettimeofday for sub-millisecond source precision.
 *
 * @return Milliseconds since the Unix epoch.
 */
long long	get_time(void)
{
	struct timeval	tv;

	gettimeofday(&tv, NULL);
	return ((tv.tv_sec * 1000) + (tv.tv_usec / 1000));
}

/**
 * @brief Sleep for exactly @p ms milliseconds with low jitter.
 *
 * Loops, calling @c usleep(500), until at least @p ms milliseconds
 * have elapsed since the call started.  Avoids the over-sleep that
 * a single @c usleep(ms * 1000) would cause.
 *
 * @param ms  Number of milliseconds to wait.
 */
void	ft_usleep(long long ms)
{
	long long	start;

	start = get_time();
	while ((get_time() - start) < ms)
		usleep(500);
}

/**
 * @brief Print a timestamped status message for philosopher @p philo.
 *
 * Acquires @c print_sem before writing to stdout to prevent output
 * from multiple processes from interleaving.  The timestamp printed
 * is relative to @c data->start_time.
 *
 * @param philo  Philosopher whose status is being reported.
 * @param msg    Null-terminated status description string.
 */
void	print_status(t_philo_bonus *philo, char *msg)
{
	long long	time;

	sem_wait(philo->data->print_sem);
	time = get_time() - philo->data->start_time;
	printf("%lld %d %s\n", time, philo->id, msg);
	sem_post(philo->data->print_sem);
}

/**
 * @brief Convert a non-negative decimal ASCII string to an @c int.
 *
 * Reads consecutive digit characters from @p str until a non-digit
 * is found.  No sign handling; no overflow guard — the caller must
 * validate the string with @c check_args first.
 *
 * @param str  Null-terminated string of decimal digits.
 * @return     Integer value represented by @p str.
 */
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

/**
 * @brief Compose a named-semaphore path as "<base><id>".
 *
 * Copies @p base into @p dst, then appends the decimal digits of
 * @p id (1-3 digits, no leading zeros).  @p dst must be large
 * enough to hold the complete result including the NUL terminator.
 *
 * @param dst   Destination buffer provided by the caller.
 * @param base  Base prefix string (e.g. @c "/philo_prot_").
 * @param id    Integer suffix to append (expected range 1–200).
 */
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
