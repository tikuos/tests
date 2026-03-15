/*
 * Tiku Operating System
 * http://tiku-os.org
 *
 * Authors: Ambuj Varshney <ambuj@tiku-os.org>
 *
 * test_kits_ds_deque.c - Double-ended queue data structure tests
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

#include "tests/tiku_test_harness.h"
#include "tikukits/ds/deque/tiku_kits_ds_deque.h"

/*---------------------------------------------------------------------------*/
/* TEST 1: INITIALIZATION                                                    */
/*---------------------------------------------------------------------------*/

void test_kits_ds_deque_init(void)
{
    struct tiku_kits_ds_deque dq;
    int rc;

    TEST_GROUP_BEGIN("Deque Init");

    rc = tiku_kits_ds_deque_init(&dq, 8);
    TEST_ASSERT(rc == TIKU_KITS_DS_OK, "init capacity 8 returns OK");
    TEST_ASSERT(tiku_kits_ds_deque_count(&dq) == 0,
                "count is 0 after init");
    TEST_ASSERT(tiku_kits_ds_deque_empty(&dq) == 1,
                "deque is empty after init");
    TEST_ASSERT(tiku_kits_ds_deque_full(&dq) == 0,
                "deque is not full after init");

    /* Max capacity accepted */
    rc = tiku_kits_ds_deque_init(&dq, TIKU_KITS_DS_DEQUE_MAX_SIZE);
    TEST_ASSERT(rc == TIKU_KITS_DS_OK, "init max capacity returns OK");

    /* Zero capacity rejected */
    rc = tiku_kits_ds_deque_init(&dq, 0);
    TEST_ASSERT(rc == TIKU_KITS_DS_ERR_PARAM,
                "zero capacity rejected");

    /* Oversized capacity rejected */
    rc = tiku_kits_ds_deque_init(&dq,
                                  TIKU_KITS_DS_DEQUE_MAX_SIZE + 1);
    TEST_ASSERT(rc == TIKU_KITS_DS_ERR_PARAM,
                "oversized capacity rejected");

    TEST_GROUP_END("Deque Init");
}

/*---------------------------------------------------------------------------*/
/* TEST 2: PUSH FRONT / POP FRONT (LIFO FROM FRONT)                         */
/*---------------------------------------------------------------------------*/

void test_kits_ds_deque_push_pop_front(void)
{
    struct tiku_kits_ds_deque dq;
    tiku_kits_ds_elem_t val;
    int rc;

    TEST_GROUP_BEGIN("Deque Push/Pop Front");

    tiku_kits_ds_deque_init(&dq, 8);

    /* Push three elements to front */
    rc = tiku_kits_ds_deque_push_front(&dq, 10);
    TEST_ASSERT(rc == TIKU_KITS_DS_OK, "push_front 10 OK");
    rc = tiku_kits_ds_deque_push_front(&dq, 20);
    TEST_ASSERT(rc == TIKU_KITS_DS_OK, "push_front 20 OK");
    rc = tiku_kits_ds_deque_push_front(&dq, 30);
    TEST_ASSERT(rc == TIKU_KITS_DS_OK, "push_front 30 OK");
    TEST_ASSERT(tiku_kits_ds_deque_count(&dq) == 3,
                "count is 3 after 3 push_fronts");

    /* Pop front returns LIFO order: 30, 20, 10 */
    rc = tiku_kits_ds_deque_pop_front(&dq, &val);
    TEST_ASSERT(rc == TIKU_KITS_DS_OK, "pop_front OK");
    TEST_ASSERT(val == 30, "first pop_front returns 30");

    rc = tiku_kits_ds_deque_pop_front(&dq, &val);
    TEST_ASSERT(rc == TIKU_KITS_DS_OK, "second pop_front OK");
    TEST_ASSERT(val == 20, "second pop_front returns 20");

    rc = tiku_kits_ds_deque_pop_front(&dq, &val);
    TEST_ASSERT(rc == TIKU_KITS_DS_OK, "third pop_front OK");
    TEST_ASSERT(val == 10, "third pop_front returns 10");

    TEST_ASSERT(tiku_kits_ds_deque_count(&dq) == 0,
                "count is 0 after all pops");

    /* Pop from empty deque */
    rc = tiku_kits_ds_deque_pop_front(&dq, &val);
    TEST_ASSERT(rc == TIKU_KITS_DS_ERR_EMPTY,
                "pop_front empty returns ERR_EMPTY");

    TEST_GROUP_END("Deque Push/Pop Front");
}

