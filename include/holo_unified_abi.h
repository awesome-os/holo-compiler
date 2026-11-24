#ifndef HOLO_UNIFIED_ABI_H
#define HOLO_UNIFIED_ABI_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * HOLO_UNIFIED_ABI_V3 — Unified Multi-Language Compile Unit & Effect Gate ABI
 * Zero-dependency, 64-byte-aligned C11 ABI for cxx, rust, zig, wasi, and ts
 * compilation units, O(1) effect admissibility checks, and sub-TU splicing.
 * ============================================================================ */

#define HOLO_ABI_MAGIC_V3      0x33564742u /* "BGV3" (little-endian) */
#define HOLO_ABI_VERSION_V3    3u
#define HOLO_CACHELINE_ALIGN   64u
#define HOLO_HASH_BYTES        32u

#if defined(HOLO_UNIFIED_ABI_STATIC)
#  define HOLO_ABI_EXPORT
#elif defined(_WIN32) || defined(__CYGWIN__)
#  define HOLO_ABI_EXPORT __declspec(dllexport)
#else
#  define HOLO_ABI_EXPORT __attribute__((visibility("default")))
#endif

#if defined(__cplusplus) && __cplusplus >= 201103L
#  define HOLO_ALIGNAS(n) alignas(n)
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
#  define HOLO_ALIGNAS(n) _Alignas(n)
#elif defined(_MSC_VER)
#  define HOLO_ALIGNAS(n) __declspec(align(n))
#else
#  define HOLO_ALIGNAS(n) __attribute__((aligned(n)))
#endif

typedef enum HoloAbiError {
    HOLO_ABI_OK                   =  0,
    HOLO_ABI_ERR_INVALID_ARG      = -1,
    HOLO_ABI_ERR_INVALID_MAGIC    = -2,
    HOLO_ABI_ERR_INVALID_VERSION  = -3,
    HOLO_ABI_ERR_INVALID_TOOLCHAIN = -4,
    HOLO_ABI_ERR_EFFECT_DENIED    = -5,
    HOLO_ABI_ERR_UNALIGNED        = -6
} HoloAbiError;

typedef enum HoloToolchainKind {
    HOLO_TOOLCHAIN_CXX  = 1u,
    HOLO_TOOLCHAIN_RUST = 2u,
    HOLO_TOOLCHAIN_ZIG  = 3u,
    HOLO_TOOLCHAIN_WASI = 4u,
    HOLO_TOOLCHAIN_TS   = 5u
} HoloToolchainKind;

/* Effect capability bitmask (uint64_t) */
#define HOLO_EFFECT_PURE          (0ULL)
#define HOLO_EFFECT_ALLOC         (1ULL << 0)
#define HOLO_EFFECT_IO_FS         (1ULL << 1)
#define HOLO_EFFECT_IO_NET        (1ULL << 2)
#define HOLO_EFFECT_ARENA_SAB     (1ULL << 3)
#define HOLO_EFFECT_UNSAFE_FFI    (1ULL << 4)
#define HOLO_EFFECT_ASYNC         (1ULL << 5)
#define HOLO_EFFECT_GPU_WGPU      (1ULL << 6)
#define HOLO_EFFECT_PROCESS_SPAWN (1ULL << 7)

#define HOLO_EFFECT_ALL_MASK (     \
    HOLO_EFFECT_ALLOC            | \
    HOLO_EFFECT_IO_FS            | \
    HOLO_EFFECT_IO_NET           | \
    HOLO_EFFECT_ARENA_SAB        | \
    HOLO_EFFECT_UNSAFE_FFI       | \
    HOLO_EFFECT_ASYNC            | \
    HOLO_EFFECT_GPU_WGPU         | \
    HOLO_EFFECT_PROCESS_SPAWN      \
)

