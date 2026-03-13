/*
 * Tiku Operating System
 * http://tiku-os.org
 *
 * Authors: Ambuj Varshney <ambuj@tiku-os.org>
 *
 * test_process_local.c - Process local storage test
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "test_process.h"

#ifdef PLATFORM_MSP430

#if TEST_PROCESS_LOCAL

/** Local state for the TIKU_PROCESS_WITH_LOCAL test */
struct local_test_state {
    uint16_t counter;
    uint8_t  phase;
};

TIKU_PROCESS_WITH_LOCAL(test_local_proc, "test_local",
                        struct local_test_state);

TIKU_PROCESS_THREAD(test_local_proc, ev, data)
{
    struct local_test_state *s = TIKU_LOCAL(struct local_test_state);

    TIKU_PROCESS_BEGIN();

    /* Phase 1: initialize local storage */
    s->counter = 10;
    s->phase   = 1;
    TEST_PRINTF("Local proc: set counter=%d phase=%d\n",
                 s->counter, s->phase);
    TIKU_PROCESS_YIELD();

    /* Phase 2: verify state survived the yield and mutate */
    s->counter += 5;
    s->phase    = 2;
    TEST_PRINTF("Local proc: counter=%d phase=%d\n",
                 s->counter, s->phase);
    TIKU_PROCESS_YIELD();

    /* Phase 3: final check */
    s->counter += 1;
    s->phase    = 3;
    TEST_PRINTF("Local proc: counter=%d phase=%d\n",
                 s->counter, s->phase);

    TIKU_PROCESS_END();
}

/** Local state for the TIKU_PROCESS_TYPED test */
struct typed_test_state {
    uint16_t value;
};

TIKU_PROCESS_TYPED(test_typed_proc, "test_typed",
                   struct typed_test_state);

TIKU_PROCESS_THREAD(test_typed_proc, ev, data)
{
    struct typed_test_state *s = test_typed_proc_local();

    TIKU_PROCESS_BEGIN();

    s->value = 42;
    TEST_PRINTF("Typed proc: value=%d\n", s->value);
    TIKU_PROCESS_YIELD();

    s->value += 8;
    TEST_PRINTF("Typed proc: value=%d\n", s->value);

    TIKU_PROCESS_END();
}

void test_process_local_storage(void)
{
    struct local_test_state *ls;
    struct typed_test_state *ts;

    TEST_GROUP_BEGIN("Process Local Storage");

    tiku_process_init();

    /*---------------------------------------------------------------*/
    /* Part A: TIKU_PROCESS_WITH_LOCAL + TIKU_LOCAL accessor          */
    /*---------------------------------------------------------------*/

    tiku_process_start(&test_local_proc, NULL);

    /* Drain INIT -> executes phase 1, yields */
    while (tiku_process_run()) {
        /* drain */
    }

    ls = (struct local_test_state *)test_local_proc.local;

    TEST_ASSERT(ls->counter == 10 && ls->phase == 1, "Local state correct after phase 1");

    /* Resume -> phase 2 */
    tiku_process_post(&test_local_proc, TIKU_EVENT_CONTINUE, NULL);
    while (tiku_process_run()) {
        /* drain */
    }

    TEST_ASSERT(ls->counter == 15 && ls->phase == 2, "Local state survived yield (phase 2)");

    /* Resume -> phase 3, process ends */
    tiku_process_post(&test_local_proc, TIKU_EVENT_CONTINUE, NULL);
    while (tiku_process_run()) {
        /* drain */
    }

    TEST_ASSERT(ls->counter == 16 && ls->phase == 3, "Local state correct at exit (phase 3)");

    TEST_ASSERT(!test_local_proc.is_running, "Process exited normally");

    /*---------------------------------------------------------------*/
    /* Part B: TIKU_PROCESS_TYPED accessor                           */
    /*---------------------------------------------------------------*/

    tiku_process_start(&test_typed_proc, NULL);

    /* Drain INIT -> sets value=42, yields */
    while (tiku_process_run()) {
        /* drain */
    }

    ts = test_typed_proc_local();

    TEST_ASSERT(ts->value == 42, "Typed accessor returns correct value");

    /* Resume -> value=50, process ends */
    tiku_process_post(&test_typed_proc, TIKU_EVENT_CONTINUE, NULL);
    while (tiku_process_run()) {
        /* drain */
    }

    TEST_ASSERT(ts->value == 50, "Typed state survived yield");

    TEST_ASSERT(!test_typed_proc.is_running, "Typed process exited normally");

    /*---------------------------------------------------------------*/
    /* Part C: Plain TIKU_PROCESS has local == NULL                   */
    /*---------------------------------------------------------------*/

    TEST_ASSERT(test_local_proc.local != NULL, "WITH_LOCAL process has non-NULL local");

    TEST_GROUP_END("Process Local Storage");
}

#endif /* TEST_PROCESS_LOCAL */

#endif /* PLATFORM_MSP430 */