/*---------------------------------------------------------------------------*/
/* TEST 3: PUSH BACK / POP BACK                                             */
/*---------------------------------------------------------------------------*/

void test_kits_ds_deque_push_pop_back(void)
{
    struct tiku_kits_ds_deque dq;
    tiku_kits_ds_elem_t val;
    int rc;

    TEST_GROUP_BEGIN("Deque Push/Pop Back");

    tiku_kits_ds_deque_init(&dq, 8);

    /* Push three elements to back */
    rc = tiku_kits_ds_deque_push_back(&dq, 10);
    TEST_ASSERT(rc == TIKU_KITS_DS_OK, "push_back 10 OK");
    rc = tiku_kits_ds_deque_push_back(&dq, 20);
    TEST_ASSERT(rc == TIKU_KITS_DS_OK, "push_back 20 OK");
    rc = tiku_kits_ds_deque_push_back(&dq, 30);
    TEST_ASSERT(rc == TIKU_KITS_DS_OK, "push_back 30 OK");
    TEST_ASSERT(tiku_kits_ds_deque_count(&dq) == 3,
                "count is 3 after 3 push_backs");

    /* Pop back returns LIFO order: 30, 20, 10 */
    rc = tiku_kits_ds_deque_pop_back(&dq, &val);
    TEST_ASSERT(rc == TIKU_KITS_DS_OK, "pop_back OK");
    TEST_ASSERT(val == 30, "first pop_back returns 30");

    rc = tiku_kits_ds_deque_pop_back(&dq, &val);
    TEST_ASSERT(rc == TIKU_KITS_DS_OK, "second pop_back OK");
    TEST_ASSERT(val == 20, "second pop_back returns 20");

    rc = tiku_kits_ds_deque_pop_back(&dq, &val);
    TEST_ASSERT(rc == TIKU_KITS_DS_OK, "third pop_back OK");
    TEST_ASSERT(val == 10, "third pop_back returns 10");

    TEST_ASSERT(tiku_kits_ds_deque_count(&dq) == 0,
                "count is 0 after all pops");

    /* Pop from empty deque */
    rc = tiku_kits_ds_deque_pop_back(&dq, &val);
    TEST_ASSERT(rc == TIKU_KITS_DS_ERR_EMPTY,
                "pop_back empty returns ERR_EMPTY");

    TEST_GROUP_END("Deque Push/Pop Back");
}

/*---------------------------------------------------------------------------*/
/* TEST 4: MIXED OPERATIONS (PUSH/POP FROM BOTH ENDS)                       */
/*---------------------------------------------------------------------------*/

void test_kits_ds_deque_mixed_ops(void)
{
    struct tiku_kits_ds_deque dq;
    tiku_kits_ds_elem_t val;

    TEST_GROUP_BEGIN("Deque Mixed Ops");

    tiku_kits_ds_deque_init(&dq, 8);

    /* Push to both ends: front=[30,20,10] back=[40,50,60]
     * Logical order: 30, 20, 10, 40, 50, 60 */
    tiku_kits_ds_deque_push_back(&dq, 10);
    tiku_kits_ds_deque_push_back(&dq, 40);
    tiku_kits_ds_deque_push_front(&dq, 20);
    tiku_kits_ds_deque_push_back(&dq, 50);
    tiku_kits_ds_deque_push_front(&dq, 30);
    tiku_kits_ds_deque_push_back(&dq, 60);

    TEST_ASSERT(tiku_kits_ds_deque_count(&dq) == 6,
                "count is 6 after mixed pushes");

    /* Pop from front: should get 30 */
    tiku_kits_ds_deque_pop_front(&dq, &val);
    TEST_ASSERT(val == 30, "pop_front returns 30");

    /* Pop from back: should get 60 */
    tiku_kits_ds_deque_pop_back(&dq, &val);
    TEST_ASSERT(val == 60, "pop_back returns 60");

    /* Pop from front: should get 20 */
    tiku_kits_ds_deque_pop_front(&dq, &val);
    TEST_ASSERT(val == 20, "pop_front returns 20");

    /* Pop from back: should get 50 */
    tiku_kits_ds_deque_pop_back(&dq, &val);
    TEST_ASSERT(val == 50, "pop_back returns 50");

    /* Remaining: 10, 40 */
    tiku_kits_ds_deque_pop_front(&dq, &val);
    TEST_ASSERT(val == 10, "pop_front returns 10");

    tiku_kits_ds_deque_pop_back(&dq, &val);
    TEST_ASSERT(val == 40, "pop_back returns 40");

    TEST_ASSERT(tiku_kits_ds_deque_empty(&dq) == 1,
                "deque empty after all pops");

    TEST_GROUP_END("Deque Mixed Ops");
}

