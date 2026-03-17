*Este proyecto ha sido creado como parte del currículo de 42 por cvizcain.*

# philo — Dining Philosophers

## Description

A classic concurrency problem: **N philosophers** sit around a circular table
with **N forks** (one between each pair of neighbours). A philosopher alternates
between *thinking*, *eating* (picking up both adjacent forks), and *sleeping*.
If a philosopher goes longer than `time_to_die` milliseconds without eating, it
dies and the simulation ends.

This mandatory implementation uses **POSIX threads** (one per philosopher plus
one monitor) and **mutexes** (one per fork, plus one for logging and one for
shared state) — no global variables, no data races.

## Instructions

### Build

```
make
```

The binary `philo` is produced. Standard `clean`, `fclean`, and `re` targets
are also available. To generate HTML documentation with Doxygen run:

```
make docs
```

The output is written to `docs/html/index.html`.

An evaluator-focused interactive study guide is also available at:

```
docs/philo_master_study_guide.html
```

### Run

```
./philo <n_philos> <time_to_die> <time_to_eat> <time_to_sleep> [n_meals]
```

| Argument       | Unit | Meaning                                                  |
|----------------|------|----------------------------------------------------------|
| `n_philos`     | —    | Number of philosophers (and forks)                       |
| `time_to_die`  | ms   | Max time between the start of two consecutive meals      |
| `time_to_eat`  | ms   | Time spent eating (both forks held)                      |
| `time_to_sleep`| ms   | Time spent sleeping after each meal                      |
| `n_meals`      | —    | *(optional)* Stop once every philosopher has eaten this many times |

### Examples

```
./philo 5 800 200 200        # five philosophers, none should die
./philo 5 800 200 200 7      # stop after every philosopher eats 7 times
./philo 4 310 200 100        # one philosopher dies around 310 ms
./philo 1 800 200 200        # single philosopher, must die
./philo 199 600 200 200 10   # stress test — 199 threads, 10 meals each
```

### Evaluation test matrix

| Command                   | Expected result                              |
|---------------------------|----------------------------------------------|
| `1 800 200 200`           | Philosopher dies ~800 ms                     |
| `5 800 200 200`           | No death                                     |
| `5 800 200 200 7`         | No death, exits after 7 meals                |
| `4 410 200 200`           | No death                                     |
| `4 310 200 100`           | One philosopher dies                         |
| `2 <any>`                 | Death message printed within 10 ms of actual death |
| `199 600 200 200 10`      | No death, all 199 philosophers eat 10 times  |

## Resources

### References

- Dijkstra, E. W. (1965). *Solution of a problem in concurrent programming control*.
  Communications of the ACM, 8(9), 569.
- [POSIX Threads Programming — Lawrence Livermore National Laboratory](https://hpc-tutorials.llnl.gov/posix/)
- [pthread_mutex_lock(3) — Linux man-pages](https://man7.org/linux/man-pages/man3/pthread_mutex_lock.3p.html)
- [gettimeofday(2) — Linux man-pages](https://man7.org/linux/man-pages/man2/gettimeofday.2.html)
- 42 subject: `philosophers.pdf` (included in this repository)
- 42 evaluation sheet: `philosophers_eval.pdf` (included in this repository)

### AI usage

AI assistance (GitHub Copilot / Claude) was used in this project for:

- **Timing logic** — identifying that `think_time = eat×2 − sleep` produced a
  philosopher cycle exactly equal to `time_to_die`, leaving zero margin for OS
  scheduling jitter; replaced with `(die − eat − sleep) / 2`.
- **Initial stagger** — identifying that a 1 ms stagger for even-ID philosophers
  was negligible with large N; replaced with `time_to_eat` for clean fork
  alternation.
- **Doxygen comments** — generating the `/** */` documentation blocks added to
  each source file and the Doxyfile configuration.

All AI-generated suggestions were reviewed, understood, and validated by the
author before being committed.
