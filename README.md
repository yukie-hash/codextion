*This project has been created as part of the 42 curriculum by yhamada.*

## Description
Codexion is an adaptation of the "Dining Philosophers Problem" to a fight over USB dongles in a shared coding hub. It asks you to implement the four major themes of concurrent programming — deadlock avoidance, starvation avoidance, fair scheduling (FIFO / EDF), and precise timing — using only pthread mutexes and condition variables.

## Instructions

### Requirements
- A C compiler (`cc`) and `make`
- An environment that supports POSIX threads (pthread) (Linux / macOS)

### Compilation
Run the following at the root of the repository to build the `codexion` executable.

```bash
make
```

It is compiled with `-Wall -Wextra -Werror -pthread`.

| Command | Description |
|---|---|
| `make` / `make all` | Build `codexion` |
| `make clean` | Remove the object files (`obj/`) |
| `make fclean` | Same as `clean`, and also remove `codexion` |
| `make re` | Run `fclean`, then build again |

### Usage
The program takes 8 arguments, all of them mandatory.

```bash
./codexion number_of_coders time_to_burnout time_to_compile time_to_debug \
           time_to_refactor number_of_compiles_required dongle_cooldown scheduler
```

| Argument | Unit | Meaning |
|---|---|---|
| `number_of_coders` | coders | Number of coders (= number of dongles). Must be at least 1 |
| `time_to_burnout` | ms | A coder burns out if they do not start compiling within this time since their last compile started (or since the simulation started) |
| `time_to_compile` | ms | Time it takes to compile. The coder holds two dongles during this time |
| `time_to_debug` | ms | Time it takes to debug |
| `time_to_refactor` | ms | Time it takes to refactor |
| `number_of_compiles_required` | times | The simulation ends when every coder has compiled at least this many times |
| `dongle_cooldown` | ms | Time after a dongle is released before it can be used again |
| `scheduler` | — | Rule that decides who gets a dongle: `fifo` (in order of request) or `edf` (earliest burnout deadline first) |

Only integers from 0 to 2147483647 are accepted as numbers. Negative numbers, decimals, values containing non-digit characters, and any scheduler other than `fifo` / `edf` are rejected: the program prints the reason and the usage to standard error and exits with status 1.

### Examples

```bash
# Nobody burns out; everyone compiles 7 times and the simulation ends
./codexion 5 800 200 200 200 7 0 fifo

# The deadline is too short, so someone burns out
./codexion 4 310 200 100 100 5 0 edf

# With a dongle cooldown
./codexion 4 600 100 100 100 5 50 edf

# A single coder (there is only one dongle, so they cannot compile and burn out)
./codexion 1 800 200 200 200 5 0 fifo
```

### Output
Each time a coder's state changes, one line is printed to standard output. The first number is the elapsed time since the start of the simulation (ms), and the second is the coder number.

```
0 1 has taken a dongle
0 1 has taken a dongle
0 1 is compiling
0 3 has taken a dongle
0 3 has taken a dongle
0 3 is compiling
200 1 is debugging
...
1204 3 burned out
```

The simulation ends when a coder burns out (`burned out` is then the last line), or when every coder has compiled `number_of_compiles_required` times.

## Resources
- Introduction To Threads (pthreads) | C Programming Tutorial  
https://www.youtube.com/watch?v=ldJ8WGZVXZk&t=176s
- スレッドの紹介 (Introduction to threads, in Japanese)  
https://www.youtube.com/watch?v=LOfGJcVnvAk
- 食事する哲学者の問題 (The Dining Philosophers Problem, in Japanese)  
https://www.youtube.com/watch?v=FYUi-u7UWgw
- 食の哲学者 (Dining Philosophers, in Japanese)  
https://www.youtube.com/watch?v=iQVgyzgSUYM

### AI Resources
- Translating the subject
- Brainstorming to understand pthreads
- Generating and explaining usage examples of pthread functions
- Discussing the design
- Discussing function names
- Explaining heap operations
- Drawing diagrams of scenarios that lead to starvation
- Writing test cases

## Blocking cases handled
### Deadlock prevention and Coffman's conditions

#### Deadlock scenario
- If every coder grabs only their left dongle at the same moment, nobody can take their right one, and they all wait for each other in a circle forever (the classic deadlock).

#### How to avoid deadlock
- Break at least one of Coffman's four conditions (the conditions under which a deadlock can occur):
  - **Mutual Exclusion**: a resource (data, a device, etc.) can be used by only one process at a time. Others must wait until it is finished.
  - **Hold and Wait**: a process holds a resource while waiting for another resource to become free.
  - **Non-preemption**: a resource that another process is already using cannot be taken away by force. You can only wait for the owner to release it.
  - **Circular Wait**: processes wait for each other's resources in a circle (e.g. process A waits for B's resource, and B waits for A's).