/*---------------------------------------------------------------------------*/
/* TEST 5: PEEK FRONT AND BACK                                              */
/*---------------------------------------------------------------------------*/

void test_kits_ds_deque_peek(void)
{
    struct tiku_kits_ds_deque dq;
    tiku_kits_ds_elem_t val;
    int rc;

    TEST_GROUP_BEGIN("Deque Peek");

    tiku_kits_ds_deque_init(&dq, 4);

    /* Peek on empty deque */
    rc = tiku_kits_ds_deque_peek_front(&dq, &val);
    TEST_ASSERT(rc == TIKU_KITS_DS_ERR_EMPTY,
                "peek_front empty returns ERR_EMPTY");
    rc = tiku_kits_ds_deque_peek_back(&dq, &val);
    TEST_ASSERT(rc == TIKU_KITS_DS_ERR_EMPTY,
                "peek_back empty returns ERR_EMPTY");

    /* Push one element and peek both ends */
    tiku_kits_ds_deque_push_back(&dq, 42);
    rc = tiku_kits_ds_deque_peek_front(&dq, &val);
    TEST_ASSERT(rc == TIKU_KITS_DS_OK, "peek_front returns OK");
    TEST_ASSERT(val == 42, "peek_front value is 42");

    rc = tiku_kits_ds_deque_peek_back(&dq, &val);
    TEST_ASSERT(rc == TIKU_KITS_DS_OK, "peek_back returns OK");
    TEST_ASSERT(val == 42, "peek_back value is 42 (single element)");

    /* Peek does not remove element */
    TEST_ASSERT(tiku_kits_ds_deque_count(&dq) == 1,
                "count unchanged after peeks");

    /* Push more, verify peek sees correct ends */
    tiku_kits_ds_deque_push_front(&dq, 10);
    tiku_kits_ds_deque_push_back(&dq, 99);

    /* Logical order: 10, 42, 99 */
    tiku_kits_ds_deque_peek_front(&dq, &val);
    TEST_ASSERT(val == 10, "peek_front sees 10");

    tiku_kits_ds_deque_peek_back(&dq, &val);
    TEST_ASSERT(val == 99, "peek_back sees 99");

    TEST_GROUP_END("Deque Peek");
}

/*---------------------------------------------------------------------------*/
/* TEST 6: RANDOM ACCESS BY INDEX                                            */
/*---------------------------------------------------------------------------*/

void test_kits_ds_deque_random_access(void)
{
    struct tiku_kits_ds_deque dq;
    tiku_kits_ds_elem_t val;
    int rc;

    TEST_GROUP_BEGIN("Deque Random Access");

    tiku_kits_ds_deque_init(&dq, 8);

    /* Build deque: push_front(20), push_front(10), push_back(30), push_back(40)
     * Logical order: 10, 20, 30, 40 */
    tiku_kits_ds_deque_push_back(&dq, 20);
    tiku_kits_ds_deque_push_front(&dq, 10);
    tiku_kits_ds_deque_push_back(&dq, 30);
    tiku_kits_ds_deque_push_back(&dq, 40);

    /* Access by logical index */
    rc = tiku_kits_ds_deque_get(&dq, 0, &val);
    TEST_ASSERT(rc == TIKU_KITS_DS_OK, "get index 0 OK");
    TEST_ASSERT(val == 10, "index 0 is 10");

    rc = tiku_kits_ds_deque_get(&dq, 1, &val);
    TEST_ASSERT(rc == TIKU_KITS_DS_OK, "get index 1 OK");
    TEST_ASSERT(val == 20, "index 1 is 20");

    rc = tiku_kits_ds_deque_get(&dq, 2, &val);
    TEST_ASSERT(rc == TIKU_KITS_DS_OK, "get index 2 OK");
    TEST_ASSERT(val == 30, "index 2 is 30");

    rc = tiku_kits_ds_deque_get(&dq, 3, &val);
    TEST_ASSERT(rc == TIKU_KITS_DS_OK, "get index 3 OK");
    TEST_ASSERT(val == 40, "index 3 is 40");

    /* Out-of-bounds index rejected */
    rc = tiku_kits_ds_deque_get(&dq, 4, &val);
    TEST_ASSERT(rc == TIKU_KITS_DS_ERR_BOUNDS,
                "index 4 out of bounds rejected");

    /* Get does not modify the deque */
    TEST_ASSERT(tiku_kits_ds_deque_count(&dq) == 4,
                "count unchanged after get operations");

    TEST_GROUP_END("Deque Random Access");
}

