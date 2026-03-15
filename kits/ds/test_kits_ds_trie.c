/*
 * Tiku Operating System
 * http://tiku-os.org
 *
 * Authors: Ambuj Varshney <ambuj@tiku-os.org>
 *
 * test_kits_ds_trie.c - Trie data structure tests
 *
 * Uses short (1-2 byte) keys to stay within the default 32-node pool.
 * The trie is declared static to avoid stack overflow on MSP430
 * (each node is 37 bytes, so the full struct is ~1.2 KB).
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

#include <string.h>
#include "tests/tiku_test_harness.h"
#include "tikukits/ds/trie/tiku_kits_ds_trie.h"

/*---------------------------------------------------------------------------*/
/* SHARED INSTANCE — single static trie to avoid BSS bloat on MSP430        */
/* (32 nodes * 37 bytes = ~1.2 KB; one copy, not nine)                      */
/*---------------------------------------------------------------------------*/

static struct tiku_kits_ds_trie tr;

/*---------------------------------------------------------------------------*/
/* TEST 1: INITIALIZATION                                                    */
/*---------------------------------------------------------------------------*/

void test_kits_ds_trie_init(void)
{
    int rc;

    TEST_GROUP_BEGIN("Trie Init");

    rc = tiku_kits_ds_trie_init(&tr);
    TEST_ASSERT(rc == TIKU_KITS_DS_OK, "init returns OK");
    TEST_ASSERT(tiku_kits_ds_trie_count(&tr) == 0,
                "count is 0 after init");

    TEST_GROUP_END("Trie Init");
}

/*---------------------------------------------------------------------------*/
/* TEST 2: INSERT AND SEARCH                                                 */
/*---------------------------------------------------------------------------*/

void test_kits_ds_trie_insert_search(void)
{
    tiku_kits_ds_elem_t val;
    int rc;

    /* Short 2-byte keys: "AB" and "CD" */
    const uint8_t k1[] = { 0x41, 0x42 };  /* "AB" */
    const uint8_t k2[] = { 0x43, 0x44 };  /* "CD" */

    TEST_GROUP_BEGIN("Trie Insert/Search");

    tiku_kits_ds_trie_init(&tr);

    rc = tiku_kits_ds_trie_insert(&tr, k1, 2, 42);
    TEST_ASSERT(rc == TIKU_KITS_DS_OK, "insert AB OK");

    rc = tiku_kits_ds_trie_insert(&tr, k2, 2, 99);
    TEST_ASSERT(rc == TIKU_KITS_DS_OK, "insert CD OK");

    TEST_ASSERT(tiku_kits_ds_trie_count(&tr) == 2,
                "count is 2 after two inserts");

    rc = tiku_kits_ds_trie_search(&tr, k1, 2, &val);
    TEST_ASSERT(rc == TIKU_KITS_DS_OK, "search AB OK");
    TEST_ASSERT(val == 42, "AB value is 42");

    rc = tiku_kits_ds_trie_search(&tr, k2, 2, &val);
    TEST_ASSERT(rc == TIKU_KITS_DS_OK, "search CD OK");
    TEST_ASSERT(val == 99, "CD value is 99");

    TEST_GROUP_END("Trie Insert/Search");
}

/*---------------------------------------------------------------------------*/
/* TEST 3: SEARCH ABSENT KEY                                                 */
/*---------------------------------------------------------------------------*/

void test_kits_ds_trie_search_absent(void)
{
    tiku_kits_ds_elem_t val;
    int rc;

    const uint8_t k1[] = { 0x10, 0x20 };
    const uint8_t missing[] = { 0x30, 0x40 };
    const uint8_t prefix[] = { 0x10 };

    TEST_GROUP_BEGIN("Trie Search Absent");

    tiku_kits_ds_trie_init(&tr);

    /* Search empty trie */
    rc = tiku_kits_ds_trie_search(&tr, missing, 2, &val);
    TEST_ASSERT(rc == TIKU_KITS_DS_ERR_NOTFOUND,
                "search empty trie returns ERR_NOTFOUND");

    /* Insert one key, search for different key */
    tiku_kits_ds_trie_insert(&tr, k1, 2, 10);

    rc = tiku_kits_ds_trie_search(&tr, missing, 2, &val);
    TEST_ASSERT(rc == TIKU_KITS_DS_ERR_NOTFOUND,
                "search missing key returns ERR_NOTFOUND");

    /* Search for prefix of existing key */
    rc = tiku_kits_ds_trie_search(&tr, prefix, 1, &val);
    TEST_ASSERT(rc == TIKU_KITS_DS_ERR_NOTFOUND,
                "search prefix returns ERR_NOTFOUND");

    TEST_GROUP_END("Trie Search Absent");
}

