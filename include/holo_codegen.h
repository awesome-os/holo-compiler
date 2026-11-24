#ifndef HOLO_CODEGEN_H
#define HOLO_CODEGEN_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * libholo_codegen — Zero-Dependency In-Process Native Machine-Code & WASI
 * Bytecode Emitter (B8G_ABI_MAGIC_V3 = 0x33564742).
 *
 * Eliminates the ~300 ms subprocess spawn floor (zig cc / clang / rustc) for
 * single-function sub-TU hot-splices by emitting executable Win64 x86-64,
 * System V x86-64, and WebAssembly (wasm32-wasi) code directly in-process.
 * ============================================================================ */

#ifndef B8G_ABI_MAGIC_V3
#  define B8G_ABI_MAGIC_V3      0x33564742u /* "BGV3" (little-endian) */
#endif

#ifndef HOLO_CACHELINE_ALIGN
#  define HOLO_CACHELINE_ALIGN  64u
#endif

#define HOLO_CG_MAX_NAME_LEN    64u

#if defined(HOLO_CODEGEN_STATIC)
#  define HOLO_CG_EXPORT
#elif defined(_WIN32) || defined(__CYGWIN__)
#  define HOLO_CG_EXPORT __declspec(dllexport)
#else
#  define HOLO_CG_EXPORT __attribute__((visibility("default")))
#endif

/* Effect capability bitmask (aligned with holo_unified_abi.h) */
#ifndef HOLO_EFFECT_PURE
#  define HOLO_EFFECT_PURE          (0ULL)
#  define HOLO_EFFECT_ALLOC         (1ULL << 0)
#  define HOLO_EFFECT_IO_FS         (1ULL << 1)
#  define HOLO_EFFECT_IO_NET        (1ULL << 2)
#  define HOLO_EFFECT_ARENA_SAB     (1ULL << 3)
#  define HOLO_EFFECT_UNSAFE_FFI    (1ULL << 4)
#  define HOLO_EFFECT_ASYNC         (1ULL << 5)
#  define HOLO_EFFECT_GPU_WGPU      (1ULL << 6)
#  define HOLO_EFFECT_PROCESS_SPAWN (1ULL << 7)
#endif

typedef enum HoloCodegenError {
    HOLO_CG_OK                     =  0,
    HOLO_CG_ERR_INVALID_ARG        = -1,
    HOLO_CG_ERR_EFFECT_FORBIDDEN   = -2,
    HOLO_CG_ERR_UNSUPPORTED_TARGET = -3,
    HOLO_CG_ERR_UNSUPPORTED_OP     = -4,
    HOLO_CG_ERR_BUFFER_TOO_SMALL   = -5,
    HOLO_CG_ERR_UNALIGNED_BUF      = -6,
    HOLO_CG_ERR_PARSE_FAILED       = -7,
    HOLO_CG_ERR_BAD_MAGIC          = -8
} HoloCodegenError;

typedef enum HoloTarget {
    HOLO_TARGET_WIN64_X64   = 1u,
    HOLO_TARGET_SYSV_X64    = 2u,
    HOLO_TARGET_WASM32_WASI = 3u
} HoloTarget;

typedef enum HoloFnOp {
    HOLO_OP_ADD_IMM   = 1u, /* return arg0 + imm */
    HOLO_OP_SUB_IMM   = 2u, /* return arg0 - imm */
    HOLO_OP_MUL_IMM   = 3u, /* return arg0 * imm */
    HOLO_OP_XOR_IMM   = 4u, /* return arg0 ^ imm */
    HOLO_OP_ADD_ARGS  = 5u, /* return arg0 + arg1 */
    HOLO_OP_RET_CONST = 6u  /* return imm */
} HoloFnOp;

/*
 * Parsed single-function IR descriptor.
 * `abi_magic` may be 0 (implicit V3) or B8G_ABI_MAGIC_V3 (0x33564742).
 * Effect admissibility gate: `(effect_mask & ~allowed_effect_mask) == 0`.
 */
typedef struct HoloFnIr {
    uint32_t abi_magic;
    uint32_t param_count;
    HoloFnOp op;
    uint32_t _pad;
    uint64_t imm;
    uint64_t effect_mask;
    uint64_t allowed_effect_mask;
    char     name[HOLO_CG_MAX_NAME_LEN];
} HoloFnIr;

/* ============================================================================
 * Public C11 API
 * ============================================================================ */

/*
 * O(1) B8G_ABI_MAGIC_V3 effect gate check.
 * Returns 1 if ir != NULL and (ir->effect_mask & ~ir->allowed_effect_mask) == 0,
 * otherwise returns 0.
 */
HOLO_CG_EXPORT int32_t holo_codegen_check_effects(const HoloFnIr *ir);

/*
 * Emit executable machine code (Win64 x86-64 / SysV x86-64) or a valid
 * WebAssembly binary module (wasm32-wasi) into `out_buf`.
 *
 * Invariants:
 * - Enforces the O(1) effect gate BEFORE emitting a single byte.
 * - On any error, sets `*out_len = 0` and writes 0 bytes.
 * - For native x86-64 targets, `out_buf` MUST be 64-byte aligned.
 */
HOLO_CG_EXPORT int32_t holo_codegen_emit(
    const HoloFnIr *ir,
    HoloTarget target,
    uint8_t *out_buf,
    size_t out_cap,
    size_t *out_len
);

/*
 * Zero-AST single-pass parser extracting simple C, Rust, Zig, or TypeScript
 * single-function definitions directly into `HoloFnIr`.
 *
 * Examples:
 *   pub fn compute_250(x: u64) -> u64 { x.wrapping_add(999999) }
 *   uint64_t compute_250(uint64_t x) { return x + 999999; }
 *   export function compute_250(x: number): number { return x * 7; }
 */
HOLO_CG_EXPORT int32_t holo_codegen_parse_fn_source(
    const char *src,
    HoloFnIr *out_ir
);

/* Return canonical target name ("win64-x64", "sysv-x64", "wasm32-wasi", "unknown"). */
HOLO_CG_EXPORT const char *holo_codegen_target_name(HoloTarget target);

#ifdef __cplusplus
}
#endif

#endif /* HOLO_CODEGEN_H */
