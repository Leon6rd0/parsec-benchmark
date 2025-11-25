/*-
 * Copyright (c) YYYY Your Name or Organization (Optional for new work)
 * Based on concepts from FreeBSD's machine-specific atomic.h headers.
 *
 * SPDX-License-Identifier: BSD-2-Clause-FreeBSD OR MIT (Choose one if needed)
 *
 * This file provides atomic operations for the RISC-V 64-bit architecture
 * using GCC's __atomic built-ins, intended for use with the PARSEC benchmark suite.
 */

#ifndef _MACHINE_ATOMIC_H_
#define _MACHINE_ATOMIC_H_

#include <stdint.h>

/* Assume SMP for userland, consistent with amd64 version's approach */
#define MPLOCKED /* No specific prefix needed for __atomic built-ins on RISC-V */

/*
 * Redefine standard integer types to match the style used in the original amd64/atomic.h
 * These are typically defined in <sys/types.h> or similar, but included here for self-containment
 * as per the original structure.
 */
#ifndef _TYPE_DEFINITIONS_
#define _TYPE_DEFINITIONS_
typedef unsigned char       u_char;
typedef unsigned short      u_short;
typedef unsigned int        u_int;
typedef unsigned long       u_long;
#endif /* _TYPE_DEFINITIONS_ */


/* --- Core Atomic Operations using GCC Built-ins --- */


/*
 * General atomic operation generator macro.
 * NAME: Operation name (e.g., add, clear)
 * TYPE: Data type suffix (e.g., int, long)
 * C_TYPE: Actual C data type (e.g., u_int, u_long)
 * BUILTIN_OP: The GCC __atomic_fetch_* builtin operation (e.g., __ATOMIC_FETCH_ADD, __ATOMIC_FETCH_AND)
 * ADJ_V: Adjustment to the value 'v' before applying the operation (e.g., ~v for clear, +v for add)
 */
#define ATOMIC_OP(NAME, TYPE, C_TYPE, BUILTIN_OP, ADJ_V) \
static __inline void \
atomic_##NAME##_##TYPE(volatile u_##TYPE *p, u_##TYPE v) \
{ \
    __atomic_fetch_##BUILTIN_OP((volatile C_TYPE *)(p), (C_TYPE)(ADJ_V), __ATOMIC_SEQ_CST); \
} \
struct __hack


/* --- Specific Instantiations for Supported Types --- */

/* Operations on 8-bit bytes (char) */
ATOMIC_OP(set,      char, uint8_t,  or,  v);
ATOMIC_OP(clear,    char, uint8_t,  and, ~v);
ATOMIC_OP(add,      char, uint8_t,  add, v);
ATOMIC_OP(subtract, char, uint8_t,  sub, v);

/* Operations on 16-bit words (short) */
ATOMIC_OP(set,      short, uint16_t, or,  v);
ATOMIC_OP(clear,    short, uint16_t, and, ~v);
ATOMIC_OP(add,      short, uint16_t, add, v);
ATOMIC_OP(subtract, short, uint16_t, sub, v);

/* Operations on 32-bit double words (int) */
ATOMIC_OP(set,      int, uint32_t,  or,  v);
ATOMIC_OP(clear,    int, uint32_t,  and, ~v);
ATOMIC_OP(add,      int, uint32_t,  add, v);
ATOMIC_OP(subtract, int, uint32_t,  sub, v);

/* Operations on 64-bit quad words (long) - Assuming LP64 model where long is 64-bit */
ATOMIC_OP(set,      long, uint64_t, or,  v);
ATOMIC_OP(clear,    long, uint64_t, and, ~v);
ATOMIC_OP(add,      long, uint64_t, add, v);
ATOMIC_OP(subtract, long, uint64_t, sub, v);


/* --- Atomic Compare-And-Swap --- */


static __inline int
atomic_cmpset_int(volatile u_int *dst, u_int exp, u_int src)
{
    /* __atomic_compare_exchange_n expects the expected value via pointer and updates it on failure */
    u_int expected = exp;
    /* Returns true (non-zero) on success, false (zero) on failure */
    return __atomic_compare_exchange_n((volatile u_int *)dst, &expected, src, 0, __ATOMIC_SEQ_CST, __ATOMIC_RELAXED);
}

static __inline int
atomic_cmpset_long(volatile u_long *dst, u_long exp, u_long src)
{
    u_long expected = exp;
    return __atomic_compare_exchange_n((volatile u_long *)dst, &expected, src, 0, __ATOMIC_SEQ_CST, __ATOMIC_RELAXED);
}


/* --- Atomic Fetch-and-Add/Subtract --- */


static __inline u_int
atomic_fetchadd_int(volatile u_int *p, u_int v)
{
    /* Returns the value before the addition */
    return __atomic_fetch_add((volatile u_int *)p, v, __ATOMIC_SEQ_CST);
}