#### What this implementation does
- Manage all competition for dongles in one place → so that scheduling can be controlled centrally
- Take the dongles only when both can be taken at the same time → checked in the branch that acquires the dongles

### Starvation prevention
#### What is starvation?
- A state in which the system as a whole keeps running, but one particular participant never gets the resource.
- Unlike deadlock, it has no fixed shape: it happens as a weakness of the rule that decides who goes first. So you have to think about what can go wrong with the rule you chose.

#### Starvation that can happen in this project
1. **Basic rule**: each dongle has its own waiting queue, and the coder at the front takes it first (FIFO: in order of request, EDF: earliest deadline first).
2. **Problem**: if the coder at the front cannot take their dongles yet, the coders behind them are blocked too, even when their own dongles are free (head-of-line blocking). Even with 4 coders, only one could compile at a time, and coders burned out under parameters that should have been feasible.
3. **Allowing overtaking causes starvation**: to fix 2, we could say "if the coder in front cannot take their dongles, you may overtake them". But then the coder in the middle is in trouble. A coder can only compile when both the left and the right dongle are free at the same time, so if both neighbors keep using them in turn, the coder in the middle never gets them.

#### Solution: allow overtaking only when the other coder still meets their deadline (`src/scheduler_rival.c`)
- You may overtake the coder in front of you (R) only when both of the following are true:
  1. R cannot take their dongles right now
  2. Even if you go first, R still meets their deadline (`now + time_to_compile + dongle_cooldown + 5ms <= R's deadline`)
- Condition 1 keeps free dongles from being wasted, and condition 2 prevents R from burning out because of overtaking. As R's deadline gets closer, nobody can overtake R anymore.

### Dongle cooldown handling

#### Requirement
- After a dongle is released, nobody can take it until `dongle_cooldown` ms have passed.

#### Implementation
- Each dongle stores "the time when it becomes available again" (`dongle_cooldown_until[]`).
- When a dongle is released, `now + dongle_cooldown` is recorded (`release_dongles`).
- A dongle can be taken only when it is free **and** that time has passed (`dongle_ready`).
- Both values are read and written inside `dongle_mutex`, so other coders never see a half-updated state right after a release.

#### How coders wait
- The way of waiting depends on why the dongles cannot be taken (`wait_for_dongles`).

| Why the dongles cannot be taken | How the coder waits | Reason |
|---|---|---|
| Someone is using them | `pthread_cond_wait` | The coder is woken up by `pthread_cond_broadcast` when the dongles are released |
| They are cooling down | `pthread_cond_timedwait` (until the cooldown ends) | A cooldown simply ends when time passes, and nobody wakes the coder up. With `cond_wait`, the coder could keep sleeping |

- Since the coder sleeps until a specific time, there is no need to check repeatedly at short intervals (polling), and no CPU time is wasted.

### Precise burnout detection

#### Requirement
- A monitor thread, separate from the coders, detects burnout and stops the simulation.
- The `burned out` message must be printed within 10 ms of the actual burnout.

#### How it is detected (`src/monitor.c`)
- Every 1 ms, the monitor checks all coders and treats as burned out any coder for whom `now - last compile start > time_to_burnout`.
- The initial value of "last compile start" is the simulation start time. This way, "a coder burns out if they do not start their first compile within `time_to_burnout` of the start" is checked with the same formula.
- Coders who have finished the required number of compiles are excluded (they no longer need to compile).
- In the same check, the monitor also looks at whether every coder has finished the required number of compiles; if so, the simulation ends normally.

#### Reading values safely
- The last compile start time is updated inside the same `dongle_mutex` as taking the dongles. The monitor also reads it inside that mutex, so it never sees a half-updated value such as "the dongles have been taken but the time is still old".

#### Why it stays within 10 ms
- The detection delay is the 1 ms check interval + oversleeping in `usleep` (usually about 1 ms) + time spent waiting for the mutex. Inside the mutex, the monitor only loops over an array once, which is short. The total fits comfortably within 10 ms.

#### How the simulation is stopped
1. Inside `dongle_mutex`, the monitor sets `stop = 1` and wakes up every waiting coder with `pthread_cond_broadcast`.
2. When it prints `burned out`, it also closes the log (`log_closed = 1`). From then on, no other coder's log is printed.
3. Each coder notices the stop and leaves its loop: right away if it was waiting for dongles (it is woken up), or within the 5 ms check of `sleep_ms` if it was working.

#### Problem and solution
A plain `usleep` cannot notice when `pthread_cond_broadcast` is called. That is why coders sleep in slices and check every 5 ms.

### Log serialization

#### Requirement
- Even when several coders print at the same time, two messages must never be mixed in one line.

#### Possible problems
- If several threads call `printf` at the same time without protection, the output can get mixed in the middle (e.g. `200 1 is debu200 3 has taken a dongle`).
- If another thread cuts in between taking the timestamp and printing, the order of timestamps and the order of lines can be reversed (e.g. a `200` line printed after a `201` line).

