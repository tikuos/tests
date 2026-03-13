# Memory Subsystem Tests

**Suite name:** `Memory`
**Source directory:** `tests/memory/`
**Header:** `test_tiku_mem.h`
**Source files:** `test_mem_arena.c`, `test_mem_pool.c`, `test_mem_persist.c`, `test_mem_mpu.c`, `test_mem_region.c`, `test_mem_common.c`

## Overview

Tests for the TikuOS memory management subsystem, covering five components:

1. **Arena allocator** - bump-pointer allocator with alignment, stats, and secure reset
2. **Pool allocator** - fixed-size block allocator with freelist, debug poisoning, and stats
3. **Persistent FRAM store** - key-value store backed by non-volatile FRAM
4. **MPU write protection** - MSP430 Memory Protection Unit permission management
5. **Region registry** - memory region type tracking, containment checks, and claim management

All memory tests run **before** interrupts are enabled. Shared test buffers
(`test_sram_pool`, `test_nvm_pool`) and the helper `test_region_reinit()` are
defined in `test_mem_common.c`.

---

## Arena Allocator Tests

**Parent flag:** `TEST_MEM` (auto-derived)

### 1. Creation and Initial Stats

| Field | Value |
|-------|-------|
| **Flag** | `TEST_MEM_CREATE` |
| **Source** | `test_mem_arena.c` : `test_mem_create_and_stats()` |
| **Group** | "Creation and Initial Stats" |

**What it tests:**
Creates an arena over a 64-byte buffer and queries initial stats. Verifies the
arena is active, has the correct ID, and all stats counters start at zero.

**Assertions:**
1. `arena_create` returns OK
2. Arena is active after create
3. Arena ID is set correctly
4. `arena_stats` returns OK
5. `total_bytes == 64`
6. `used_bytes == 0`
7. `peak_bytes == 0`
8. `alloc_count == 0`

---

### 2. Basic Allocation

| Field | Value |
|-------|-------|
| **Flag** | `TEST_MEM_ALLOC` |
| **Source** | `test_mem_arena.c` : `test_mem_basic_alloc()` |
| **Group** | "Basic Allocation" |

**What it tests:**
Allocates 8 bytes then 4 bytes from a 64-byte arena. Verifies pointers are
at expected offsets (contiguous), writes distinct patterns to both allocations,
and confirms they don't overlap. Checks stats reflect 12 bytes used and 2
allocations.

**Assertions:**
1. First alloc returns non-NULL at buffer base
2. Second alloc returns non-NULL at offset 8
3. Memory regions don't overlap (pattern integrity)
4. `used_bytes == 12`, `alloc_count == 2`

---

### 3. Alignment

| Field | Value |
|-------|-------|
| **Flag** | `TEST_MEM_ALIGNMENT` |
| **Source** | `test_mem_arena.c` : `test_mem_alignment()` |
| **Group** | "Alignment" |

**What it tests:**
Requests odd sizes (3, 1, 5 bytes) and verifies the arena rounds each up to
`TIKU_MEM_ARCH_ALIGNMENT`. Checks that returned pointers are at the correct
aligned offsets and that all pointers satisfy platform alignment requirements.

**Assertions:**
1. 3-byte alloc starts at base
2. Next alloc starts at aligned offset (not offset 3)
3. 5-byte request at correct aligned offset
4. `used_bytes` reflects aligned sizes
5. All 3 pointers are platform-aligned

---

### 4. Arena Full

| Field | Value |
|-------|-------|
| **Flag** | `TEST_MEM_FULL` |
| **Source** | `test_mem_arena.c` : `test_mem_arena_full()` |
| **Group** | "Arena Full" |

**What it tests:**
Creates an arena of size `4*A` (where A = alignment). Allocates `3*A` bytes
(succeeds), attempts `2*A` (fails - only A bytes left), allocates exactly A
(succeeds), then attempts 1 byte (fails - completely full).

**Assertions:**
1. `3*A` alloc succeeds in `4*A` arena
2. `2*A` alloc fails when only A bytes remain
3. Exact-fit A-byte alloc succeeds
4. 1-byte alloc fails when arena is full

---

### 5. Reset

| Field | Value |
|-------|-------|
| **Flag** | `TEST_MEM_RESET` |
| **Source** | `test_mem_arena.c` : `test_mem_reset()` |
| **Group** | "Reset" |

**What it tests:**
Allocates 20 bytes (12+8), resets the arena, verifies counters are zeroed but
peak is preserved. Re-allocates from base to confirm reset worked.

**Assertions:**
1. 20 bytes used before reset
2. `arena_reset` returns OK
3. `used_bytes == 0` after reset
4. `alloc_count == 0` after reset
5. `peak_bytes == 20` preserved after reset
6. Allocation after reset starts at base

---

### 6. Peak Tracking Across Resets

