#include "defuser.h"

#include <inttypes.h>
#include <stdio.h>

/* Recovered for personal package f82a5b3ae1197c17b128d4eb. */
static int write_answer(char *out, size_t cap, const uint32_t *values, size_t count) {
    size_t used = 0;
    for (size_t i = 0; i < count; ++i) {
        int size = snprintf(out + used, cap - used, "%s%" PRIu32,
                            i ? " " : "", values[i]);
        if (size < 0 || (size_t)size >= cap - used) return 1;
        used += (size_t)size;
    }
    return count == 0;
}

int solve_phase_1(uint64_t challenge, char *out, size_t cap) {
    uint32_t lo = (uint32_t)challenge;
    uint32_t hi = (uint32_t)(challenge >> 32);
    uint32_t middle = (uint32_t)(challenge >> 16);
    uint32_t step1 = (middle + UINT32_C(0xf0d9)) ^ hi;
    uint32_t step0 = (lo + hi * 8u) ^ UINT32_C(0x9cfd);
    uint32_t values[] = {(step1 ^ UINT32_C(0xea2f)) + step0 * 8u};
    return write_answer(out, cap, values, 1);
}

int solve_phase_2(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[6];
    values[0] = (uint32_t)((challenge >> 20) & 63u);
    for (unsigned i = 1; i < 6; ++i) {
        uint32_t part = (uint32_t)((challenge >> (5u + 2u * i)) & 63u);
        values[i] = (values[i - 1] + part) ^ (10u * i);
    }
    return write_answer(out, cap, values, 6);
}

int solve_phase_3(uint64_t challenge, char *out, size_t cap) {
    uint32_t index = (uint32_t)challenge & 7u;
    uint32_t x = (uint32_t)(challenge >> 3) ^ (uint32_t)(challenge >> 32);
    uint32_t y;
    /* The jump table orders cases as 4,2,3,1,6,7,0,5. */
    switch (index) {
        case 0: y = x * 3u + UINT32_C(0xc0fd); break;
        case 1: y = x * 3u + UINT32_C(0xd802); break;
        case 2: y = x * 5u + UINT32_C(0x1402d); break;
        case 3: y = (x ^ UINT32_C(0x57b8)) + (x >> 1); break;
        case 4: y = (x * 4u) ^ UINT32_C(0x8e3c); break;
        case 5: y = x * 5u + UINT32_C(0x2ab6b); break;
        case 6: y = (x * 4u) ^ UINT32_C(0x999c); break;
        default: y = (x ^ UINT32_C(0xb2d5)) + (x >> 2); break;
    }
    uint32_t values[] = {index, y};
    return write_answer(out, cap, values, 2);
}

static uint32_t walk4(uint32_t target, uint32_t low, uint32_t high, uint32_t key) {
    uint32_t mid = low + ((high - low) >> 1);
    if (target == mid) return (mid + UINT32_C(0xac)) ^ key;
    if (target < mid) {
        return (3u * walk4(target, low, mid - 1u, key))
             ^ (mid + key) ^ UINT32_C(0x17);
    }
    return (3u * walk4(target, mid + 1u, high, key))
         ^ (mid + key) ^ UINT32_C(0x13);
}

int solve_phase_4(uint64_t challenge, char *out, size_t cap) {
    uint32_t target = (uint32_t)challenge & 127u;
    uint32_t key = (uint32_t)(challenge >> 7) ^ (uint32_t)(challenge >> 32);
    uint32_t values[] = {walk4(target, 0, 127, key)};
    return write_answer(out, cap, values, 1);
}

struct node5 { uint32_t value, tag; unsigned next; };

int solve_phase_5(uint64_t challenge, char *out, size_t cap) {
    /* Nodes in address order, beginning at 0x405ba0; each occupies 16 bytes. */
    static const struct node5 nodes[] = {
        {0xa7f8u, 0x045au, 4}, {0x2e83u, 0x0970u, 7},
        {0xe438u, 0x0767u, 9}, {0x4561u, 0x013bu, 0},
        {0x6085u, 0x0a45u, 5}, {0xcb37u, 0x0371u, 12},
        {0xfc4du, 0x0b16u, 11}, {0x7839u, 0x0847u, 8},
        {0xb981u, 0x0282u, 3}, {0xb652u, 0x0573u, 1},
        {0x75bdu, 0x0080u, 6}, {0xa814u, 0x0647u, 2}
    };
    static const unsigned heads[] = {10, 6, 11, 2, 9, 1, 7, 8};
    unsigned node = heads[(unsigned)challenge & 7u];
    unsigned count = (unsigned)((challenge >> 3) & 3u) + 1u;
    uint32_t acc = (uint32_t)(challenge >> 6) ^ (uint32_t)(challenge >> 32)
                 ^ UINT32_C(0xbdba);
    uint32_t tag = 0;
    for (unsigned i = 0; i < count; ++i) {
        if (node >= sizeof(nodes) / sizeof(nodes[0])) return 1;
        tag = nodes[node].tag;
        acc = ((acc + nodes[node].value) * 5u) ^ tag;
        node = nodes[node].next;
    }
    uint32_t values[] = {acc, tag};
    return write_answer(out, cap, values, 2);
}

static uint32_t op6(unsigned slot, uint32_t x) {
    /* Function table: op1,op0,op5,op3,op2,op0,op4,op2. */
    switch (slot) {
        case 0: return ((x << 5) | (x >> 27)) ^ UINT32_C(0xe20e);
        case 1:
        case 5: return (x + UINT32_C(0xf0bb)) ^ (x >> 1);
        case 2: return (x * 8u + UINT32_C(0x2fe3)) ^ (x * 5u);
        case 3: return ((x >> 1) ^ x) + UINT32_C(0x3ab7);
        case 4:
        case 7: return (x ^ UINT32_C(0xf5c7)) + (x << 5);
        default: return (x >> 7) | (x << 25);
    }
}

int solve_phase_6(uint64_t challenge, char *out, size_t cap) {
    uint32_t x = (uint32_t)(challenge >> 6) ^ (uint32_t)(challenge >> 32);
    x = op6((unsigned)challenge & 7u, x) ^ UINT32_C(0x3270);
    x = op6((unsigned)(challenge >> 3) & 7u, x) + UINT32_C(0x9c3b);
    uint32_t values[] = {x};
    return write_answer(out, cap, values, 1);
}

int solve_secret(uint64_t challenge, uint64_t context, char *out, size_t cap) {
    uint32_t a = (uint32_t)challenge ^ (uint32_t)(challenge >> 32);
    uint32_t b = (uint32_t)context ^ (uint32_t)(context >> 32);
    uint32_t selector = (uint32_t)context & 3u;
    uint32_t x = a ^ b;
    uint32_t y = ((x >> 1) ^ UINT32_C(0x1636)) + (x ^ UINT32_C(0x4de6));
    y ^= selector * UINT32_C(0x4de6);
    y ^= (uint32_t)(context >> 16);
    uint32_t z = y ^ ((a >> 1) + b + selector + UINT32_C(0x1636));
    /* secret_gate checks OPEN at byte offset 32, then parses three integers. */
    int size = snprintf(out, cap,
                        "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAOPEN %" PRIu32 " %" PRIu32 " %" PRIu32,
                        selector, y, z);
    return size < 0 || (size_t)size >= cap;
}