static __inline u_int
atomic_fetchsubtract_int(volatile u_int *p, int v)
{
    /* Subtracting v is equivalent to adding -v */
    return __atomic_fetch_add((volatile u_int *)p, (u_int)(-v), __ATOMIC_SEQ_CST);
}


/* --- Atomic Load-Acquire and Store-Release --- */


#define ATOMIC_LOAD_ACQ(TYPE, C_TYPE) \
static __inline u_##TYPE \
atomic_load_acq_##TYPE(volatile u_##TYPE *p) \
{ \
    u_##TYPE res; \
    __atomic_load((const volatile C_TYPE *)(p), (C_TYPE *)&res, __ATOMIC_ACQUIRE); \
    return res; \
} \
struct __hack

#define ATOMIC_STORE_REL(TYPE, C_TYPE) \
static __inline void \
atomic_store_rel_##TYPE(volatile u_##TYPE *p, u_##TYPE v) \
{ \
    __atomic_store((volatile C_TYPE *)(p), (C_TYPE *)&v, __ATOMIC_RELEASE); \
} \
struct __hack


ATOMIC_LOAD_ACQ(char, uint8_t);
ATOMIC_LOAD_ACQ(short, uint16_t);
ATOMIC_LOAD_ACQ(int, uint32_t);
ATOMIC_LOAD_ACQ(long, uint64_t);

ATOMIC_STORE_REL(char, uint8_t);
ATOMIC_STORE_REL(short, uint16_t);
ATOMIC_STORE_REL(int, uint32_t);
ATOMIC_STORE_REL(long, uint64_t);


/* --- Atomic Read-And-Clear --- */


static __inline u_int
atomic_readandclear_int(volatile u_int *addr)
{
    /* Setting to 0 after fetch is equivalent to fetching the old value and clearing */
    return __atomic_exchange_n((volatile u_int *)addr, 0, __ATOMIC_SEQ_CST);
}

static __inline u_long
atomic_readandclear_long(volatile u_long *addr)
{
    return __atomic_exchange_n((volatile u_long *)addr, 0, __ATOMIC_SEQ_CST);
}


/* --- Alias Macros for Consistency (Acquire/Release variants map to plain versions) --- */


#define atomic_set_acq_char             atomic_set_char
#define atomic_set_rel_char             atomic_set_char
#define atomic_clear_acq_char           atomic_clear_char
#define atomic_clear_rel_char           atomic_clear_char
#define atomic_add_acq_char             atomic_add_char
#define atomic_add_rel_char             atomic_add_char
#define atomic_subtract_acq_char        atomic_subtract_char
#define atomic_subtract_rel_char        atomic_subtract_char

#define atomic_set_acq_short            atomic_set_short
#define atomic_set_rel_short            atomic_set_short
#define atomic_clear_acq_short          atomic_clear_short
#define atomic_clear_rel_short          atomic_clear_short
#define atomic_add_acq_short            atomic_add_short
#define atomic_add_rel_short            atomic_add_short
#define atomic_subtract_acq_short       atomic_subtract_short
#define atomic_subtract_rel_short       atomic_subtract_short

#define atomic_set_acq_int              atomic_set_int
#define atomic_set_rel_int              atomic_set_int
#define atomic_clear_acq_int            atomic_clear_int
#define atomic_clear_rel_int            atomic_clear_int
#define atomic_add_acq_int              atomic_add_int
#define atomic_add_rel_int              atomic_add_int
#define atomic_subtract_acq_int         atomic_subtract_int
#define atomic_subtract_rel_int         atomic_subtract_int
#define atomic_cmpset_acq_int           atomic_cmpset_int
#define atomic_cmpset_rel_int           atomic_cmpset_int

#define atomic_set_acq_long             atomic_set_long
#define atomic_set_rel_long             atomic_set_long
#define atomic_clear_acq_long           atomic_clear_long
#define atomic_clear_rel_long           atomic_clear_long
#define atomic_add_acq_long             atomic_add_long
#define atomic_add_rel_long             atomic_add_long
#define atomic_subtract_acq_long        atomic_subtract_long
#define atomic_subtract_rel_long        atomic_subtract_long
#define atomic_cmpset_acq_long          atomic_cmpset_long
#define atomic_cmpset_rel_long          atomic_cmpset_long


/* --- Type Aliases for Generic Naming Conventions --- */


#define atomic_set_8            atomic_set_char
#define atomic_set_acq_8        atomic_set_acq_char
#define atomic_set_rel_8        atomic_set_rel_char
#define atomic_clear_8          atomic_clear_char
#define atomic_clear_acq_8      atomic_clear_acq_char
#define atomic_clear_rel_8      atomic_clear_rel_char
#define atomic_add_8            atomic_add_char
#define atomic_add_acq_8        atomic_add_acq_char
#define atomic_add_rel_8        atomic_add_rel_char
#define atomic_subtract_8       atomic_subtract_char
#define atomic_subtract_acq_8   atomic_subtract_acq_char
#define atomic_subtract_rel_8   atomic_subtract_rel_char
#define atomic_load_acq_8       atomic_load_acq_char
#define atomic_store_rel_8      atomic_store_rel_char