/*---------------------------------------------------------------------------*/
/* TEST 4: CONTAINS                                                          */
/*---------------------------------------------------------------------------*/

void test_kits_ds_trie_contains(void)
{


    const uint8_t k1[] = { 0xAA };
    const uint8_t k2[] = { 0xBB };
    const uint8_t absent[] = { 0xCC };

    TEST_GROUP_BEGIN("Trie Contains");

    tiku_kits_ds_trie_init(&tr);

    tiku_kits_ds_trie_insert(&tr, k1, 1, 1);
    tiku_kits_ds_trie_insert(&tr, k2, 1, 2);

    TEST_ASSERT(tiku_kits_ds_trie_contains(&tr, k1, 1) == 1,
                "contains AA is true");
    TEST_ASSERT(tiku_kits_ds_trie_contains(&tr, k2, 1) == 1,
                "contains BB is true");
    TEST_ASSERT(tiku_kits_ds_trie_contains(&tr, absent, 1) == 0,
                "contains CC is false");

    /* NULL trie */
    TEST_ASSERT(tiku_kits_ds_trie_contains(NULL, k1, 1) == 0,
                "contains NULL trie returns 0");

    TEST_GROUP_END("Trie Contains");
}

/*---------------------------------------------------------------------------*/
/* TEST 5: MULTIPLE KEYS                                                     */
/*---------------------------------------------------------------------------*/

void test_kits_ds_trie_multiple_keys(void)
{
    tiku_kits_ds_elem_t val;
    int rc;

    /* Four 1-byte keys — uses ~9 nodes (root + 2 nibbles each,
       with prefix sharing on common high nibbles) */
    const uint8_t k1[] = { 0x11 };
    const uint8_t k2[] = { 0x22 };
    const uint8_t k3[] = { 0x33 };
    const uint8_t k4[] = { 0x44 };

    TEST_GROUP_BEGIN("Trie Multiple Keys");

    tiku_kits_ds_trie_init(&tr);

    rc = tiku_kits_ds_trie_insert(&tr, k1, 1, 10);
    TEST_ASSERT(rc == TIKU_KITS_DS_OK, "insert 0x11 OK");

    rc = tiku_kits_ds_trie_insert(&tr, k2, 1, 20);
    TEST_ASSERT(rc == TIKU_KITS_DS_OK, "insert 0x22 OK");

    rc = tiku_kits_ds_trie_insert(&tr, k3, 1, 30);
    TEST_ASSERT(rc == TIKU_KITS_DS_OK, "insert 0x33 OK");

    rc = tiku_kits_ds_trie_insert(&tr, k4, 1, 40);
    TEST_ASSERT(rc == TIKU_KITS_DS_OK, "insert 0x44 OK");

    TEST_ASSERT(tiku_kits_ds_trie_count(&tr) == 4,
                "count is 4 after four inserts");

    rc = tiku_kits_ds_trie_search(&tr, k1, 1, &val);
    TEST_ASSERT(rc == TIKU_KITS_DS_OK && val == 10,
                "0x11 value is 10");

    rc = tiku_kits_ds_trie_search(&tr, k2, 1, &val);
    TEST_ASSERT(rc == TIKU_KITS_DS_OK && val == 20,
                "0x22 value is 20");

    rc = tiku_kits_ds_trie_search(&tr, k3, 1, &val);
    TEST_ASSERT(rc == TIKU_KITS_DS_OK && val == 30,
                "0x33 value is 30");

    rc = tiku_kits_ds_trie_search(&tr, k4, 1, &val);
    TEST_ASSERT(rc == TIKU_KITS_DS_OK && val == 40,
                "0x44 value is 40");

    TEST_GROUP_END("Trie Multiple Keys");
}

/*---------------------------------------------------------------------------*/
/* TEST 6: REMOVE                                                            */
/*---------------------------------------------------------------------------*/