| Field | Value |
|-------|-------|
| **Flag** | `TEST_MEM_PEAK` |
| **Source** | `test_mem_arena.c` : `test_mem_peak_tracking()` |
| **Group** | "Peak Tracking Across Resets" |

**What it tests:**
Three allocation/reset cycles: 12 bytes, then 32 bytes (new peak), then 8
bytes. Verifies peak tracks the lifetime maximum and doesn't decrease in
subsequent cycles.

**Assertions:**
1. Peak is 12 after first cycle
2. Peak is 32 after second cycle
3. Peak remains 32 after smaller third cycle
4. `used_bytes == 8` in third cycle

---

### 7. Invalid Inputs

| Field | Value |
|-------|-------|
| **Flag** | `TEST_MEM_INVALID` |
| **Source** | `test_mem_arena.c` : `test_mem_invalid_inputs()` |
| **Group** | "Invalid Inputs" |

**What it tests:**
Exercises all NULL-pointer and zero-size guard checks: NULL arena in create,
NULL buffer in create, alloc from NULL arena, alloc of 0 bytes, reset NULL
arena, stats with NULL arena, stats with NULL output.

**Assertions:**
1. Create with NULL arena returns `ERR_INVALID`
2. Create with NULL buffer returns `ERR_INVALID`
3. Alloc from NULL arena returns NULL
4. Alloc of 0 bytes returns NULL
5. Reset NULL arena returns `ERR_INVALID`
6. Stats with NULL arena returns `ERR_INVALID`
7. Stats with NULL output returns `ERR_INVALID`

---

### 8. Secure Reset

| Field | Value |
|-------|-------|
| **Flag** | `TEST_MEM_SECURE_RESET` |
| **Source** | `test_mem_arena.c` : `test_mem_secure_reset()` |
| **Group** | "Secure Reset" |

**What it tests:**
Fills a 32-byte arena with 0xAA pattern, calls `tiku_arena_secure_reset()`,
then verifies every byte is zeroed. Confirms stats are reset but peak is
preserved. Also tests NULL arena rejection.

**Assertions:**
1. Buffer has data before secure reset
2. `secure_reset` returns OK
3. All bytes zeroed after secure reset
4. `used_bytes == 0`, `alloc_count == 0`
5. `peak_bytes == 32` preserved
6. Allocation after secure reset starts at base
7. Secure reset of NULL arena returns `ERR_INVALID`

---

### 9. Two Independent Arenas

| Field | Value |
|-------|-------|
| **Flag** | `TEST_MEM_TWO_ARENAS` |
| **Source** | `test_mem_arena.c` : `test_mem_two_arenas()` |
| **Group** | "Two Independent Arenas" |

**What it tests:**
Creates two arenas (A: 32 bytes, B: 64 bytes) on separate buffers. Allocates
from each, verifies they use their own buffers. Resets A and confirms B is
unaffected. Re-allocates from A and confirms B still unaffected.

**Assertions:**
1. Arena A allocs from buf_a, Arena B from buf_b
2. Stats are independent (A: 8 bytes, B: 16 bytes)
3. Resetting A doesn't affect B's used_bytes
4. Re-allocating from A doesn't affect B

---

## Pool Allocator Tests

**Parent flag:** `TEST_POOL` (auto-derived)

### 10. Pool Creation and Initial Stats

| Field | Value |
|-------|-------|
| **Flag** | `TEST_POOL_CREATE` |
| **Source** | `test_mem_pool.c` : `test_pool_create_and_stats()` |
| **Group** | "Pool Creation and Initial Stats" |

**What it tests:**
Creates a pool with 4 blocks of 8 bytes each. Verifies pool metadata and
initial stats are correct.

**Assertions:**
1. `pool_create` returns OK
2. Pool is active, ID correct, `block_count == 4`, `used_count == 0`
3. `free_head` is non-NULL
4. Stats: `total_bytes`, `used_bytes == 0`, `peak_bytes == 0`, `alloc_count == 0`

---

### 11. Basic Alloc and Free

| Field | Value |
|-------|-------|
| **Flag** | `TEST_POOL_ALLOC_FREE` |
| **Source** | `test_mem_pool.c` : `test_pool_basic_alloc_free()` |
| **Group** | "Pool Basic Alloc and Free" |

**What it tests:**
Allocates two blocks, verifies they're distinct and non-overlapping (pattern
write test). Frees the first block and checks stats update.

**Assertions:**
1. Two allocs return non-NULL, different pointers
2. Memory patterns intact (no overlap)
3. Stats reflect 2 blocks used
4. After free, `alloc_count == 1`

---

### 12. Pool Exhaustion

| Field | Value |
|-------|-------|
| **Flag** | `TEST_POOL_EXHAUSTION` |
| **Source** | `test_mem_pool.c` : `test_pool_exhaustion()` |
| **Group** | "Pool Exhaustion" |

