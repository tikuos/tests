/*
 * Tiku Operating System
 * http://tiku-os.org
 *
 * Authors: Ambuj Varshney <ambuj@tiku-os.org>
 *
 * test_process_queue.c - Queue query functions test
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

#if TEST_PROCESS_QUEUE

TIKU_PROCESS(test_queue_proc, "test_queue");

TIKU_PROCESS_THREAD(test_queue_proc, ev, data)
{
    TIKU_PROCESS_BEGIN();

    while (1) {
        TIKU_PROCESS_WAIT_EVENT();
    }

    TIKU_PROCESS_END();
}

void test_process_queue_query(void)
{
    unsigned int i;
    uint8_t posted;

    TEST_GROUP_BEGIN("Queue Query Functions");

    /* Disable interrupts for the entire test so the timer ISR cannot
     * post events into the queue while we are checking its state. */
    tiku_atomic_enter();

    /* Fresh init — queue must be empty */
    tiku_process_init();

    /* --- Empty queue checks --- */
    TEST_ASSERT(tiku_process_queue_empty(), "Queue is empty after init");

    TEST_ASSERT(tiku_process_queue_length() == 0, "Queue length is 0 after init");

    TEST_ASSERT(tiku_process_queue_space() == TIKU_QUEUE_SIZE, "Queue space equals TIKU_QUEUE_SIZE after init");

    TEST_ASSERT(!tiku_process_queue_full(), "Queue is not full after init");

    /* --- Start process and post some events --- */
    tiku_process_start(&test_queue_proc, NULL);

    /* start posts an INIT event, so length should be 1 */
    TEST_ASSERT(tiku_process_queue_length() == 1, "Queue length is 1 after process start");

    TEST_ASSERT(!tiku_process_queue_empty(), "Queue is not empty after post");

    TEST_ASSERT(tiku_process_queue_space() == TIKU_QUEUE_SIZE - 1, "Queue space decremented after 1 post");

    /* Drain INIT */
    while (tiku_process_run()) {
        /* drain */
    }

    /* Queue should be empty again after draining */
    TEST_ASSERT(tiku_process_queue_empty(), "Queue empty after draining events");

    /* --- Fill the queue completely --- */
    for (i = 0; i < TIKU_QUEUE_SIZE; i++) {
        posted = tiku_process_post(&test_queue_proc,
                                    TIKU_EVENT_CONTINUE, NULL);
        TEST_ASSERT(posted, "Post event succeeds");
    }

    TEST_ASSERT(tiku_process_queue_full(), "Queue is full after filling");

    TEST_ASSERT(tiku_process_queue_length() == TIKU_QUEUE_SIZE, "Queue length equals TIKU_QUEUE_SIZE");

    TEST_ASSERT(tiku_process_queue_space() == 0, "Queue space is 0 when full");

    /* Post should fail when queue is full */
    posted = tiku_process_post(&test_queue_proc,
                                TIKU_EVENT_CONTINUE, NULL);
    TEST_ASSERT(!posted, "Post returns 0 when queue is full");

    /* --- Drain one event and verify counts update --- */
    tiku_process_run();

    TEST_ASSERT(tiku_process_queue_length() == TIKU_QUEUE_SIZE - 1, "Queue length decremented after one run");

    TEST_ASSERT(tiku_process_queue_space() == 1, "Queue space is 1 after draining one");

    TEST_ASSERT(!tiku_process_queue_full(), "Queue no longer full after drain");

    /* --- tiku_process_is_running --- */
    TEST_ASSERT(tiku_process_is_running(&test_queue_proc), "is_running returns 1 for active process");

    /* Drain remaining events and exit the process */
    while (tiku_process_run()) {
        /* drain */
    }
    tiku_process_exit(&test_queue_proc);

    TEST_ASSERT(!tiku_process_is_running(&test_queue_proc), "is_running returns 0 after exit");

    tiku_atomic_exit();

    TEST_GROUP_END("Queue Query Functions");
}

#endif /* TEST_PROCESS_QUEUE */

#endif /* PLATFORM_MSP430 */