void test_kits_ds_trie_remove(void)
{
    tiku_kits_ds_elem_t val;
    int rc;

    const uint8_t k1[] = { 0x0A };
    const uint8_t k2[] = { 0x0B };
    const uint8_t k3[] = { 0x0C };
    const uint8_t missing[] = { 0xFF };

    TEST_GROUP_BEGIN("Trie Remove");

    tiku_kits_ds_trie_init(&tr);

    tiku_kits_ds_trie_insert(&tr, k1, 1, 1);
    tiku_kits_ds_trie_insert(&tr, k2, 1, 2);
    tiku_kits_ds_trie_insert(&tr, k3, 1, 3);

    /* Remove middle key */
    rc = tiku_kits_ds_trie_remove(&tr, k2, 1);
    TEST_ASSERT(rc == TIKU_KITS_DS_OK, "remove 0x0B OK");
    TEST_ASSERT(tiku_kits_ds_trie_count(&tr) == 2,
                "count is 2 after remove");

    /* Removed key no longer found */
    rc = tiku_kits_ds_trie_search(&tr, k2, 1, &val);
    TEST_ASSERT(rc == TIKU_KITS_DS_ERR_NOTFOUND,
                "removed key not found");

    TEST_ASSERT(tiku_kits_ds_trie_contains(&tr, k2, 1) == 0,
                "contains removed key is false");

    /* Other keys still accessible */
    rc = tiku_kits_ds_trie_search(&tr, k1, 1, &val);
    TEST_ASSERT(rc == TIKU_KITS_DS_OK, "key 0x0A still OK");
    TEST_ASSERT(val == 1, "key 0x0A value intact");

    rc = tiku_kits_ds_trie_search(&tr, k3, 1, &val);
    TEST_ASSERT(rc == TIKU_KITS_DS_OK, "key 0x0C still OK");
    TEST_ASSERT(val == 3, "key 0x0C value intact");

    /* Remove non-existent key */
    rc = tiku_kits_ds_trie_remove(&tr, missing, 1);
    TEST_ASSERT(rc == TIKU_KITS_DS_ERR_NOTFOUND,
                "remove missing key returns ERR_NOTFOUND");

    TEST_GROUP_END("Trie Remove");
}

/*---------------------------------------------------------------------------*/
/* TEST 7: OVERWRITE EXISTING KEY                                            */
/*---------------------------------------------------------------------------*/

void test_kits_ds_trie_overwrite(void)
{
    tiku_kits_ds_elem_t val;
    int rc;

    const uint8_t key[] = { 0x55 };

    TEST_GROUP_BEGIN("Trie Overwrite");

    tiku_kits_ds_trie_init(&tr);

    rc = tiku_kits_ds_trie_insert(&tr, key, 1, 100);
    TEST_ASSERT(rc == TIKU_KITS_DS_OK, "first insert OK");
    TEST_ASSERT(tiku_kits_ds_trie_count(&tr) == 1,
                "count is 1 after insert");

    rc = tiku_kits_ds_trie_insert(&tr, key, 1, 200);
    TEST_ASSERT(rc == TIKU_KITS_DS_OK, "overwrite returns OK");
    TEST_ASSERT(tiku_kits_ds_trie_count(&tr) == 1,
                "count still 1 after overwrite");

    rc = tiku_kits_ds_trie_search(&tr, key, 1, &val);
    TEST_ASSERT(rc == TIKU_KITS_DS_OK, "search after overwrite OK");
    TEST_ASSERT(val == 200, "value updated to 200");

    /* Overwrite again */
    tiku_kits_ds_trie_insert(&tr, key, 1, -50);
    tiku_kits_ds_trie_search(&tr, key, 1, &val);
    TEST_ASSERT(val == -50, "value updated to -50");
    TEST_ASSERT(tiku_kits_ds_trie_count(&tr) == 1,
                "count still 1");

    TEST_GROUP_END("Trie Overwrite");
}

/*---------------------------------------------------------------------------*/
/* TEST 8: CLEAR AND RESET                                                   */
/*---------------------------------------------------------------------------*/