**What it tests:**
Allocates all 4 blocks, attempts a 5th (returns NULL). Frees one block, then
allocates again (succeeds).

**Assertions:**
1. All 4 block allocs succeed
2. 5th alloc returns NULL
3. Alloc after free succeeds

---

### 13. Free Out Of Range

| Field | Value |
|-------|-------|
| **Flag** | `TEST_POOL_FREE_RANGE` |
| **Source** | `test_mem_pool.c` : `test_pool_free_out_of_range()` |
| **Group** | "Pool Free Out Of Range" |

**What it tests:**
Attempts to free pointers from a different buffer, past the pool end, and
before the pool start. All must be rejected.

**Assertions:**
1. Free of out-of-range pointer returns `ERR_INVALID`
2. Free of pointer past buffer end returns `ERR_INVALID`
3. Free of pointer before buffer start returns `ERR_INVALID`

---

### 14. Free Misaligned

| Field | Value |
|-------|-------|
| **Flag** | `TEST_POOL_FREE_ALIGN` |
| **Source** | `test_mem_pool.c` : `test_pool_free_misaligned()` |
| **Group** | "Pool Free Misaligned" |

**What it tests:**
Attempts to free pointers within the pool buffer but not on block boundaries
(offset 1, block_size+1). Both must be rejected.

**Assertions:**
1. Free of misaligned pointer (offset 1) returns `ERR_INVALID`
2. Free of misaligned pointer (block+1) returns `ERR_INVALID`

---

### 15. Alloc-Free-Realloc (LIFO)

| Field | Value |
|-------|-------|
| **Flag** | `TEST_POOL_REALLOC` |
| **Source** | `test_mem_pool.c` : `test_pool_alloc_free_realloc()` |
| **Group** | "Pool Alloc-Free-Realloc (LIFO)" |

**What it tests:**
Allocates p1 then p2. Frees p2 then p1. Re-allocates twice and verifies LIFO
ordering: first re-alloc returns p1 (last freed), second returns p2.

**Assertions:**
1. Re-alloc returns last freed block (p1)
2. Second re-alloc returns previously freed block (p2)

---

### 16. Peak Tracking

| Field | Value |
|-------|-------|
| **Flag** | `TEST_POOL_PEAK` |
| **Source** | `test_mem_pool.c` : `test_pool_peak_tracking()` |
| **Group** | "Pool Peak Tracking" |

**What it tests:**
Allocates 3 blocks (peak = 3), frees all, allocates 1. Verifies peak remains
at 3 throughout.

**Assertions:**
1. Peak is 3 blocks after 3 allocs
2. Peak preserved after freeing all blocks
3. Peak remains 3 after smaller reuse

---

### 17. Pool Reset

| Field | Value |
|-------|-------|
| **Flag** | `TEST_POOL_RESET` |
| **Source** | `test_mem_pool.c` : `test_pool_reset()` |
| **Group** | "Pool Reset" |

**What it tests:**
Allocates all 4 blocks, resets pool. Verifies stats zeroed, peak preserved,
and allocation resumes from base.

**Assertions:**
1. `pool_reset` returns OK
2. `used_bytes == 0`, `alloc_count == 0`
3. `peak_bytes` preserved
4. Allocation succeeds from base after reset

---

### 18. Pool Invalid Inputs

| Field | Value |
|-------|-------|
| **Flag** | `TEST_POOL_INVALID` |
| **Source** | `test_mem_pool.c` : `test_pool_invalid_inputs()` |
| **Group** | "Pool Invalid Inputs" |

**What it tests:**
NULL pool, NULL buffer, zero block count in create. Alloc from NULL pool.
Free to NULL pool, free NULL pointer. Reset NULL pool. Stats with NULL
pool/output.

**Assertions:**
1. Create with NULL pool/buffer/zero-count returns `ERR_INVALID`
2. Alloc from NULL pool returns NULL
3. Free to NULL pool returns `ERR_INVALID`
4. Free NULL pointer returns `ERR_INVALID`
5. Reset/stats with NULL returns `ERR_INVALID`

---

### 19. Two Independent Pools

| Field | Value |
|-------|-------|
| **Flag** | `TEST_POOL_TWO_POOLS` |
| **Source** | `test_mem_pool.c` : `test_pool_two_pools()` |
| **Group** | "Two Independent Pools" |

**What it tests:**
Creates two pools (A: 8-byte blocks, B: 16-byte blocks). Allocates from both,
resets A, frees from B. Verifies operations on one pool don't affect the other.

**Assertions:**
1. Both pools allocate independently
2. Resetting A doesn't affect B
3. Freeing from B works independently

---

### 20. Block Size Alignment

| Field | Value |
|-------|-------|
| **Flag** | `TEST_POOL_BLOCK_ALIGN` |
| **Source** | `test_mem_pool.c` : `test_pool_block_size_alignment()` |
| **Group** | "Pool Block Size Alignment" |