#### Implementation (`src/log.c`)
- There is one mutex dedicated to logging (`log_mutex`), and it is always taken before printing.
- Inside the mutex, "get the elapsed time → print one line with `printf`" is done as one step.
  - Each line is printed with a single `printf`, so lines are never mixed.
  - The time is also taken inside the mutex, so the order of lines always matches the order of timestamps.
- When `burned out` is printed, `log_closed` is set to 1. Other coders check `log_closed` before printing, so nothing is printed after `burned out`.

## Thread synchronization mechanisms

### Synchronization primitives used

| Primitive | Type | What it protects |
|---|---|---|
| `dongle_mutex` | `pthread_mutex_t` | The state of the dongles (free / cooldown), the waiting queues (heaps), each coder's last compile start time and compile count, and the stop flag `stop` |
| `dongle_cond` | `pthread_cond_t` | Used together with `dongle_mutex`. Tells coders that "the dongle situation has changed" or that "the simulation must stop" |
| `log_mutex` | `pthread_mutex_t` | Standard output, and the flag `log_closed` that tells whether the log is closed |

- There are only two mutexes and one condition variable. The fewer there are, the easier it is to see which mutex protects what, and the less likely a deadlock caused by taking mutexes in different orders becomes.
- The two mutexes are never held at the same time.
- Values that are fixed before the threads are created and never change afterwards (the arguments, the start time) are only read, so they are not protected by a mutex.

### Dongles: taking and releasing
- **Taking** (`take_dongles`)
  1. Lock `dongle_mutex` and put your ticket in the left and right waiting queues.
  2. Wait in a `while` loop until "both dongles are free, their cooldown is over, and it is your turn". If someone is using them, sleep with `pthread_cond_wait`; if they are cooling down, sleep with `pthread_cond_timedwait`.
  3. Once you can take them, remove your ticket from the queues, mark both dongles as in use, and record the compile start time. All of this is done inside one mutex.
  4. Unlock the mutex, then print `has taken a dongle` twice.
- **Releasing** (`release_dongles`)
  - Inside `dongle_mutex`, mark the dongles as free, update the cooldown end time and the compile count, and then wake up every waiting coder with `pthread_cond_broadcast`.
- **Why the condition is re-checked in a `while` loop**: a condition variable can wake up even without a notification (spurious wakeup). Also, being woken up by a broadcast does not mean it is your turn.
- **Why `broadcast` instead of `signal`**: who can take the dongles depends on the order of the queues, so waking up only one coder might wake up someone who cannot take them.

### Logging
- Inside `log_mutex`, "get the elapsed time → print one line with `printf`" is done. Lines are never mixed, and the order of timestamps matches the order of lines (see "Log serialization" for details).

### Communication between coders and the monitor
- Coders and the monitor do not talk to each other directly. They share information through values protected by `dongle_mutex`.

| Direction | What is passed | Written by | Read by |
|---|---|---|---|
| Coder → Monitor | Last compile start time | The coder, when taking the dongles | The monitor, every 1 ms |
| Coder → Monitor | Compile count | The coder, when releasing the dongles | The monitor, to check whether everyone has finished |
| Monitor → Coder | Stop flag `stop` | The monitor, when it finds a burnout or that everyone has finished | Coders, while waiting and while working |
| Monitor → Coder | Whether the log is closed (`log_closed`) | The monitor, when printing `burned out` (`log_mutex`) | Coders, before printing a log line |

- **How the stop is delivered**: inside `dongle_mutex`, the monitor sets `stop = 1` and then calls `pthread_cond_broadcast`. Coders sleeping while waiting for dongles are woken up, see `stop`, and leave their loop. Coders that are working check `stop` every 5 ms (`sleep_ms`).
- **Why `stop` is protected by `dongle_mutex`**: `stop` is part of the condition that coders wait on with the condition variable. If it were protected by a different mutex, the monitor could broadcast between the moment a coder checks "`stop` is still 0" and the moment it goes to sleep; the coder would then miss the notification and keep sleeping (a lost wakeup). Protecting it with the same mutex means nothing can cut in between the check and going to sleep.

### Examples of preventing race conditions (data races)
- **Taking the same dongle twice**: two neighbors both decide "it is free" at the same time and both take it. Because everything from the check to the acquisition is done inside `dongle_mutex`, one coder can only check after the other has finished taking.
- **Wrong detection based on an old compile start time**: a coder has taken the dongles, but the monitor reads the old time before it is updated and reports a burnout. Taking the dongles and updating the time happen inside the same mutex, and the monitor reads inside the same mutex too, so it never sees a half-updated state.
- **Mixed log lines**: lines are printed one at a time under `log_mutex`, so they never get mixed.