typedef enum HoloSpliceDecision {
    HOLO_SPLICE_UNCHANGED    = 0,
    HOLO_SPLICE_BODY_ONLY    = 1,
    HOLO_SPLICE_FULL_REBUILD = 2
} HoloSpliceDecision;

/*
 * 64-byte-aligned, 128-byte (2 cache lines) compile unit descriptor.
 * Layout is strictly identical across Win64, Linux x64/ARM64, macOS, and WASI.
 */
typedef struct HoloCompileUnitV3 {
    HOLO_ALIGNAS(64) uint32_t abi_magic;  /* offset   0: HOLO_ABI_MAGIC_V3 (0x33564742) */
    uint32_t abi_version;                 /* offset   4: HOLO_ABI_VERSION_V3 (3)        */
    uint32_t toolchain_kind;              /* offset   8: HoloToolchainKind (1..5)       */
    uint32_t flags;                       /* offset  12: unit flags (reserved = 0)      */
    uint64_t effect_mask;                 /* offset  16: effects required by this unit  */
    uint64_t allowed_effect_mask;         /* offset  24: effects permitted by policy    */
    uint8_t  item_hash[HOLO_HASH_BYTES];  /* offset  32: full item/body SHA-256 hash    */
    uint8_t  sig_hash[HOLO_HASH_BYTES];   /* offset  64: public ABI/signature SHA-256   */
    uint64_t unit_id;                     /* offset  96: unique compile unit identifier */
    uint64_t arena_offset;                /* offset 104: 64-byte aligned arena offset   */
    uint64_t arena_size;                  /* offset 112: payload size in B8G_ARENA_V1   */
    uint64_t _reserved;                   /* offset 120: zero-initialized padding       */
} HoloCompileUnitV3;

/* ============================================================================
 * Public C11 ABI Functions
 * ============================================================================ */

/* Initialize a HoloCompileUnitV3 with magic, version, toolchain, and effect masks. */
HOLO_ABI_EXPORT int32_t holo_unit_init(
    HoloCompileUnitV3 *u,
    uint32_t toolchain_kind,
    uint64_t effect_mask,
    uint64_t allowed_effect_mask
);

/* Validate header magic, version, toolchain kind, arena alignment, and effect gate. */
HOLO_ABI_EXPORT int32_t holo_unit_validate(const HoloCompileUnitV3 *u);

/*
 * O(1) bitwise effect admissibility gate:
 * Returns 1 if u != NULL and (u->effect_mask & ~u->allowed_effect_mask) == 0,
 * otherwise returns 0.
 */
HOLO_ABI_EXPORT int32_t holo_check_effect_admissibility(const HoloCompileUnitV3 *u);

/*
 * Sub-TU function-level splice decision:
 * - HOLO_SPLICE_UNCHANGED    (0): sig_hash identical AND item_hash identical
 * - HOLO_SPLICE_BODY_ONLY    (1): sig_hash identical AND item_hash changed
 * - HOLO_SPLICE_FULL_REBUILD (2): sig_hash changed (or NULL input, fail-closed)
 */
HOLO_ABI_EXPORT HoloSpliceDecision holo_splice_decision(
    const uint8_t prev_item[HOLO_HASH_BYTES],
    const uint8_t prev_sig[HOLO_HASH_BYTES],
    const uint8_t next_item[HOLO_HASH_BYTES],
    const uint8_t next_sig[HOLO_HASH_BYTES]
);

/* Compare two full HoloCompileUnitV3 descriptors (also checks toolchain_kind). */
HOLO_ABI_EXPORT HoloSpliceDecision holo_splice_decision_units(
    const HoloCompileUnitV3 *prev,
    const HoloCompileUnitV3 *next
);

/* Return canonical lowercase toolchain name ("cxx", "rust", "zig", "wasi", "ts", "unknown"). */
HOLO_ABI_EXPORT const char *holo_toolchain_name(uint32_t toolchain_kind);

#ifdef __cplusplus
}
#endif

#endif /* HOLO_UNIFIED_ABI_H */