**What it tests:**
Creates pools with block_size=1 and block_size=7. Verifies the pool clamps to
minimum (freelist pointer size) and rounds up to platform alignment.

**Assertions:**
1. `block_size >= sizeof(void *)` (minimum for freelist pointer)
2. `block_size % alignment == 0`
3. Odd block_size rounded up correctly

---

### 21. Stats Mapping

| Field | Value |
|-------|-------|
| **Flag** | `TEST_POOL_STATS` |
| **Source** | `test_mem_pool.c` : `test_pool_stats_mapping()` |
| **Group** | "Pool Stats Mapping" |

**What it tests:**
Allocates 2 blocks and verifies stats fields map correctly to pool internals:
`total_bytes = block_size * block_count`, `used_bytes = block_size * used_count`,
etc.

**Assertions:**
1. `total_bytes = block_size * block_count`
2. `used_bytes = block_size * 2`
3. `alloc_count = used_count`
4. `peak_bytes = block_size * 2`

---

### 22. Debug Poisoning

| Field | Value |
|-------|-------|
| **Flag** | `TEST_POOL_POISON` |
| **Source** | `test_mem_pool.c` : `test_pool_debug_poisoning()` |
| **Group** | "Pool Debug Poisoning" |

**What it tests:**
When `TIKU_POOL_DEBUG` is enabled: allocates a 16-byte block, fills with 0xAA,
frees it. Verifies all bytes after the freelist pointer are poisoned with 0xDE.
Skipped when `TIKU_POOL_DEBUG` is disabled.

**Assertions:**
1. Freed bytes after freelist ptr are 0xDE (when debug enabled)
2. Block has bytes beyond freelist pointer to poison

---

### 23. Alloc Within Buffer

| Field | Value |
|-------|-------|
| **Flag** | `TEST_POOL_WITHIN_BUF` |
| **Source** | `test_mem_pool.c` : `test_pool_alloc_within_buffer()` |
| **Group** | "Pool Alloc Within Buffer" |

**What it tests:**
Allocates all 4 blocks and verifies each pointer falls within the pool's
backing buffer.

**Assertions:**
1. Each block starts at or after buffer start
2. Each block end is within buffer bounds

---

## Persistent FRAM Store Tests

**Parent flag:** `TEST_PERSIST` (auto-derived)

### 24. Init on Zeroed Store

| Field | Value |
|-------|-------|
| **Flag** | `TEST_PERSIST_INIT` |
| **Source** | `test_mem_persist.c` : `test_persist_init_zeroed()` |
| **Group** | "Persist Init on Zeroed Store" |

**What it tests:**
Initializes a zeroed-out store and verifies count is 0. Also tests NULL
rejection.

**Assertions:**
1. `persist_init` returns OK
2. `count == 0` on zeroed store
3. NULL store returns `ERR_INVALID`

---

### 25. Register and Count

| Field | Value |
|-------|-------|
| **Flag** | `TEST_PERSIST_REGISTER` |
| **Source** | `test_mem_persist.c` : `test_persist_register_and_count()` |
| **Group** | "Persist Register and Count" |

**What it tests:**
Registers a key and verifies count increments. Tests NULL argument rejection
for store, key, buffer, and zero capacity.

**Assertions:**
1. `count == 0` before register
2. `count == 1` after first register
3. NULL store/key/buffer and zero capacity all return `ERR_INVALID`

---

### 26. Write Then Read

| Field | Value |
|-------|-------|
| **Flag** | `TEST_PERSIST_WRITE_READ` |
| **Source** | `test_mem_persist.c` : `test_persist_write_read()` |
| **Group** | "Persist Write Then Read" |

**What it tests:**
Writes 4 bytes (`0xDE 0xAD 0xBE 0xEF`), reads them back, and compares.

**Assertions:**
1. Write returns OK
2. Read returns OK
3. Read length matches write length
4. Read data matches written data

---

### 27. Read Small Buffer

| Field | Value |
|-------|-------|
| **Flag** | `TEST_PERSIST_SMALL_BUF` |
| **Source** | `test_mem_persist.c` : `test_persist_read_small_buffer()` |
| **Group** | "Persist Read Small Buffer" |

**What it tests:**
Writes 8 bytes, attempts to read into a 2-byte buffer. Verifies the read
fails with `ERR_NOMEM` and reports the required size in `out_len`.

**Assertions:**
1. Read with small buffer returns `ERR_NOMEM`
2. `out_len` reports required size (8)

---

### 28. Write Exceeds Capacity

| Field | Value |
|-------|-------|
| **Flag** | `TEST_PERSIST_OVERFLOW` |
| **Source** | `test_mem_persist.c` : `test_persist_write_exceeds_capacity()` |
| **Group** | "Persist Write Exceeds Capacity" |