/*---------------------------------------------------------------------------*/
/* TEST 7: FULL AND EMPTY STATES                                             */
/*---------------------------------------------------------------------------*/

void test_kits_ds_deque_full_empty(void)
{
    struct tiku_kits_ds_deque dq;
    tiku_kits_ds_elem_t val;
    int rc;
    uint16_t i;

    TEST_GROUP_BEGIN("Deque Full/Empty");

    tiku_kits_ds_deque_init(&dq, 4);

    TEST_ASSERT(tiku_kits_ds_deque_empty(&dq) == 1,
                "empty after init");
    TEST_ASSERT(tiku_kits_ds_deque_full(&dq) == 0,
                "not full after init");

    /* Fill to capacity using alternating ends */
    tiku_kits_ds_deque_push_back(&dq, 1);
    tiku_kits_ds_deque_push_front(&dq, 2);
    tiku_kits_ds_deque_push_back(&dq, 3);
    tiku_kits_ds_deque_push_front(&dq, 4);

    TEST_ASSERT(tiku_kits_ds_deque_full(&dq) == 1,
                "full at capacity");
    TEST_ASSERT(tiku_kits_ds_deque_empty(&dq) == 0,
                "not empty when full");
    TEST_ASSERT(tiku_kits_ds_deque_count(&dq) == 4,
                "count equals capacity");

    /* Push on full deque fails from both ends */
    rc = tiku_kits_ds_deque_push_front(&dq, 99);
    TEST_ASSERT(rc == TIKU_KITS_DS_ERR_FULL,
                "push_front on full returns ERR_FULL");
    rc = tiku_kits_ds_deque_push_back(&dq, 99);
    TEST_ASSERT(rc == TIKU_KITS_DS_ERR_FULL,
                "push_back on full returns ERR_FULL");

    /* Pop all elements to verify empty */
    for (i = 0; i < 4; i++) {
        rc = tiku_kits_ds_deque_pop_front(&dq, &val);
        TEST_ASSERT(rc == TIKU_KITS_DS_OK, "pop during drain OK");
    }

    TEST_ASSERT(tiku_kits_ds_deque_empty(&dq) == 1,
                "empty after draining all");
    TEST_ASSERT(tiku_kits_ds_deque_full(&dq) == 0,
                "not full after draining all");
    TEST_ASSERT(tiku_kits_ds_deque_count(&dq) == 0,
                "count is 0 after draining all");

    TEST_GROUP_END("Deque Full/Empty");
}

/*---------------------------------------------------------------------------*/
/* TEST 8: CLEAR AND RESET                                                   */
/*---------------------------------------------------------------------------*/

void test_kits_ds_deque_clear_reset(void)
{
    struct tiku_kits_ds_deque dq;
    tiku_kits_ds_elem_t val;
    int rc;

    TEST_GROUP_BEGIN("Deque Clear/Reset");

    tiku_kits_ds_deque_init(&dq, 8);

    /* Fill some elements */
    tiku_kits_ds_deque_push_back(&dq, 100);
    tiku_kits_ds_deque_push_front(&dq, 200);
    tiku_kits_ds_deque_push_back(&dq, 300);

    /* Clear resets the deque */
    rc = tiku_kits_ds_deque_clear(&dq);
    TEST_ASSERT(rc == TIKU_KITS_DS_OK, "clear returns OK");
    TEST_ASSERT(tiku_kits_ds_deque_count(&dq) == 0,
                "count is 0 after clear");
    TEST_ASSERT(tiku_kits_ds_deque_empty(&dq) == 1,
                "deque is empty after clear");

    /* Pop after clear should fail */
    rc = tiku_kits_ds_deque_pop_front(&dq, &val);
    TEST_ASSERT(rc == TIKU_KITS_DS_ERR_EMPTY,
                "pop_front after clear returns ERR_EMPTY");
    rc = tiku_kits_ds_deque_pop_back(&dq, &val);
    TEST_ASSERT(rc == TIKU_KITS_DS_ERR_EMPTY,
                "pop_back after clear returns ERR_EMPTY");

    /* Deque is usable again after clear */
    rc = tiku_kits_ds_deque_push_back(&dq, 999);
    TEST_ASSERT(rc == TIKU_KITS_DS_OK, "push_back after clear OK");
    rc = tiku_kits_ds_deque_push_front(&dq, 111);
    TEST_ASSERT(rc == TIKU_KITS_DS_OK, "push_front after clear OK");

    tiku_kits_ds_deque_pop_front(&dq, &val);
    TEST_ASSERT(val == 111, "front value after clear is 111");
    tiku_kits_ds_deque_pop_back(&dq, &val);
    TEST_ASSERT(val == 999, "back value after clear is 999");

    TEST_GROUP_END("Deque Clear/Reset");
}

