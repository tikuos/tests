# Process / Protothread Tests

**Suite name:** `Process`
**Source directory:** `tests/process/`
**Header:** `test_process.h`
**Parent flag:** `TEST_PROCESS` (auto-derived from sub-flags)

## Overview

Tests for the TikuOS process subsystem (`tiku_process_*` API), which provides
cooperative multitasking via protothreads. Processes communicate through an
event queue and are scheduled by a simple run-to-completion dispatcher.

These tests run **before** interrupts are enabled (no timer ISRs). All
scheduling is driven by explicit `tiku_process_run()` calls.

**Key constants:**
- `TEST_EVENT_CUSTOM` - application-defined event type for testing
- `TEST_NUM_EVENTS` - number of events to post (default: 5)
- `TEST_NUM_POLLS` - number of poll requests (default: 3)

---

## Tests

### 1. Basic Process Lifecycle

| Field | Value |
|-------|-------|
| **Flag** | `TEST_PROCESS_LIFECYCLE` |
| **Source** | `test_process_lifecycle.c` |
| **Function** | `test_process_lifecycle()` |
| **Group** | "Basic Process Lifecycle" |

**What it tests:**
Creates a single process that advances through two phases. Phase 1 runs on
INIT event delivery, then the process yields with `TIKU_PROCESS_WAIT_EVENT()`.
Phase 2 runs when a CONTINUE event is posted. After PROCESS_END, the process
auto-exits.

**Assertions:**
1. `lifecycle_phase == 1` - Process received INIT event
2. `test_lifecycle_proc.is_running` - Process is running after yield
3. `lifecycle_phase == 2` - Process reached phase 2
4. `!test_lifecycle_proc.is_running` - Process auto-exited at PROCESS_END

---

### 2. Event Posting

| Field | Value |
|-------|-------|
| **Flag** | `TEST_PROCESS_EVENTS` |
| **Source** | `test_process_events.c` |
| **Function** | `test_process_events()` |
| **Group** | "Event Posting" |

**What it tests:**
Posts `TEST_NUM_EVENTS` custom events to a process that counts them in a
while loop. Verifies all events are delivered and that `tiku_process_post()`
returns success.

**Assertions:**
1. Each `tiku_process_post()` returns success
2. `event_recv_count == TEST_NUM_EVENTS` - All custom events received
3. Post after all events drained returns 1 (queue not full)

---

### 3. Cooperative Yield

| Field | Value |
|-------|-------|
| **Flag** | `TEST_PROCESS_YIELD` |
| **Source** | `test_process_yield.c` |
| **Function** | `test_process_yield()` |
| **Group** | "Cooperative Yield" |

**What it tests:**
A process executes three phases separated by `TIKU_PROCESS_YIELD()`. Each
CONTINUE event resumes the process from where it yielded, advancing to the
next phase. After phase 3 (PROCESS_END), the process terminates.

**Assertions:**
1. `yield_phase == 1` - Phase 1 reached after INIT
2. `yield_phase == 2` - Phase 2 reached after first resume
3. `yield_phase == 3` - Phase 3 reached after second resume
4. `!test_yield_proc.is_running` - Process ended after final phase

---

### 4. Broadcast Events

| Field | Value |
|-------|-------|
| **Flag** | `TEST_PROCESS_BROADCAST` |
| **Source** | `test_process_broadcast.c` |
| **Function** | `test_process_broadcast()` |
| **Group** | "Broadcast Events" |

**What it tests:**
Two processes (A and B) are started. A broadcast event
(`TIKU_PROCESS_BROADCAST` as target) is posted. Both processes must receive
the event. A second broadcast verifies repeated delivery.

**Assertions:**
1. `bcast_count_a == 1` - Process A received broadcast
2. `bcast_count_b == 1` - Process B received broadcast
3. `bcast_count_a == 2 && bcast_count_b == 2` - Both received second broadcast

---

### 5. Process Poll

| Field | Value |
|-------|-------|
| **Flag** | `TEST_PROCESS_POLL` |
| **Source** | `test_process_poll.c` |
| **Function** | `test_process_poll()` |
| **Group** | "Process Poll" |

**What it tests:**
Requests `TEST_NUM_POLLS` poll events via `tiku_process_poll()`. The target
process counts `TIKU_EVENT_POLL` events and toggles LED2 on each delivery.

**Assertions:**
1. `poll_count == TEST_NUM_POLLS` - Received all poll events

---

### 6. Queue Query Functions

| Field | Value |
|-------|-------|
| **Flag** | `TEST_PROCESS_QUEUE` |
| **Source** | `test_process_queue.c` |
| **Function** | `test_process_queue_query()` |
| **Group** | "Queue Query Functions" |

**What it tests:**
Exercises the queue introspection API: `tiku_process_queue_empty()`,
`tiku_process_queue_length()`, `tiku_process_queue_space()`,
`tiku_process_queue_full()`, and `tiku_process_is_running()`. Runs with
interrupts disabled (`tiku_atomic_enter()`) to prevent timer ISR interference.

Tests the queue through its full lifecycle: empty after init, length 1 after
process start (INIT event), empty after drain, full after posting
`TIKU_QUEUE_SIZE` events, and post failure when full.

