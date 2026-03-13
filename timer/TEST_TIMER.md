# Timer Subsystem Tests

**Suite name:** `Timer`
**Source directory:** `tests/timer/`
**Header:** `test_timer.h`
**Parent flag:** `TEST_TIMER` (auto-derived from sub-flags)

## Overview

Tests for the TikuOS timer subsystem, covering both software timers
(event timers, callback timers) and hardware timers (htimers). Software
timers are built on top of the process/event system and require the clock
ISR to be running. Hardware timers operate at ISR level with direct
hardware timer compare registers.

**Hardware dependency:** All tests require MSP430 hardware (`PLATFORM_MSP430`)
and run **after** `__enable_interrupt()` in the test runner.

**Key constants:**
- `TEST_TIMER_INTERVAL` - 1-second event timer interval (in ticks)
- `TEST_TIMER_SHORT` - 250 ms callback timer interval (in ticks)
- `TEST_TIMER_PERIODIC_CNT` - periodic callback repeat count (default: 3)
- `TEST_TIMER_DRAIN_MAX` - max polling iterations before timeout
- `TEST_HTIMER_PERIOD` - hardware timer period (default: 25 ms in ticks)
- `TEST_HTIMER_REPEAT_CNT` - hardware timer repeat count (default: 5)

---

## Tests

### 1. Event Timer

| Field | Value |
|-------|-------|
| **Flag** | `TEST_TIMER_EVENT` |
| **Source** | `test_timer_event.c` |
| **Function** | `test_timer_event()` |
| **Group** | "Event Timer" |

**What it tests:**
Sets a 1-second event timer (`tiku_timer_set_event()`) from within a
protothread. The process waits for `TIKU_EVENT_TIMER` using
`TIKU_PROCESS_WAIT_EVENT_UNTIL`. A polling loop advances the clock and
requests timer polls until the event fires or the loop times out.

**Assertions:**
1. `event_timer_fired` - Event timer fired
2. `tiku_timer_expired(&event_tmr)` - Timer reports expired

---

### 2. Callback Timer

| Field | Value |
|-------|-------|
| **Flag** | `TEST_TIMER_CALLBACK` |
| **Source** | `test_timer_callback.c` |
| **Function** | `test_timer_callback()` |
| **Group** | "Callback Timer" |

**What it tests:**
Sets a 250 ms callback timer (`tiku_timer_set_callback()`) with a function
pointer. The callback increments a counter when fired. A polling loop waits
for the callback to execute.

**Assertions:**
1. `callback_count == 1` - Callback fired exactly once

---

### 3. Periodic Timer (Drift-Free Reset)

| Field | Value |
|-------|-------|
| **Flag** | `TEST_TIMER_PERIODIC` |
| **Source** | `test_timer_periodic.c` |
| **Function** | `test_timer_periodic()` |
| **Group** | "Periodic Timer" |

**What it tests:**
A callback timer that reschedules itself using `tiku_timer_reset()` for
drift-free periodic execution. The callback fires `TEST_TIMER_PERIODIC_CNT`
times (default: 3), each at `TEST_TIMER_SHORT` interval. After the last
tick, it does NOT reschedule.

**Assertions:**
1. `periodic_count == TEST_TIMER_PERIODIC_CNT` - Periodic timer fired expected number of times
2. `tiku_timer_expired(&periodic_tmr)` - Timer stopped after final tick

---

### 4. Timer Stop

| Field | Value |
|-------|-------|
| **Flag** | `TEST_TIMER_STOP` |
| **Source** | `test_timer_stop.c` |
| **Function** | `test_timer_stop()` |
| **Group** | "Timer Stop" |

**What it tests:**
Sets a 1-second callback timer, verifies it is active, then immediately
stops it with `tiku_timer_stop()`. Runs the scheduler past the original
expiration time to confirm the callback never fires.

**Assertions:**
1. `!tiku_timer_expired(&stop_tmr)` - Timer is active after set
2. `tiku_timer_expired(&stop_tmr)` - Timer reports expired after stop
3. `stop_callback_fired == 0` - Callback did not fire after stop

---

### 5. Hardware Timer Basic (One-Shot)

| Field | Value |
|-------|-------|
| **Flag** | `TEST_HTIMER_BASIC` |
| **Source** | `test_htimer_basic.c` |
| **Function** | `test_htimer_basic()` |
| **Group** | "Hardware Timer Basic" |

**What it tests:**
Schedules a one-shot hardware timer (`tiku_htimer_set()`) for
`TEST_HTIMER_PERIOD` ticks from now. The ISR callback sets a flag. A
busy-wait loop (up to 50000 iterations) checks for the flag. After the
one-shot fires, no timer should be pending.

**Note:** The scheduled state is captured immediately after `tiku_htimer_set()`
and before any UART I/O, because at 9600 baud a single printf takes ~36 ms,
long enough for the ISR to fire and clear the pending state.

**Assertions:**
1. `ret == TIKU_HTIMER_OK` - `tiku_htimer_set` succeeded
2. `is_sched` - HTimer reports scheduled (captured before UART I/O)
3. `htimer_basic_fired` - HTimer one-shot fired
4. `!tiku_htimer_is_scheduled()` - No htimer scheduled after one-shot

---

### 6. Hardware Timer Periodic (Self-Reschedule)

| Field | Value |
|-------|-------|
| **Flag** | `TEST_HTIMER_PERIODIC` |
| **Source** | `test_htimer_periodic.c` |
| **Function** | `test_htimer_periodic()` |
| **Group** | "Hardware Timer Periodic" |

**What it tests:**
Schedules a hardware timer that reschedules itself in its ISR callback,
creating drift-free periodic ticks. Fires `TEST_HTIMER_REPEAT_CNT` times
(default: 5) at `TEST_HTIMER_PERIOD` intervals (default: 25 ms). The
callback does NO UART I/O to avoid timing drift (printf at 9600 baud takes
~31 ms, exceeding the 25 ms period).

**Assertions:**
1. `ret == TIKU_HTIMER_OK` - `tiku_htimer_set` succeeded
2. `htimer_periodic_count == TEST_HTIMER_REPEAT_CNT` - HTimer periodic completed expected ticks
3. `!tiku_htimer_is_scheduled()` - No htimer scheduled after final tick

---

## Requests

<!--
  Add new test requests below. Each request should describe:
  - What behavior to test
  - Expected outcome
  - Any specific parameters or edge cases

  An LLM will read these requests and generate the corresponding C test
  code in a new file under tests/timer/. After implementation, move
  the request to the Tests section above with full documentation.

  Example format:

  ### Request: Timer restart with different interval
  Set a callback timer for 500ms, stop it before it fires, then restart
  with a 100ms interval. Verify the callback fires at the new shorter
  interval, not the original. Expected: callback_count == 1 within ~150ms.
-->
