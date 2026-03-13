/*
 * Tiku Operating System
 * http://tiku-os.org
 *
 * Authors: Ambuj Varshney <ambuj@tiku-os.org>
 *
 * test_process_current_cleared.c - Current process cleared after dispatch test
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

#if TEST_PROCESS_CURRENT_CLEARED

static volatile unsigned int cc_inside_ok = 0;

TIKU_PROCESS(test_cc_proc, "cc_proc");

TIKU_PROCESS_THREAD(test_cc_proc, ev, data)
{
    TIKU_PROCESS_BEGIN();

    /* Verify TIKU_THIS() is valid inside the thread */
    if (TIKU_THIS() == &test_cc_proc) {
        cc_inside_ok = 1;
    }

    TIKU_PROCESS_WAIT_EVENT();

    TIKU_PROCESS_END();
}

void test_process_current_cleared(void)
{
    TEST_GROUP_BEGIN("Current Process Cleared After Dispatch");

    cc_inside_ok = 0;

    tiku_process_init();
    tiku_process_start(&test_cc_proc, NULL);

    /* Drain INIT — thread runs, sets cc_inside_ok */
    while (tiku_process_run()) {
        /* drain */
    }

    /* Inside the thread, TIKU_THIS() should have been &test_cc_proc */
    TEST_ASSERT(cc_inside_ok == 1, "TIKU_THIS() valid inside process thread");

    /* After dispatch returns, TIKU_THIS() must be NULL */
    TEST_ASSERT(TIKU_THIS() == NULL, "TIKU_THIS() is NULL after dispatch");

    /* Clean up */
    tiku_process_exit(&test_cc_proc);
    TEST_GROUP_END("Current Process Cleared After Dispatch");
}

#endif /* TEST_PROCESS_CURRENT_CLEARED */

#endif /* PLATFORM_MSP430 */