#define atomic_set_16           atomic_set_short
#define atomic_set_acq_16       atomic_set_acq_short
#define atomic_set_rel_16       atomic_set_rel_short
#define atomic_clear_16         atomic_clear_short
#define atomic_clear_acq_16     atomic_clear_acq_short
#define atomic_clear_rel_16     atomic_clear_rel_short
#define atomic_add_16           atomic_add_short
#define atomic_add_acq_16       atomic_add_acq_short
#define atomic_add_rel_16       atomic_add_rel_short
#define atomic_subtract_16      atomic_subtract_short
#define atomic_subtract_acq_16  atomic_subtract_acq_short
#define atomic_subtract_rel_16  atomic_subtract_rel_short
#define atomic_load_acq_16      atomic_load_acq_short
#define atomic_store_rel_16     atomic_store_rel_short

#define atomic_set_32           atomic_set_int
#define atomic_set_acq_32       atomic_set_acq_int
#define atomic_set_rel_32       atomic_set_rel_int
#define atomic_clear_32         atomic_clear_int
#define atomic_clear_acq_32     atomic_clear_acq_int
#define atomic_clear_rel_32     atomic_clear_rel_int
#define atomic_add_32           atomic_add_int
#define atomic_add_acq_32       atomic_add_acq_int
#define atomic_add_rel_32       atomic_add_rel_int
#define atomic_subtract_32      atomic_subtract_int
#define atomic_subtract_acq_32  atomic_subtract_acq_int
#define atomic_subtract_rel_32  atomic_subtract_rel_int
#define atomic_load_acq_32      atomic_load_acq_int
#define atomic_store_rel_32     atomic_store_rel_int
#define atomic_cmpset_32        atomic_cmpset_int
#define atomic_cmpset_acq_32    atomic_cmpset_acq_int
#define atomic_cmpset_rel_32    atomic_cmpset_rel_int
#define atomic_readandclear_32  atomic_readandclear_int
#define atomic_fetchadd_32      atomic_fetchadd_int
#define atomic_fetchsubtract_32 atomic_fetchsubtract_int

#define atomic_set_64           atomic_set_long
#define atomic_set_acq_64       atomic_set_acq_long
#define atomic_set_rel_64       atomic_set_rel_long
#define atomic_clear_64         atomic_clear_long
#define atomic_clear_acq_64     atomic_clear_acq_long
#define atomic_clear_rel_64     atomic_clear_rel_long
#define atomic_add_64           atomic_add_long
#define atomic_add_acq_64       atomic_add_acq_long
#define atomic_add_rel_64       atomic_add_rel_long
#define atomic_subtract_64      atomic_subtract_long
#define atomic_subtract_acq_64  atomic_subtract_acq_long
#define atomic_subtract_rel_64  atomic_subtract_rel_long
#define atomic_load_acq_64      atomic_load_acq_long
#define atomic_store_rel_64     atomic_store_rel_long
#define atomic_cmpset_64        atomic_cmpset_long
#define atomic_cmpset_acq_64    atomic_cmpset_acq_long
#define atomic_cmpset_rel_64    atomic_cmpset_rel_long
#define atomic_readandclear_64  atomic_readandclear_long

#define atomic_set_ptr          atomic_set_long
#define atomic_set_acq_ptr      atomic_set_acq_long
#define atomic_set_rel_ptr      atomic_set_rel_long
#define atomic_clear_ptr        atomic_clear_long
#define atomic_clear_acq_ptr    atomic_clear_acq_long
#define atomic_clear_rel_ptr    atomic_clear_rel_long
#define atomic_add_ptr          atomic_add_long
#define atomic_add_acq_ptr      atomic_add_acq_long
#define atomic_add_rel_ptr      atomic_add_rel_long
#define atomic_subtract_ptr     atomic_subtract_long
#define atomic_subtract_acq_ptr atomic_subtract_acq_long
#define atomic_subtract_rel_ptr atomic_subtract_rel_long
#define atomic_load_acq_ptr     atomic_load_acq_long
#define atomic_store_rel_ptr    atomic_store_rel_long
#define atomic_cmpset_ptr       atomic_cmpset_long
#define atomic_cmpset_acq_ptr   atomic_cmpset_acq_long
#define atomic_cmpset_rel_ptr   atomic_cmpset_rel_long
#define atomic_readandclear_ptr atomic_readandclear_long


#endif /* !_MACHINE_ATOMIC_H_ */