**Assertions:**
1. Queue empty after init
2. Queue length is 0 after init
3. Queue space equals `TIKU_QUEUE_SIZE` after init
4. Queue not full after init
5. Queue length is 1 after process start
6. Queue not empty after post
7. Queue space decremented after 1 post
8. Queue empty after draining events
9. Each of `TIKU_QUEUE_SIZE` posts succeeds
10. Queue is full after filling
11. Queue length equals `TIKU_QUEUE_SIZE`
12. Queue space is 0 when full
13. Post returns 0 when queue is full
14. Queue length decremented after one run
15. Queue space is 1 after draining one
16. Queue no longer full after drain
17. `is_running` returns 1 for active process
18. `is_running` returns 0 after exit

---

### 7. Process Local Storage

| Field | Value |
|-------|-------|
| **Flag** | `TEST_PROCESS_LOCAL` |
| **Source** | `test_process_local.c` |
| **Function** | `test_process_local_storage()` |
| **Group** | "Process Local Storage" |

**What it tests:**
Three-part test:

**Part A: `TIKU_PROCESS_WITH_LOCAL` + `TIKU_LOCAL` accessor.**
A process with a `local_test_state` struct (counter, phase) advances through
3 phases, mutating local storage at each yield. Verifies state survives yields.

**Part B: `TIKU_PROCESS_TYPED` accessor.**
A typed process uses `test_typed_proc_local()` to access its `typed_test_state`.
Sets value=42, yields, then increments to 50. Verifies typed accessor works.

**Part C:** Plain `TIKU_PROCESS_WITH_LOCAL` has non-NULL `.local` pointer.

**Assertions:**
1. `counter==10, phase==1` after phase 1
2. `counter==15, phase==2` after phase 2 (state survived yield)
3. `counter==16, phase==3` at exit
4. Process exited normally
5. Typed accessor returns `value==42`
6. Typed state `value==50` after yield
7. Typed process exited normally
8. `WITH_LOCAL` process has non-NULL local

---

### 8. Broadcast Exit Safety

| Field | Value |
|-------|-------|
| **Flag** | `TEST_PROCESS_BROADCAST_EXIT` |
| **Source** | `test_process_broadcast_exit.c` |
| **Function** | `test_process_broadcast_exit()` |
| **Group** | "Broadcast Exit Safety" |

**What it tests:**
Three processes (A, B, C). On receiving a broadcast, process B calls
`TIKU_PROCESS_EXIT()`, unlinking itself from the process list mid-iteration.
Verifies that processes A and C still receive the broadcast despite B
modifying the list during delivery.

This is a regression test for broadcast list corruption.

**Assertions:**
1. `bce_count_a == 1` - Process A received broadcast
2. `bce_count_b == 1` - Process B received broadcast before exit
3. `!test_bce_proc_b.is_running` - Process B exited during broadcast
4. `bce_count_c == 1` - Process C received broadcast after B exited

---

### 9. Graceful Exit vs Force Exit

| Field | Value |
|-------|-------|
| **Flag** | `TEST_PROCESS_GRACEFUL_EXIT` |
| **Source** | `test_process_graceful_exit.c` |
| **Function** | `test_process_graceful_exit()` |
| **Group** | "Graceful Exit vs Force Exit" |

**What it tests:**
Two-part test comparing exit mechanisms:

**Part A: `TIKU_EVENT_EXIT` (graceful).** Process receives the event, performs
cleanup (sets `ge_cleanup_done`), then explicitly calls `TIKU_PROCESS_EXIT()`.

**Part B: `TIKU_EVENT_FORCE_EXIT` (unconditional).** Process receives the event
and does NOT call `TIKU_PROCESS_EXIT()`. The scheduler kills it anyway.

**Assertions:**
1. `ge_cleanup_done == 1` - Graceful process ran cleanup
2. `!test_graceful_proc.is_running` - Graceful process exited after cleanup
3. `fe_cleanup_done == 1` - Force process thread body executed
4. `!test_force_proc.is_running` - Force process killed unconditionally

---

### 10. Current Process Cleared After Dispatch

| Field | Value |
|-------|-------|
| **Flag** | `TEST_PROCESS_CURRENT_CLEARED` |
| **Source** | `test_process_current_cleared.c` |
| **Function** | `test_process_current_cleared()` |
| **Group** | "Current Process Cleared After Dispatch" |

**What it tests:**
Inside the process thread, `TIKU_THIS()` should return a pointer to the
currently running process. After dispatch returns (scheduler idle),
`TIKU_THIS()` must be NULL to prevent dangling references.

**Assertions:**
1. `cc_inside_ok == 1` - `TIKU_THIS()` valid inside process thread
2. `TIKU_THIS() == NULL` - `TIKU_THIS()` is NULL after dispatch

---

## Requests

<!--
  Add new test requests below. Each request should describe:
  - What behavior to test
  - Expected outcome
  - Any specific parameters or edge cases

  An LLM will read these requests and generate the corresponding C test
  code in a new file under tests/process/. After implementation, move
  the request to the Tests section above with full documentation.

  Example format:

  ### Request: Process priority ordering
  Create 3 processes with different priorities. Post events to all three
  and verify that the scheduler delivers events in priority order (highest
  first). Expected: high-priority process runs before low-priority even
  if low-priority was started first.
-->