/*---------------------------------------------------------------------------*/
/* TEST 9: NULL POINTER INPUTS                                               */
/*---------------------------------------------------------------------------*/

void test_kits_ds_deque_null_inputs(void)
{
    struct tiku_kits_ds_deque dq;
    tiku_kits_ds_elem_t val;
    int rc;

    TEST_GROUP_BEGIN("Deque NULL Inputs");

    /* init with NULL */
    rc = tiku_kits_ds_deque_init(NULL, 8);
    TEST_ASSERT(rc == TIKU_KITS_DS_ERR_NULL,
                "init NULL rejected");

    /* push_front with NULL deque */
    rc = tiku_kits_ds_deque_push_front(NULL, 42);
    TEST_ASSERT(rc == TIKU_KITS_DS_ERR_NULL,
                "push_front NULL rejected");

    /* push_back with NULL deque */
    rc = tiku_kits_ds_deque_push_back(NULL, 42);
    TEST_ASSERT(rc == TIKU_KITS_DS_ERR_NULL,
                "push_back NULL rejected");

    /* pop_front with NULL deque */
    rc = tiku_kits_ds_deque_pop_front(NULL, &val);
    TEST_ASSERT(rc == TIKU_KITS_DS_ERR_NULL,
                "pop_front NULL dq rejected");

    /* pop_front with NULL value */
    tiku_kits_ds_deque_init(&dq, 4);
    tiku_kits_ds_deque_push_back(&dq, 1);
    rc = tiku_kits_ds_deque_pop_front(&dq, NULL);
    TEST_ASSERT(rc == TIKU_KITS_DS_ERR_NULL,
                "pop_front NULL value rejected");

    /* pop_back with NULL deque */
    rc = tiku_kits_ds_deque_pop_back(NULL, &val);
    TEST_ASSERT(rc == TIKU_KITS_DS_ERR_NULL,
                "pop_back NULL dq rejected");

    /* pop_back with NULL value */
    rc = tiku_kits_ds_deque_pop_back(&dq, NULL);
    TEST_ASSERT(rc == TIKU_KITS_DS_ERR_NULL,
                "pop_back NULL value rejected");

    /* peek_front with NULL deque */
    rc = tiku_kits_ds_deque_peek_front(NULL, &val);
    TEST_ASSERT(rc == TIKU_KITS_DS_ERR_NULL,
                "peek_front NULL dq rejected");

    /* peek_front with NULL value */
    rc = tiku_kits_ds_deque_peek_front(&dq, NULL);
    TEST_ASSERT(rc == TIKU_KITS_DS_ERR_NULL,
                "peek_front NULL value rejected");

    /* peek_back with NULL deque */
    rc = tiku_kits_ds_deque_peek_back(NULL, &val);
    TEST_ASSERT(rc == TIKU_KITS_DS_ERR_NULL,
                "peek_back NULL dq rejected");

    /* peek_back with NULL value */
    rc = tiku_kits_ds_deque_peek_back(&dq, NULL);
    TEST_ASSERT(rc == TIKU_KITS_DS_ERR_NULL,
                "peek_back NULL value rejected");

    /* get with NULL deque */
    rc = tiku_kits_ds_deque_get(NULL, 0, &val);
    TEST_ASSERT(rc == TIKU_KITS_DS_ERR_NULL,
                "get NULL dq rejected");

    /* get with NULL value */
    rc = tiku_kits_ds_deque_get(&dq, 0, NULL);
    TEST_ASSERT(rc == TIKU_KITS_DS_ERR_NULL,
                "get NULL value rejected");

    /* clear with NULL */
    rc = tiku_kits_ds_deque_clear(NULL);
    TEST_ASSERT(rc == TIKU_KITS_DS_ERR_NULL,
                "clear NULL rejected");

    /* count with NULL returns 0 */
    TEST_ASSERT(tiku_kits_ds_deque_count(NULL) == 0,
                "count NULL returns 0");

    /* full with NULL returns 0 */
    TEST_ASSERT(tiku_kits_ds_deque_full(NULL) == 0,
                "full NULL returns 0");

    /* empty with NULL returns 0 */
    TEST_ASSERT(tiku_kits_ds_deque_empty(NULL) == 0,
                "empty NULL returns 0");

    TEST_GROUP_END("Deque NULL Inputs");
}