**What it tests:**
Registers a key with 4-byte capacity, attempts to write 8 bytes.

**Assertions:**
1. Write exceeding capacity returns `ERR_NOMEM`

---

### 29. Read Not Found

| Field | Value |
|-------|-------|
| **Flag** | `TEST_PERSIST_NOT_FOUND` |
| **Source** | `test_mem_persist.c` : `test_persist_read_not_found()` |
| **Group** | "Persist Read Not Found" |

**What it tests:**
Reads a key that was never registered.

**Assertions:**
1. Read non-existent key returns `ERR_NOT_FOUND`

---

### 30. Delete

| Field | Value |
|-------|-------|
| **Flag** | `TEST_PERSIST_DELETE` |
| **Source** | `test_mem_persist.c` : `test_persist_delete()` |
| **Group** | "Persist Delete" |

**What it tests:**
Registers and writes a key, deletes it, verifies count decremented, read
returns NOT_FOUND. Also tests deleting a non-existent key.

**Assertions:**
1. Delete returns OK
2. Count decremented after delete
3. Read after delete returns `ERR_NOT_FOUND`
4. Delete non-existent key returns `ERR_NOT_FOUND`

---

### 31. Store Full

| Field | Value |
|-------|-------|
| **Flag** | `TEST_PERSIST_FULL` |
| **Source** | `test_mem_persist.c` : `test_persist_full()` |
| **Group** | "Persist Store Full" |

**What it tests:**
Fills all `TIKU_PERSIST_MAX_ENTRIES` slots, then attempts one more register.

**Assertions:**
1. All `TIKU_PERSIST_MAX_ENTRIES` registers succeed
2. Count equals max entries
3. Register beyond max returns `ERR_FULL`

---

### 32. Reboot Survival

| Field | Value |
|-------|-------|
| **Flag** | `TEST_PERSIST_REBOOT` |
| **Source** | `test_mem_persist.c` : `test_persist_reboot_survival()` |
| **Group** | "Persist Reboot Survival" |
| **Hardware-only** | Yes (uses `__attribute__((section(".persistent")))` and software POR) |

**What it tests:**
Two-phase test across a real device reset. Phase WRITE: writes data to
FRAM-backed store, sets phase flag, triggers BOR via `PMMCTL0 = PMMPW | PMMSWPOR`.
Phase VERIFY (after reboot): re-inits store, reads data back, compares.

**Assertions (Phase VERIFY):**
1. `count == 1` after reboot
2. Read after reboot returns OK
3. Data length preserved
4. Data intact after reboot

---

### 33. Power-Cycle Survival

| Field | Value |
|-------|-------|
| **Flag** | `TEST_PERSIST_POWERCYCLE` |
| **Source** | `test_mem_persist.c` : `test_persist_powercycle_survival()` |
| **Group** | "Persist Power-Cycle Survival" |
| **Hardware-only** | Yes (requires physical power removal) |

**What it tests:**
Similar to reboot test but requires physical power removal (USB disconnect /
battery removal) instead of software reset. Phase WRITE: writes data, prompts
user to remove power (LED1 blinks). Phase VERIFY (after power restore): waits
10s for console, re-inits, reads back, compares.

**Assertions (Phase VERIFY):**
1. `count == 1` after power-cycle
2. Read after power-cycle returns OK
3. Data length preserved
4. Data intact after power-cycle

---

### 34. Wear Check

| Field | Value |
|-------|-------|
| **Flag** | `TEST_PERSIST_WEAR` |
| **Source** | `test_mem_persist.c` : `test_persist_wear_check()` |
| **Group** | "Persist Wear Check" |

**What it tests:**
Registers a key, checks initial wear (0 writes, no warning). Manually sets
`write_count` above `TIKU_PERSIST_WEAR_THRESHOLD`, verifies the wear check
returns 1. Also tests non-existent key.

**Assertions:**
1. Wear check returns 0 initially
2. `write_count == 0` initially
3. Wear check returns 1 above threshold
4. `write_count` matches set value
5. Non-existent key returns `ERR_NOT_FOUND`

---

### 35. Register Same Key Twice

| Field | Value |
|-------|-------|
| **Flag** | `TEST_PERSIST_DUP_KEY` |
| **Source** | `test_mem_persist.c` : `test_persist_register_twice()` |
| **Group** | "Persist Register Same Key Twice" |

**What it tests:**
Registers a key, writes data, re-registers with a different buffer. Verifies
count stays 1 (no duplicate slot) and `value_len` is preserved.

**Assertions:**
1. Re-register returns OK
2. Count stays 1
3. Read after re-register returns OK
4. `value_len` preserved after re-register

---

## MPU Write Protection Tests

**Parent flag:** `TEST_MPU` (auto-derived)

### 36. Init Defaults

