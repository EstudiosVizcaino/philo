<div align="center">

# 🍝 Philosophers

**A multithreaded solution to the Dining Philosophers problem, written in C.**

![Grade](https://img.shields.io/badge/grade-100%2F100-brightgreen?style=flat-square)
![Norminette](https://img.shields.io/badge/norminette-0%20errors-brightgreen?style=flat-square)
![Language](https://img.shields.io/badge/language-C-00599C?style=flat-square&logo=c&logoColor=white)
![42](https://img.shields.io/badge/42-Madrid-000000?style=flat-square)
![Threads](https://img.shields.io/badge/concurrency-pthreads-orange?style=flat-square)

</div>

---

## Table of Contents

- [Overview](#overview)
- [The Problem](#the-problem)
- [Getting Started](#getting-started)
- [Usage](#usage)
- [Output Format](#output-format)
- [Testing](#testing)
- [How It Works](#how-it-works)
- [Project Structure](#project-structure)
- [Design Notes](#design-notes)
- [Constraints](#constraints)
- [License](#license)

---

## Overview

`philo` simulates a table of philosophers who alternate between **eating**, **sleeping**
and **thinking**. Each philosopher is an independent thread, each fork is a mutex, and a
dedicated monitor thread watches for starvation. The simulation ends as soon as a
philosopher dies — or, optionally, once every philosopher has eaten a given number of
times.

This is the classic 42 School *Philosophers* project: an exercise in thread
synchronization, deadlock avoidance and precise timing under strict resource rules.

**Highlights**

- 🧵 One `pthread` per philosopher, plus a separate monitor thread
- 🔒 Deadlock-free fork acquisition via parity-based lock ordering
- ⏱️ Millisecond-accurate timestamps and sleeping, no busy-wait spikes
- 🛡️ Every shared field guarded by a mutex — no data races
- 🧹 Clean shutdown: all threads joined, all mutexes destroyed, zero leaks
- 📏 Compiles with `-Wall -Wextra -Werror` and follows the 42 Norm

---

## The Problem

> *N* philosophers sit around a circular table with a large bowl of spaghetti in the
> middle. There is exactly one fork between each pair of neighbours — *N* philosophers,
> *N* forks. A philosopher needs **both** the fork on their left and the fork on their
> right to eat. When done eating they put both forks down, sleep, then think, then get
> hungry again.

The difficulty is that the naive solution — "everyone grabs their left fork, then their
right" — **deadlocks**: every philosopher ends up holding one fork and waiting forever for
a neighbour who will never let go. A philosopher who waits too long between meals
**starves and dies**, and the simulation must report it within milliseconds.

Solving it means satisfying three properties at once:

| Property | Meaning |
| --- | --- |
| **Mutual exclusion** | A fork is held by at most one philosopher at a time. |
| **Deadlock freedom** | The table never reaches a state where everyone waits forever. |
| **Starvation freedom** | No philosopher is indefinitely denied access to the forks. |

---

## Getting Started

### Requirements

- A C compiler (`cc` / `gcc` / `clang`)
- `make`
- A POSIX system with the pthreads library (Linux or macOS)

### Build

```bash
git clone https://github.com/EstudiosVizcaino/philo.git
cd philo
make
```

This produces a `philo` binary in the project root.

### Make Targets

| Target | Description |
| --- | --- |
| `make` / `make all` | Build the `philo` binary |
| `make clean` | Remove object files |
| `make fclean` | Remove object files and the binary |
| `make re` | Full rebuild (`fclean` + `all`) |

---

## Usage

```bash
./philo <number_of_philosophers> <time_to_die> <time_to_eat> <time_to_sleep> [number_of_times_each_philosopher_must_eat]
```

| Argument | Type | Description |
| --- | --- | --- |
| `number_of_philosophers` | int > 0 | How many philosophers — and therefore how many forks — are at the table. |
| `time_to_die` | ms > 0 | If a philosopher does not *start* a new meal within this window since their last one began, they die. |
| `time_to_eat` | ms > 0 | How long a meal takes. Both forks stay held for its full duration. |
| `time_to_sleep` | ms > 0 | How long a philosopher sleeps after eating. |
| `number_of_times_each_philosopher_must_eat` | int > 0 | *Optional.* When every philosopher has eaten this many times, the simulation stops successfully. Omit it to run until someone dies. |

All arguments must be positive integers. Anything else — letters, signs, decimals, zero,
or the wrong number of arguments — is rejected with an error message and exit status `1`.

### Examples

```bash
./philo 5 800 200 200        # 5 philosophers, nobody should ever die
./philo 5 800 200 200 7      # stops once all 5 have eaten 7 meals
./philo 4 410 200 200        # tight but survivable
./philo 4 310 200 100        # a philosopher must die
./philo 1 800 200 200        # a lone philosopher can never eat — dies at 800ms
```

---

## Output Format

Every state change prints one line to standard output:

```
<timestamp_in_ms> <philosopher_id> <action>
```

The timestamp is measured from the start of the simulation, and philosopher IDs start at
`1`. The five possible actions are:

```
0 1 has taken a fork
0 1 has taken a fork
0 1 is eating
201 1 is sleeping
401 1 is thinking
801 1 died
```

Log lines are serialized behind a dedicated print mutex, so lines never interleave. Once
a death has been declared, no further state lines are printed — the death line is always
the last line of output.

---

## Testing

A quick set of scenarios worth running:

| Command | Expected result |
| --- | --- |
| `./philo 1 800 200 200` | Takes one fork, cannot get a second, dies at ~800 ms. |
| `./philo 5 800 200 200` | Runs indefinitely, nobody dies. |
| `./philo 4 410 200 200` | Runs indefinitely, nobody dies. |
| `./philo 100 800 200 200` | Runs indefinitely, nobody dies. |
| `./philo 5 800 200 200 7` | Exits cleanly once all philosophers have eaten 7 times. |
| `./philo 4 310 200 100` | One philosopher dies, and it is reported within 10 ms. |
| `./philo 0 800 200 200` | `Error: invalid argument` |
| `./philo 5 800 200` | Usage message |

Because output is block-buffered when piped, use `stdbuf -oL ./philo ...` when you want to
inspect a long-running simulation live through a pipe.

### Checking for leaks and races

```bash
valgrind --leak-check=full --show-leak-kinds=all ./philo 5 800 200 200 7
valgrind --tool=helgrind ./philo 5 800 200 200 7
cc -Wall -Wextra -Werror -pthread -fsanitize=thread *.c -o philo_tsan && ./philo_tsan 5 800 200 200 7
```

---

## How It Works

```
                 main thread
                      │
        ┌─────────────┼──────────────┬──────────────┐
        │             │              │              │
   philosopher 1  philosopher 2  ...  philosopher N   monitor
        │             │              │              │
        └──── forks[] (N mutexes) ───┘         polls every 1 ms
                      │                     ┌────────┴────────┐
              meal_mutex guards        death check      meal-count check
          last_meal_time / meals_eaten
```

1. **Startup.** Arguments are validated, `N` fork mutexes plus the print and meal mutexes
   are initialized, and every philosopher's `last_meal_time` is stamped with a common
   start time so the clocks agree.
2. **The philosopher loop.** Each thread repeats *take forks → eat → drop forks → sleep →
   think* until the simulation is flagged as over.
3. **The monitor.** A separate thread scans every philosopher once per millisecond. If any
   philosopher's time since their last meal exceeds `time_to_die`, it sets the death flag
   and prints the death line. If the optional meal target is given and everyone has
   reached it, it sets the completion flag instead.
4. **Shutdown.** Both flags cause the philosopher loops to exit. `main` joins every thread,
   then destroys the mutexes and frees the two heap allocations.

---

## Project Structure

```
philo/
├── philo.h        # Data structures (t_data, t_philo) and prototypes
├── main.c         # Argument validation, thread creation and joining
├── init.c         # Allocation, mutex initialization, teardown
├── routine.c      # The philosopher lifecycle: forks, eat, sleep, think
├── monitor.c      # Death detection and meal-target detection
├── utils.c        # Timing, precise sleep, synchronized logging, atoi
└── Makefile
```

| File | Responsibility |
| --- | --- |
| `main.c` | Validates input, spawns the philosopher and monitor threads, joins them, cleans up. |
| `init.c` | Allocates the fork and philosopher arrays, initializes all mutexes, and provides `cleanup()`. |
| `routine.c` | The per-thread routine, including the single-philosopher special case and fork ordering. |
| `monitor.c` | Polls philosopher state and decides when the simulation ends. |
| `utils.c` | `get_time`, `ft_usleep`, `print_status`, `is_dead`, `ft_atoi`. |

---

## Design Notes

### Deadlock avoidance — parity-based lock ordering

Odd-numbered philosophers take their **left** fork first; even-numbered philosophers take
their **right** fork first. This breaks the circular-wait condition, which is one of the
four Coffman conditions required for deadlock: with a consistent asymmetry in the table,
at least one philosopher can always complete their acquisition and release both forks.
Even philosophers also wait 1 ms before their first meal, which staggers the initial rush
on the forks.

### Starvation avoidance — a thinking delay

With an odd number of philosophers, forks cannot be shared in neat pairs, and a
philosopher can be repeatedly beaten to a fork by a faster neighbour. To even things out,
odd-sized tables insert a thinking delay of `time_to_eat * 2 - time_to_sleep`
(clamped at zero) after sleeping. This yields the fork to the neighbour who has been
waiting longest and keeps every philosopher's meal cadence within their death window.

### Precise timing

`get_time()` returns milliseconds from `gettimeofday`. Rather than a single `usleep()` —
which the kernel is free to overshoot — `ft_usleep()` loops in short 100 µs increments and
re-checks the clock, so meal and sleep durations stay accurate without pinning a core.
The monitor polls at 1 ms, comfortably within the requirement that a death be reported
within 10 ms.

### Race-free shared state

- `forks[i]` — one mutex per fork; only a holder may "use" it.
- `meal_mutex` — guards `last_meal_time` and `meals_eaten`, which the philosopher writes
  and the monitor reads.
- `print_mutex` — serializes all output and is checked against the end-of-simulation flags,
  so no state line can be printed after a death.

`last_meal_time` is stamped *before* the meal is announced, which is what makes the death
deadline count from the start of a meal rather than its end.

### The single philosopher

With one philosopher there is only one fork, so a second one can never be acquired. That
case is handled explicitly: take the fork, wait out `time_to_die`, and let the monitor
report the death — rather than blocking forever on a mutex that will never be released.

---

## Constraints

The 42 subject restricts the implementation to a small set of allowed functions:

`memset`, `printf`, `malloc`, `free`, `write`, `usleep`, `gettimeofday`,
`pthread_create`, `pthread_detach`, `pthread_join`, `pthread_mutex_init`,
`pthread_mutex_destroy`, `pthread_mutex_lock`, `pthread_mutex_unlock`

In addition:

- No global variables.
- Must compile with `-Wall -Wextra -Werror`.
- Must follow the 42 Norm (functions ≤ 25 lines, ≤ 5 functions per file, ≤ 4 parameters).
- No memory leaks, and every mutex must be destroyed before exit.

The original subject and evaluation sheet are included in this repository as
[`philosophers.pdf`](philosophers.pdf) and
[`philosophers_eval.pdf`](philosophers_eval.pdf).

---

## License

This is an educational project from the 42 School curriculum, published for reference and
study. No formal license is attached to the source. The subject and evaluation PDFs are
the property of 42 School and are included for reference only.

---

<div align="center">
<sub>Built as part of the 42 School curriculum.</sub>
</div>