void test_kits_ds_trie_clear_reset(void)
{
    tiku_kits_ds_elem_t val;
    int rc;

    const uint8_t k1[] = { 0x01 };
    const uint8_t k2[] = { 0x02 };
    const uint8_t k3[] = { 0x03 };

    TEST_GROUP_BEGIN("Trie Clear/Reset");

    tiku_kits_ds_trie_init(&tr);

    tiku_kits_ds_trie_insert(&tr, k1, 1, 100);
    tiku_kits_ds_trie_insert(&tr, k2, 1, 200);
    tiku_kits_ds_trie_insert(&tr, k3, 1, 300);
    TEST_ASSERT(tiku_kits_ds_trie_count(&tr) == 3,
                "count is 3 before clear");

    rc = tiku_kits_ds_trie_clear(&tr);
    TEST_ASSERT(rc == TIKU_KITS_DS_OK, "clear returns OK");
    TEST_ASSERT(tiku_kits_ds_trie_count(&tr) == 0,
                "count is 0 after clear");

    /* Previous keys no longer found */
    rc = tiku_kits_ds_trie_search(&tr, k1, 1, &val);
    TEST_ASSERT(rc == TIKU_KITS_DS_ERR_NOTFOUND,
                "k1 not found after clear");

    TEST_ASSERT(tiku_kits_ds_trie_contains(&tr, k2, 1) == 0,
                "k2 not contained after clear");

    /* Can re-use trie after clear */
    const uint8_t knew[] = { 0xEE };
    rc = tiku_kits_ds_trie_insert(&tr, knew, 1, 999);
    TEST_ASSERT(rc == TIKU_KITS_DS_OK,
                "insert after clear OK");
    tiku_kits_ds_trie_search(&tr, knew, 1, &val);
    TEST_ASSERT(val == 999, "value after clear correct");

    TEST_GROUP_END("Trie Clear/Reset");
}

/*---------------------------------------------------------------------------*/
/* TEST 9: NULL POINTER INPUTS                                               */
/*---------------------------------------------------------------------------*/

void test_kits_ds_trie_null_inputs(void)
{
    tiku_kits_ds_elem_t val;
    int rc;

    const uint8_t key[] = { 0xAB };

    TEST_GROUP_BEGIN("Trie NULL Inputs");

    tiku_kits_ds_trie_init(&tr);

    rc = tiku_kits_ds_trie_init(NULL);
    TEST_ASSERT(rc == TIKU_KITS_DS_ERR_NULL,
                "init NULL rejected");

    rc = tiku_kits_ds_trie_insert(NULL, key, 1, 10);
    TEST_ASSERT(rc == TIKU_KITS_DS_ERR_NULL,
                "insert NULL trie rejected");

    rc = tiku_kits_ds_trie_insert(&tr, NULL, 1, 10);
    TEST_ASSERT(rc == TIKU_KITS_DS_ERR_NULL,
                "insert NULL key rejected");

    rc = tiku_kits_ds_trie_search(NULL, key, 1, &val);
    TEST_ASSERT(rc == TIKU_KITS_DS_ERR_NULL,
                "search NULL trie rejected");

    rc = tiku_kits_ds_trie_search(&tr, NULL, 1, &val);
    TEST_ASSERT(rc == TIKU_KITS_DS_ERR_NULL,
                "search NULL key rejected");

    rc = tiku_kits_ds_trie_search(&tr, key, 1, NULL);
    TEST_ASSERT(rc == TIKU_KITS_DS_ERR_NULL,
                "search NULL value rejected");

    rc = tiku_kits_ds_trie_remove(NULL, key, 1);
    TEST_ASSERT(rc == TIKU_KITS_DS_ERR_NULL,
                "remove NULL trie rejected");

    rc = tiku_kits_ds_trie_remove(&tr, NULL, 1);
    TEST_ASSERT(rc == TIKU_KITS_DS_ERR_NULL,
                "remove NULL key rejected");

    rc = tiku_kits_ds_trie_clear(NULL);
    TEST_ASSERT(rc == TIKU_KITS_DS_ERR_NULL,
                "clear NULL rejected");

    TEST_ASSERT(tiku_kits_ds_trie_count(NULL) == 0,
                "count NULL returns 0");

    TEST_ASSERT(tiku_kits_ds_trie_contains(NULL, key, 1) == 0,
                "contains NULL trie returns 0");

    TEST_GROUP_END("Trie NULL Inputs");
}