| Field | Value |
|-------|-------|
| **Flag** | `TEST_MPU_INIT` |
| **Source** | `test_mem_mpu.c` : `test_mpu_init_defaults()` |
| **Group** | "MPU Init Defaults" |

**What it tests:**
Calls `tiku_mpu_init()` and verifies SAM register is 0x0555 (all segments
R+X, no W) and MPU enable bit is set.

**Assertions:**
1. SAM is 0x0555 after init
2. CTL has enable bit set

---

### 37. Unlock / Lock

| Field | Value |
|-------|-------|
| **Flag** | `TEST_MPU_UNLOCK_LOCK` |
| **Source** | `test_mem_mpu.c` : `test_mpu_unlock_lock()` |
| **Group** | "MPU Unlock / Lock" |

**What it tests:**
Unlocks NVM (ORs write bits 0x0222 into SAM), verifies SAM becomes 0x0777.
Locks with saved state, verifies SAM restored to 0x0555.

**Assertions:**
1. Unlock returns previous SAM (0x0555)
2. SAM is 0x0777 after unlock
3. SAM restored to 0x0555 after lock

---

### 38. Set Permissions

| Field | Value |
|-------|-------|
| **Flag** | `TEST_MPU_SET_PERM` |
| **Source** | `test_mem_mpu.c` : `test_mpu_set_permissions()` |
| **Group** | "MPU Set Permissions" |

**What it tests:**
Sets segment 3 to RD_WR (0x03), verifies segments 1 and 2 are unchanged.

**Assertions:**
1. Segment 1 unchanged (R+X)
2. Segment 2 unchanged (R+X)
3. Segment 3 set to RD_WR (0x3)

---

### 39. Scoped Write

| Field | Value |
|-------|-------|
| **Flag** | `TEST_MPU_SCOPED` |
| **Source** | `test_mem_mpu.c` : `test_mpu_scoped_write()` |
| **Group** | "MPU Scoped Write" |

**What it tests:**
Calls `tiku_mpu_scoped_write()` with a callback that captures the SAM during
execution. Verifies write bits are set during the callback and SAM is restored
after.

**Assertions:**
1. Callback was invoked
2. SAM had write bits during callback (0x0222 bits set)
3. SAM locked again after scoped_write (0x0555)

---

### 40. Lock/Unlock Idempotency

| Field | Value |
|-------|-------|
| **Flag** | `TEST_MPU_IDEMPOTENT` |
| **Source** | `test_mem_mpu.c` : `test_mpu_idempotent()` |
| **Group** | "MPU Lock/Unlock Idempotency" |

**What it tests:**
Locks when already locked (no change). Double unlocks (second returns
already-unlocked state 0x0777). Restoring with original saved state relocks.

**Assertions:**
1. Lock when already locked keeps 0x0555
2. First unlock returns 0x0555
3. Second unlock returns 0x0777
4. SAM still 0x0777 after double unlock
5. Lock with original saved state restores 0x0555

---

### 41. All Segments Independent

| Field | Value |
|-------|-------|
| **Flag** | `TEST_MPU_ALL_SEGMENTS` |
| **Source** | `test_mem_mpu.c` : `test_mpu_all_segments()` |
| **Group** | "MPU All Segments Independent" |

**What it tests:**
Sets each segment to a different permission (ALL, READ, RD_WR). Then changes
only segment 2 and verifies segments 1 and 3 are unchanged.

**Assertions:**
1. Segment 1 is ALL (0x7)
2. Segment 2 is READ (0x1)
3. Segment 3 is RD_WR (0x3)
4. After changing seg 2 to EXEC: seg 1 and 3 unchanged

---

### 42. Permission Flags

| Field | Value |
|-------|-------|
| **Flag** | `TEST_MPU_PERM_FLAGS` |
| **Source** | `test_mem_mpu.c` : `test_mpu_permission_flags()` |
| **Group** | "MPU Permission Flags" |

**What it tests:**
Tests each permission enum value (READ, WRITE, EXEC, RD_WR, RD_EXEC, ALL) on
segment 2 and verifies the expected SAM nybble value.

**Assertions:**
1. READ maps to 0x01
2. WRITE maps to 0x02
3. EXEC maps to 0x04
4. RD_WR maps to 0x03
5. RD_EXEC maps to 0x05
6. ALL maps to 0x07

---

### 43. Re-init Restores Defaults

| Field | Value |
|-------|-------|
| **Flag** | `TEST_MPU_REINIT` |
| **Source** | `test_mem_mpu.c` : `test_mpu_reinit_restores()` |
| **Group** | "MPU Re-init Restores Defaults" |

**What it tests:**
Applies custom permissions to all segments, then calls `tiku_mpu_init()` again.
Verifies SAM returns to default 0x0555 and MPU remains enabled.

**Assertions:**
1. SAM is non-default after custom permissions
2. SAM restored to 0x0555 after re-init
3. MPU still enabled after re-init

