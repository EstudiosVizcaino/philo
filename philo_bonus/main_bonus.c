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

/**
 * @brief Validate that @p str is a non-negative decimal integer
 *        within INT_MAX.
 *
 * Returns 0 for empty strings, strings containing non-digit
 * characters, or values greater than 2 147 483 647.
 *
 * @param str  Null-terminated string to validate.
 * @return     1 if valid, 0 otherwise.
 */
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

/**
 * @brief Validate argument count and individual argument values.
 *
 * Expects 4 or 5 arguments after the program name.  The first four
 * (n, t_die, t_eat, t_sleep) must be strictly positive integers.
 * The optional fifth (n_eat) must be a valid non-negative integer
 * (0 is handled by the caller before @c init_data is reached).
 * Writes a usage or error message to stderr on failure.
 *
 * @param argc  Argument count as received by @c main.
 * @param argv  Argument vector as received by @c main.
 * @return      1 if all arguments are valid, 0 otherwise.
 */
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

/**
 * @brief Send SIGKILL to every live philosopher child process.
 *
 * Iterates @c data->pids and kills each entry whose value is > 0.
 * The > 0 guard prevents accidentally sending SIGKILL to the whole
 * process group if a @c fork failed before filling a slot.
 *
 * @param data  Simulation data whose @c pids array is iterated.
 */
void	kill_all(t_data_bonus *data)
{
	int	i;

	i = 0;
	while (i < data->num_philos)
	{
		if (data->pids[i] > 0)
			kill(data->pids[i], SIGKILL);
		i++;
	}
}

/**
 * @brief Fork all philosopher processes and wait for a stop signal.
 *
 * Records @c start_time, forks one child per philosopher (each child
 * calls @c run_philo and exits), then blocks on @c dead_sem.  When
 * @c dead_sem is posted — either by a dying philosopher or by the
 * meal monitor — @c kill_all terminates any remaining children.
 *
 * @param data  Pointer to fully-initialised simulation data.
 */
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

/**
 * @brief Program entry point for the bonus philosophers simulation.
 *
 * Validates arguments, short-circuits if n_eat is 0, initialises
 * data, launches the meal-monitor thread, runs the simulation via
 * @c start_processes, reaps all child processes with @c waitpid,
 * joins the meal thread, and calls @c cleanup before returning.
 *
 * @param argc  Argument count (must be 5 or 6).
 * @param argv  Argument vector: name n t_die t_eat t_sleep [n_eat].
 * @return      0 on normal exit, 1 on argument or init error.
 */
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
		if (data.pids[i] > 0)
			waitpid(data.pids[i], NULL, 0);
		sem_post(data.meal_sem);
		i++;
	}
	pthread_join(data.meal_thread, NULL);
	cleanup(&data);
	return (0);
}