---

### 44. Unlock Custom Base

| Field | Value |
|-------|-------|
| **Flag** | `TEST_MPU_UNLOCK_CUSTOM` |
| **Source** | `test_mem_mpu.c` : `test_mpu_unlock_custom_base()` |
| **Group** | "MPU Unlock Custom Base" |

**What it tests:**
Sets non-default permissions (seg1=ALL, seg2=READ, seg3=default), creating
base 0x0517. Unlocks (ORs 0x0222) to get 0x0737. Verifies each segment got
write bit without losing existing bits. Locks to restore base.

**Assertions:**
1. Custom base is 0x0517
2. Unlock returns custom base
3. SAM is 0x0737 after unlock
4. Each segment has correct combined permissions
5. Lock restores custom base

---

### 45. Scoped Write Custom Base

| Field | Value |
|-------|-------|
| **Flag** | `TEST_MPU_SCOPED_CUSTOM` |
| **Source** | `test_mem_mpu.c` : `test_mpu_scoped_write_custom()` |
| **Group** | "MPU Scoped Write Custom Base" |

**What it tests:**
Sets seg2 to EXEC-only (base 0x0545). Scoped write ORs 0x0222 to get 0x0767
during callback. Verifies restoration to 0x0545 after.

**Assertions:**
1. Custom base is 0x0545
2. Callback invoked
3. SAM during callback is 0x0767
4. SAM restored to 0x0545 after scoped write

---

### 46. Violation Detection

| Field | Value |
|-------|-------|
| **Flag** | `TEST_MPU_VIOLATION` |
| **Source** | `test_mem_mpu.c` : `test_mpu_violation_detect()` |
| **Group** | "MPU Violation Detection" |

**What it tests:**
Phase 1: With all segments locked (R+X), writes to protected FRAM (0x4400 on
hardware, stub on host). Verifies violation flag is set. Clears flags.
Phase 2: Unlocks NVM, writes again. Verifies no violation when writable.

**Assertions:**
1. No violation flags after init
2. Violation flag set after write to locked segment
3. Violation flags cleared successfully
4. No violation when segment is writable

---

## Region Registry Tests

**Parent flag:** `TEST_REGION` (auto-derived)

### 47. Init Valid

| Field | Value |
|-------|-------|
| **Flag** | `TEST_REGION_INIT` |
| **Source** | `test_mem_region.c` : `test_region_init_valid()` |
| **Group** | "Region Init Valid" |

**What it tests:**
Initializes the region registry with the platform-provided table and verifies
it has at least SRAM and NVM regions.

**Assertions:**
1. Init with valid table succeeds
2. Table has at least 2 regions

---

### 48. Init Invalid

| Field | Value |
|-------|-------|
| **Flag** | `TEST_REGION_INIT_INVALID` |
| **Source** | `test_mem_region.c` : `test_region_init_invalid()` |
| **Group** | "Region Init Invalid" |

**What it tests:**
NULL table, zero count, and overlapping regions all rejected.

**Assertions:**
1. NULL table returns `ERR_INVALID`
2. Zero count returns `ERR_INVALID`
3. Overlapping regions return `ERR_INVALID`

---

### 49. Contains Basic

| Field | Value |
|-------|-------|
| **Flag** | `TEST_REGION_CONTAINS` |
| **Source** | `test_mem_region.c` : `test_region_contains_basic()` |
| **Group** | "Region Contains Basic" |

**What it tests:**
SRAM buffer recognized as SRAM, NVM buffer as NVM. NULL pointer and zero size
rejected.

**Assertions:**
1. SRAM buffer recognized as SRAM
2. NVM buffer recognized as NVM
3. NULL pointer rejected
4. Zero size rejected

---

### 50. Contains Wrong Type

| Field | Value |
|-------|-------|
| **Flag** | `TEST_REGION_WRONG_TYPE` |
| **Source** | `test_mem_region.c` : `test_region_contains_wrong_type()` |
| **Group** | "Region Contains Wrong Type" |

**What it tests:**
SRAM buffer should not match NVM type, NVM should not match SRAM, SRAM should
not match PERIPHERAL.

**Assertions:**
1. SRAM buffer not NVM
2. NVM buffer not SRAM
3. SRAM buffer not PERIPHERAL

---

### 51. Boundary Conditions

| Field | Value |
|-------|-------|
| **Flag** | `TEST_REGION_BOUNDARY` |
| **Source** | `test_mem_region.c` : `test_region_contains_boundary()` |
| **Group** | "Region Contains Boundary" |

**What it tests:**
Uses a custom region table matching test pool sizes exactly. Tests: exact
region boundary passes, one byte past end fails, partial overflow fails,
single byte at start/end of region passes.

**Assertions:**
1. Exact region boundary passes
2. One byte past end fails
3. Partial overflow past end fails
4. Single byte at start passes
5. Single byte at last position passes

---

### 52. Overflow Protection

| Field | Value |
|-------|-------|
| **Flag** | `TEST_REGION_OVERFLOW` |
| **Source** | `test_mem_region.c` : `test_region_contains_overflow()` |
| **Group** | "Region Contains Overflow" |

**What it tests:**
Crafts a pointer near address space max so `ptr + size` wraps to zero. Verifies
the overflow check catches this.

**Assertions:**
1. Wrapping pointer range rejected

---

### 53. Claim and Unclaim

| Field | Value |
|-------|-------|
| **Flag** | `TEST_REGION_CLAIM` |
| **Source** | `test_mem_region.c` : `test_region_claim_unclaim()` |
| **Group** | "Region Claim and Unclaim" |

**What it tests:**
Claims a range, unclaims it, tries unclaiming again (NOT_FOUND), unclaims NULL
(NOT_FOUND), re-claims after unclaim.

**Assertions:**
1. Claim succeeds
2. Unclaim succeeds
3. Double unclaim returns NOT_FOUND
4. Unclaim NULL returns NOT_FOUND
5. Re-claim after unclaim succeeds

---

### 54. Claim Overlap

| Field | Value |
|-------|-------|
| **Flag** | `TEST_REGION_CLAIM_OVERLAP` |
| **Source** | `test_mem_region.c` : `test_region_claim_overlap()` |
| **Group** | "Region Claim Overlap" |

**What it tests:**
Claims [0,64), tries overlapping [32,96) and exact duplicate - both rejected.
Non-overlapping [128,192) and adjacent [64,128) both succeed.

**Assertions:**
1. First claim succeeds
2. Overlapping claim rejected
3. Duplicate claim rejected
4. Non-overlapping claim succeeds
5. Adjacent claim succeeds

---

### 55. Claim Unknown Memory

| Field | Value |
|-------|-------|
| **Flag** | `TEST_REGION_CLAIM_UNKNOWN` |
| **Source** | `test_mem_region.c` : `test_region_claim_unknown()` |
| **Group** | "Region Claim Unknown Memory" |

**What it tests:**
Uses custom table matching test pools. Claims address outside all regions,
NULL pointer, and zero size - all rejected.

**Assertions:**
1. Claim outside all regions rejected
2. NULL pointer rejected
3. Zero size rejected

---

### 56. Claim Table Full

| Field | Value |
|-------|-------|
| **Flag** | `TEST_REGION_CLAIM_FULL` |
| **Source** | `test_mem_region.c` : `test_region_claim_full()` |
| **Group** | "Region Claim Table Full" |

**What it tests:**
Fills all `TIKU_REGION_MAX_CLAIMS` slots with 8-byte non-overlapping ranges.
One more claim returns `ERR_FULL`.

**Assertions:**
1. All `TIKU_REGION_MAX_CLAIMS` claims succeed
2. Claim beyond max returns `ERR_FULL`

---

### 57. Get Type Found

| Field | Value |
|-------|-------|
| **Flag** | `TEST_REGION_GET_TYPE` |
| **Source** | `test_mem_region.c` : `test_region_get_type_found()` |
| **Group** | "Region Get Type" |

**What it tests:**
Looks up type for addresses in SRAM and NVM regions, at region base, and at
region end. All should return the correct type.

**Assertions:**
1. SRAM address returns SRAM type
2. NVM address returns NVM type
3. Address at region base returns correct type
4. Address at region end returns correct type

---

### 58. Get Type Not Found

| Field | Value |
|-------|-------|
| **Flag** | `TEST_REGION_NOT_FOUND` |
| **Source** | `test_mem_region.c` : `test_region_get_type_not_found()` |
| **Group** | "Region Get Type Not Found" |

**What it tests:**
Uses custom table matching test pools. Address past pool end, NULL pointer,
NULL output - all return NOT_FOUND.

**Assertions:**
1. Address outside regions returns NOT_FOUND
2. NULL pointer returns NOT_FOUND
3. NULL output returns NOT_FOUND

---

## Requests

<!--
  Add new test requests below. Specify which component (arena, pool, persist,
  mpu, region) the test is for. Each request should describe:
  - What behavior to test
  - Expected outcome
  - Any specific parameters or edge cases

  An LLM will read these requests and generate the corresponding C test
  code in the appropriate source file under tests/memory/. After
  implementation, move the request to the relevant Tests section above
  with full documentation.

  Example format:

  ### Request: [Arena] Concurrent arena usage from ISR context
  Allocate from an arena inside a timer ISR callback while the main loop
  is also allocating from the same arena. Verify that either: (a) allocations
  are serialized correctly, or (b) the API documents that ISR-context usage
  is unsupported and returns an error. Expected: defined behavior, no
  corruption.
-->
