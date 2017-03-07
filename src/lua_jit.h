/*
** LuaJIT common internal definitions.
** Copyright (C) 2005-2014 Mike Pall. See Copyright Notice in luajit.h
*/

#ifndef _LJ_DEF_H
#define _LJ_DEF_H

//#include "lua.h"

#if defined(_MSC_VER)
/* MSVC is stuck in the last century and doesn't have C99's stdint.h. */
//typedef __int8 int8_t;
typedef __int16 int16_t;
typedef __int32 int32_t;
typedef __int64 int64_t;
typedef unsigned __int8 uint8_t;
typedef unsigned __int16 uint16_t;
typedef unsigned __int32 uint32_t;
typedef unsigned __int64 uint64_t;
#ifdef _WIN64
typedef __int64 intptr_t;
typedef unsigned __int64 uintptr_t;
#else
typedef __int32 intptr_t;
typedef unsigned __int32 uintptr_t;
#endif
#elif defined(__symbian__)
/* Cough. */
typedef signed char int8_t;
typedef short int int16_t;
typedef int int32_t;
typedef long long int64_t;
typedef unsigned char uint8_t;
typedef unsigned short int uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;
typedef int intptr_t;
typedef unsigned int uintptr_t;
#else
#include <stdint.h>
#endif

/* Needed everywhere. */
#include <string.h>
#include <stdlib.h>

/* Various VM limits. */
#define LJ_MAX_MEM  0x7fffff00  /* Max. total memory allocation. */
#define LJ_MAX_ALLOC  LJ_MAX_MEM  /* Max. individual allocation length. */
#define LJ_MAX_STR  LJ_MAX_MEM  /* Max. string length. */
#define LJ_MAX_UDATA  LJ_MAX_MEM  /* Max. userdata length. */

#define LJ_MAX_STRTAB (1<<26)   /* Max. string table size. */
#define LJ_MAX_HBITS  26    /* Max. hash bits. */
#define LJ_MAX_ABITS  28    /* Max. bits of array key. */
#define LJ_MAX_ASIZE  ((1<<(LJ_MAX_ABITS-1))+1)  /* Max. array part size. */
#define LJ_MAX_COLOSIZE 16    /* Max. elems for colocated array. */

#define LJ_MAX_LINE LJ_MAX_MEM  /* Max. source code line number. */
#define LJ_MAX_XLEVEL 200   /* Max. syntactic nesting level. */
#define LJ_MAX_BCINS  (1<<26)   /* Max. # of bytecode instructions. */
#define LJ_MAX_SLOTS  250   /* Max. # of slots in a Lua func. */
#define LJ_MAX_LOCVAR 200   /* Max. # of local variables. */
#define LJ_MAX_UPVAL  60    /* Max. # of upvalues. */

#define LJ_MAX_IDXCHAIN 100   /* __index/__newindex chain limit. */
#define LJ_STACK_EXTRA  5   /* Extra stack space (metamethods). */

#define LJ_NUM_CBPAGE 1   /* Number of FFI callback pages. */

/* Minimum table/buffer sizes. */
#define LJ_MIN_GLOBAL 6   /* Min. global table size (hbits). */
#define LJ_MIN_REGISTRY 2   /* Min. registry size (hbits). */
#define LJ_MIN_STRTAB 256   /* Min. string table size (pow2). */
#define LJ_MIN_SBUF 32    /* Min. string buffer length. */
#define LJ_MIN_VECSZ  8   /* Min. size for growable vectors. */
#define LJ_MIN_IRSZ 32    /* Min. size for growable IR. */
#define LJ_MIN_K64SZ  16    /* Min. size for chained K64Array. */

/* JIT compiler limits. */
#define LJ_MAX_JSLOTS 250   /* Max. # of stack slots for a trace. */
#define LJ_MAX_PHI  64    /* Max. # of PHIs for a loop. */
#define LJ_MAX_EXITSTUBGR 16  /* Max. # of exit stub groups. */

/* Various macros. */
#ifndef UNUSED
#define UNUSED(x) ((void)(x)) /* to avoid warnings */
#endif

#define U64x(hi, lo)  (((uint64_t)0x##hi << 32) + (uint64_t)0x##lo)
#define i32ptr(p) ((int32_t)(intptr_t)(void *)(p))
#define u32ptr(p) ((uint32_t)(intptr_t)(void *)(p))

#define checki8(x)  ((x) == (int32_t)(int8_t)(x))
#define checku8(x)  ((x) == (int32_t)(uint8_t)(x))
#define checki16(x) ((x) == (int32_t)(int16_t)(x))
#define checku16(x) ((x) == (int32_t)(uint16_t)(x))
#define checki32(x) ((x) == (int32_t)(x))
#define checku32(x) ((x) == (uint32_t)(x))
#define checkptr32(x) ((uintptr_t)(x) == (uint32_t)(uintptr_t)(x))

/* Every half-decent C compiler transforms this into a rotate instruction. */
#define lj_rol(x, n)  (((x)<<(n)) | ((x)>>(-(int)(n)&(8*sizeof(x)-1))))
#define lj_ror(x, n)  (((x)<<(-(int)(n)&(8*sizeof(x)-1))) | ((x)>>(n)))

/* A really naive Bloom filter. But sufficient for our needs. */
typedef uintptr_t BloomFilter;
#define BLOOM_MASK  (8*sizeof(BloomFilter) - 1)
#define bloombit(x) ((uintptr_t)1 << ((x) & BLOOM_MASK))
#define bloomset(b, x)  ((b) |= bloombit((x)))
#define bloomtest(b, x) ((b) & bloombit((x)))

#if defined(__GNUC__)

#define LJ_NORET  __attribute__((noreturn))
#define LJ_ALIGN(n) __attribute__((aligned(n)))
#define LJ_INLINE inline
#define LJ_AINLINE  inline __attribute__((always_inline))
#define LJ_NOINLINE __attribute__((noinline))

#if defined(__ELF__) || defined(__MACH__)
#if !((defined(__sun__) && defined(__svr4__)) || defined(__CELLOS_LV2__))
#define LJ_NOAPI  extern __attribute__((visibility("hidden")))
#endif
#endif

/* Note: it's only beneficial to use fastcall on x86 and then only for up to
** two non-FP args. The amalgamated compile covers all LJ_FUNC cases. Only
** indirect calls and related tail-called C functions are marked as fastcall.
*/
#if defined(__i386__)
#define LJ_FASTCALL __attribute__((fastcall))
#endif

#define LJ_LIKELY(x)  __builtin_expect(!!(x), 1)
#define LJ_UNLIKELY(x)  __builtin_expect(!!(x), 0)

#define lj_ffs(x) ((uint32_t)__builtin_ctz(x))
/* Don't ask ... */
#if defined(__INTEL_COMPILER) && (defined(__i386__) || defined(__x86_64__))
static LJ_AINLINE uint32_t lj_fls(uint32_t x)
{
  uint32_t r; __asm__("bsrl %1, %0" : "=r" (r) : "rm" (x) : "cc"); return r;
}
#else
#define lj_fls(x) ((uint32_t)(__builtin_clz(x)^31))
#endif

#if defined(__arm__)
static LJ_AINLINE uint32_t lj_bswap(uint32_t x)
{
  uint32_t r;
#if __ARM_ARCH_6__ || __ARM_ARCH_6J__ || __ARM_ARCH_6T2__ || __ARM_ARCH_6Z__ ||\
    __ARM_ARCH_6ZK__ || __ARM_ARCH_7__ || __ARM_ARCH_7A__ || __ARM_ARCH_7R__
  __asm__("rev %0, %1" : "=r" (r) : "r" (x));
  return r;
#else
#ifdef __thumb__
  r = x ^ lj_ror(x, 16);
#else
  __asm__("eor %0, %1, %1, ror #16" : "=r" (r) : "r" (x));
#endif
  return ((r & 0xff00ffffu) >> 8) ^ lj_ror(x, 8);
#endif
}

static LJ_AINLINE uint64_t lj_bswap64(uint64_t x)
{
  return ((uint64_t)lj_bswap((uint32_t)x)<<32) | lj_bswap((uint32_t)(x>>32));
}
#elif (__GNUC__ > 4) || (__GNUC__ == 4 && __GNUC_MINOR__ >= 3)
static LJ_AINLINE uint32_t lj_bswap(uint32_t x)
{
  return (uint32_t)__builtin_bswap32((int32_t)x);
}

static LJ_AINLINE uint64_t lj_bswap64(uint64_t x)
{
  return (uint64_t)__builtin_bswap64((int64_t)x);
}
#elif defined(__i386__) || defined(__x86_64__)
static LJ_AINLINE uint32_t lj_bswap(uint32_t x)
{
  uint32_t r; __asm__("bswap %0" : "=r" (r) : "0" (x)); return r;
}

#if defined(__i386__)
static LJ_AINLINE uint64_t lj_bswap64(uint64_t x)
{
  return ((uint64_t)lj_bswap((uint32_t)x)<<32) | lj_bswap((uint32_t)(x>>32));
}
#else
static LJ_AINLINE uint64_t lj_bswap64(uint64_t x)
{
  uint64_t r; __asm__("bswap %0" : "=r" (r) : "0" (x)); return r;
}
#endif
#else
static LJ_AINLINE uint32_t lj_bswap(uint32_t x)
{
  return (x << 24) | ((x & 0xff00) << 8) | ((x >> 8) & 0xff00) | (x >> 24);
}

static LJ_AINLINE uint64_t lj_bswap64(uint64_t x)
{
  return (uint64_t)lj_bswap((uint32_t)(x >> 32)) |
   ((uint64_t)lj_bswap((uint32_t)x) << 32);
}
#endif

typedef union __attribute__((packed)) Unaligned16 {
  uint16_t u;
  uint8_t b[2];
} Unaligned16;

typedef union __attribute__((packed)) Unaligned32 {
  uint32_t u;
  uint8_t b[4];
} Unaligned32;

/* Unaligned load of uint16_t. */
static LJ_AINLINE uint16_t lj_getu16(const void *p)
{
  return ((const Unaligned16 *)p)->u;
}

/* Unaligned load of uint32_t. */
static LJ_AINLINE uint32_t lj_getu32(const void *p)
{
  return ((const Unaligned32 *)p)->u;
}

#elif defined(_MSC_VER)

#define LJ_NORET  __declspec(noreturn)
#define LJ_ALIGN(n) __declspec(align(n))
#define LJ_INLINE __inline
#define LJ_AINLINE  __forceinline
#define LJ_NOINLINE __declspec(noinline)
#if defined(_M_IX86)
#define LJ_FASTCALL __fastcall
#endif

#ifdef _M_PPC
unsigned int _CountLeadingZeros(long);
#pragma intrinsic(_CountLeadingZeros)
static LJ_AINLINE uint32_t lj_fls(uint32_t x)
{
  return _CountLeadingZeros(x) ^ 31;
}
#else
unsigned char _BitScanForward(uint32_t *, unsigned long);
unsigned char _BitScanReverse(uint32_t *, unsigned long);
#pragma intrinsic(_BitScanForward)
#pragma intrinsic(_BitScanReverse)

static LJ_AINLINE uint32_t lj_ffs(uint32_t x)
{
  uint32_t r; _BitScanForward(&r, x); return r;
}

static LJ_AINLINE uint32_t lj_fls(uint32_t x)
{
  uint32_t r; _BitScanReverse(&r, x); return r;
}
#endif

unsigned long _byteswap_ulong(unsigned long);
uint64_t _byteswap_uint64(uint64_t);
#define lj_bswap(x) (_byteswap_ulong((x)))
#define lj_bswap64(x) (_byteswap_uint64((x)))

#if defined(_M_PPC) && defined(LUAJIT_NO_UNALIGNED)
/*
** Replacement for unaligned loads on Xbox 360. Disabled by default since it's
** usually more costly than the occasional stall when crossing a cache-line.
*/
static LJ_AINLINE uint16_t lj_getu16(const void *v)
{
  const uint8_t *p = (const uint8_t *)v;
  return (uint16_t)((p[0]<<8) | p[1]);
}
static LJ_AINLINE uint32_t lj_getu32(const void *v)
{
  const uint8_t *p = (const uint8_t *)v;
  return (uint32_t)((p[0]<<24) | (p[1]<<16) | (p[2]<<8) | p[3]);
}
#else
/* Unaligned loads are generally ok on x86/x64. */
#define lj_getu16(p)  (*(uint16_t *)(p))
#define lj_getu32(p)  (*(uint32_t *)(p))
#endif

#else
#error "missing defines for your compiler"
#endif

/* Optional defines. */
#ifndef LJ_FASTCALL
#define LJ_FASTCALL
#endif
#ifndef LJ_NORET
#define LJ_NORET
#endif
#ifndef LJ_NOAPI
#define LJ_NOAPI  extern
#endif
#ifndef LJ_LIKELY
#define LJ_LIKELY(x)  (x)
#define LJ_UNLIKELY(x)  (x)
#endif

/* Attributes for internal functions. */
#define LJ_DATA   LJ_NOAPI
#define LJ_DATADEF
#define LJ_ASMF   LJ_NOAPI
#define LJ_FUNCA  LJ_NOAPI
#if defined(ljamalg_c)
#define LJ_FUNC   static
#else
#define LJ_FUNC   LJ_NOAPI
#endif
#define LJ_FUNC_NORET LJ_FUNC LJ_NORET
#define LJ_FUNCA_NORET  LJ_FUNCA LJ_NORET
#define LJ_ASMF_NORET LJ_ASMF LJ_NORET

/* Runtime assertions. */
#ifdef lua_assert
#define check_exp(c, e)   (lua_assert(c), (e))
#define api_check(l, e)   lua_assert(e)
#else
#define lua_assert(c)   ((void)0)
#define check_exp(c, e)   (e)
#define api_check   luai_apicheck
#endif

/* Static assertions. */
#define LJ_ASSERT_NAME2(name, line) name ## line
#define LJ_ASSERT_NAME(line)    LJ_ASSERT_NAME2(lj_assert_, line)
#ifdef __COUNTER__
#define LJ_STATIC_ASSERT(cond) \
  extern void LJ_ASSERT_NAME(__COUNTER__)(int STATIC_ASSERTION_FAILED[(cond)?1:-1])
#else
#define LJ_STATIC_ASSERT(cond) \
  extern void LJ_ASSERT_NAME(__LINE__)(int STATIC_ASSERTION_FAILED[(cond)?1:-1])
#endif

#endif

/*
** Target architecture selection.
** Copyright (C) 2005-2014 Mike Pall. See Copyright Notice in luajit.h
*/

#ifndef _LJ_ARCH_H
#define _LJ_ARCH_H

//#include "lua.h"

/* Target endianess. */
#define LUAJIT_LE 0
#define LUAJIT_BE 1

/* Target architectures. */
#define LUAJIT_ARCH_X86   1
#define LUAJIT_ARCH_x86   1
#define LUAJIT_ARCH_X64   2
#define LUAJIT_ARCH_x64   2
#define LUAJIT_ARCH_ARM   3
#define LUAJIT_ARCH_arm   3
#define LUAJIT_ARCH_PPC   4
#define LUAJIT_ARCH_ppc   4
#define LUAJIT_ARCH_PPCSPE  5
#define LUAJIT_ARCH_ppcspe  5
#define LUAJIT_ARCH_MIPS  6
#define LUAJIT_ARCH_mips  6

/* Target OS. */
#define LUAJIT_OS_OTHER   0
#define LUAJIT_OS_WINDOWS 1
#define LUAJIT_OS_LINUX   2
#define LUAJIT_OS_OSX   3
#define LUAJIT_OS_BSD   4
#define LUAJIT_OS_POSIX   5

/* Select native target if no target defined. */
#ifndef LUAJIT_TARGET

#if defined(__i386) || defined(__i386__) || defined(_M_IX86)
#define LUAJIT_TARGET LUAJIT_ARCH_X86
#elif defined(__x86_64__) || defined(__x86_64) || defined(_M_X64) || defined(_M_AMD64)
#define LUAJIT_TARGET LUAJIT_ARCH_X64
#elif defined(__arm__) || defined(__arm) || defined(__ARM__) || defined(__ARM)
#define LUAJIT_TARGET LUAJIT_ARCH_ARM
#elif defined(__ppc__) || defined(__ppc) || defined(__PPC__) || defined(__PPC) || defined(__powerpc__) || defined(__powerpc) || defined(__POWERPC__) || defined(__POWERPC) || defined(_M_PPC)
#ifdef __NO_FPRS__
#define LUAJIT_TARGET LUAJIT_ARCH_PPCSPE
#else
#define LUAJIT_TARGET LUAJIT_ARCH_PPC
#endif
#elif defined(__mips__) || defined(__mips) || defined(__MIPS__) || defined(__MIPS)
#define LUAJIT_TARGET LUAJIT_ARCH_MIPS
#else
#error "No support for this architecture (yet)"
#endif

#endif

/* Select native OS if no target OS defined. */
#ifndef LUAJIT_OS

#if defined(_WIN32) && !defined(_XBOX_VER)
#define LUAJIT_OS LUAJIT_OS_WINDOWS
#elif defined(__linux__)
#define LUAJIT_OS LUAJIT_OS_LINUX
#elif defined(__MACH__) && defined(__APPLE__)
#define LUAJIT_OS LUAJIT_OS_OSX
#elif (defined(__FreeBSD__) || defined(__FreeBSD_kernel__) || \
       defined(__NetBSD__) || defined(__OpenBSD__)) && !defined(__ORBIS__)
#define LUAJIT_OS LUAJIT_OS_BSD
#elif (defined(__sun__) && defined(__svr4__)) || defined(__CYGWIN__)
#define LUAJIT_OS LUAJIT_OS_POSIX
#else
#define LUAJIT_OS LUAJIT_OS_OTHER
#endif

#endif

/* Set target OS properties. */
#if LUAJIT_OS == LUAJIT_OS_WINDOWS
#define LJ_OS_NAME  "Windows"
#elif LUAJIT_OS == LUAJIT_OS_LINUX
#define LJ_OS_NAME  "Linux"
#elif LUAJIT_OS == LUAJIT_OS_OSX
#define LJ_OS_NAME  "OSX"
#elif LUAJIT_OS == LUAJIT_OS_BSD
#define LJ_OS_NAME  "BSD"
#elif LUAJIT_OS == LUAJIT_OS_POSIX
#define LJ_OS_NAME  "POSIX"
#else
#define LJ_OS_NAME  "Other"
#endif

#define LJ_TARGET_WINDOWS (LUAJIT_OS == LUAJIT_OS_WINDOWS)
#define LJ_TARGET_LINUX   (LUAJIT_OS == LUAJIT_OS_LINUX)
#define LJ_TARGET_OSX   (LUAJIT_OS == LUAJIT_OS_OSX)
#define LJ_TARGET_IOS   (LJ_TARGET_OSX && LUAJIT_TARGET == LUAJIT_ARCH_ARM)
#define LJ_TARGET_POSIX   (LUAJIT_OS > LUAJIT_OS_WINDOWS)
#define LJ_TARGET_DLOPEN  LJ_TARGET_POSIX

#ifdef __CELLOS_LV2__
#define LJ_TARGET_PS3   1
#define LJ_TARGET_CONSOLE 1
#endif

#ifdef __ORBIS__
#define LJ_TARGET_PS4   1
#define LJ_TARGET_CONSOLE 1
#undef NULL
#define NULL ((void*)0)
#endif

#if _XBOX_VER >= 200
#define LJ_TARGET_XBOX360 1
#define LJ_TARGET_CONSOLE 1
#endif

#define LJ_NUMMODE_SINGLE 0 /* Single-number mode only. */
#define LJ_NUMMODE_SINGLE_DUAL  1 /* Default to single-number mode. */
#define LJ_NUMMODE_DUAL   2 /* Dual-number mode only. */
#define LJ_NUMMODE_DUAL_SINGLE  3 /* Default to dual-number mode. */

/* Set target architecture properties. */
#if LUAJIT_TARGET == LUAJIT_ARCH_X86

#define LJ_ARCH_NAME    "x86"
#define LJ_ARCH_BITS    32
#define LJ_ARCH_ENDIAN    LUAJIT_LE
#if LJ_TARGET_WINDOWS || __CYGWIN__
#define LJ_ABI_WIN    1
#else
#define LJ_ABI_WIN    0
#endif
#define LJ_TARGET_X86   1
#define LJ_TARGET_X86ORX64  1
#define LJ_TARGET_EHRETREG  0
#define LJ_TARGET_MASKSHIFT 1
#define LJ_TARGET_MASKROT 1
#define LJ_TARGET_UNALIGNED 1
#define LJ_ARCH_NUMMODE   LJ_NUMMODE_SINGLE_DUAL

#elif LUAJIT_TARGET == LUAJIT_ARCH_X64

#define LJ_ARCH_NAME    "x64"
#define LJ_ARCH_BITS    64
#define LJ_ARCH_ENDIAN    LUAJIT_LE
#define LJ_ABI_WIN    LJ_TARGET_WINDOWS
#define LJ_TARGET_X64   1
#define LJ_TARGET_X86ORX64  1
#define LJ_TARGET_EHRETREG  0
#define LJ_TARGET_JUMPRANGE 31  /* +-2^31 = +-2GB */
#define LJ_TARGET_MASKSHIFT 1
#define LJ_TARGET_MASKROT 1
#define LJ_TARGET_UNALIGNED 1
#define LJ_ARCH_NUMMODE   LJ_NUMMODE_SINGLE_DUAL

#elif LUAJIT_TARGET == LUAJIT_ARCH_ARM

#define LJ_ARCH_NAME    "arm"
#define LJ_ARCH_BITS    32
#define LJ_ARCH_ENDIAN    LUAJIT_LE
#if !defined(LJ_ARCH_HASFPU) && __SOFTFP__
#define LJ_ARCH_HASFPU    0
#endif
#if !defined(LJ_ABI_SOFTFP) && !__ARM_PCS_VFP
#define LJ_ABI_SOFTFP   1
#endif
#define LJ_ABI_EABI   1
#define LJ_TARGET_ARM   1
#define LJ_TARGET_EHRETREG  0
#define LJ_TARGET_JUMPRANGE 25  /* +-2^25 = +-32MB */
#define LJ_TARGET_MASKSHIFT 0
#define LJ_TARGET_MASKROT 1
#define LJ_TARGET_UNIFYROT  2 /* Want only IR_BROR. */
#define LJ_ARCH_NUMMODE   LJ_NUMMODE_DUAL

#if __ARM_ARCH_7__ || __ARM_ARCH_7A__ || __ARM_ARCH_7R__ || __ARM_ARCH_7S__
#define LJ_ARCH_VERSION   70
#elif __ARM_ARCH_6T2__
#define LJ_ARCH_VERSION   61
#elif __ARM_ARCH_6__ || __ARM_ARCH_6J__ || __ARM_ARCH_6K__ || __ARM_ARCH_6Z__ || __ARM_ARCH_6ZK__
#define LJ_ARCH_VERSION   60
#else
#define LJ_ARCH_VERSION   50
#endif

#elif LUAJIT_TARGET == LUAJIT_ARCH_PPC

#define LJ_ARCH_NAME    "ppc"
#if _LP64
#define LJ_ARCH_BITS    64
#else
#define LJ_ARCH_BITS    32
#endif
#define LJ_ARCH_ENDIAN    LUAJIT_BE
#define LJ_TARGET_PPC   1
#define LJ_TARGET_EHRETREG  3
#define LJ_TARGET_JUMPRANGE 25  /* +-2^25 = +-32MB */
#define LJ_TARGET_MASKSHIFT 0
#define LJ_TARGET_MASKROT 1
#define LJ_TARGET_UNIFYROT  1 /* Want only IR_BROL. */
#define LJ_ARCH_NUMMODE   LJ_NUMMODE_DUAL_SINGLE

#if _ARCH_PWR7
#define LJ_ARCH_VERSION   70
#elif _ARCH_PWR6
#define LJ_ARCH_VERSION   60
#elif _ARCH_PWR5X
#define LJ_ARCH_VERSION   51
#elif _ARCH_PWR5
#define LJ_ARCH_VERSION   50
#elif _ARCH_PWR4
#define LJ_ARCH_VERSION   40
#else
#define LJ_ARCH_VERSION   0
#endif
#if __PPC64__ || __powerpc64__ || LJ_TARGET_CONSOLE
#define LJ_ARCH_PPC64   1
#define LJ_ARCH_NOFFI   1
#endif
#if _ARCH_PPCSQ
#define LJ_ARCH_SQRT    1
#endif
#if _ARCH_PWR5X
#define LJ_ARCH_ROUND   1
#endif
#if __PPU__
#define LJ_ARCH_CELL    1
#endif
#if LJ_TARGET_XBOX360
#define LJ_ARCH_XENON   1
#endif

#elif LUAJIT_TARGET == LUAJIT_ARCH_PPCSPE

#define LJ_ARCH_NAME    "ppcspe"
#define LJ_ARCH_BITS    32
#define LJ_ARCH_ENDIAN    LUAJIT_BE
#ifndef LJ_ABI_SOFTFP
#define LJ_ABI_SOFTFP   1
#endif
#define LJ_ABI_EABI   1
#define LJ_TARGET_PPCSPE  1
#define LJ_TARGET_EHRETREG  3
#define LJ_TARGET_JUMPRANGE 25  /* +-2^25 = +-32MB */
#define LJ_TARGET_MASKSHIFT 0
#define LJ_TARGET_MASKROT 1
#define LJ_TARGET_UNIFYROT  1 /* Want only IR_BROL. */
#define LJ_ARCH_NUMMODE   LJ_NUMMODE_SINGLE
#define LJ_ARCH_NOFFI   1 /* NYI: comparisons, calls. */
#define LJ_ARCH_NOJIT   1

#elif LUAJIT_TARGET == LUAJIT_ARCH_MIPS

#if defined(__MIPSEL__) || defined(__MIPSEL) || defined(_MIPSEL)
#define LJ_ARCH_NAME    "mipsel"
#define LJ_ARCH_ENDIAN    LUAJIT_LE
#else
#define LJ_ARCH_NAME    "mips"
#define LJ_ARCH_ENDIAN    LUAJIT_BE
#endif
#define LJ_ARCH_BITS    32
#define LJ_TARGET_MIPS    1
#define LJ_TARGET_EHRETREG  4
#define LJ_TARGET_JUMPRANGE 27  /* 2*2^27 = 256MB-aligned region */
#define LJ_TARGET_MASKSHIFT 1
#define LJ_TARGET_MASKROT 1
#define LJ_TARGET_UNIFYROT  2 /* Want only IR_BROR. */
#define LJ_ARCH_NUMMODE   LJ_NUMMODE_SINGLE

#if _MIPS_ARCH_MIPS32R2
#define LJ_ARCH_VERSION   20
#else
#define LJ_ARCH_VERSION   10
#endif

#else
#error "No target architecture defined"
#endif

#ifndef LJ_PAGESIZE
#define LJ_PAGESIZE   4096
#endif

/* Check for minimum required compiler versions. */
#if defined(__GNUC__)
#if LJ_TARGET_X86
#if (__GNUC__ < 3) || ((__GNUC__ == 3) && __GNUC_MINOR__ < 4)
#error "Need at least GCC 3.4 or newer"
#endif
#elif LJ_TARGET_X64
#if __GNUC__ < 4
#error "Need at least GCC 4.0 or newer"
#endif
#elif LJ_TARGET_ARM
#if (__GNUC__ < 4) || ((__GNUC__ == 4) && __GNUC_MINOR__ < 2)
#error "Need at least GCC 4.2 or newer"
#endif
#elif !LJ_TARGET_PS3
#if (__GNUC__ < 4) || ((__GNUC__ == 4) && __GNUC_MINOR__ < 3)
#error "Need at least GCC 4.3 or newer"
#endif
#endif
#endif

/* Check target-specific constraints. */
#ifndef _BUILDVM_H
#if LJ_TARGET_X64
#if __USING_SJLJ_EXCEPTIONS__
#error "Need a C compiler with native exception handling on x64"
#endif
#elif LJ_TARGET_ARM
#if defined(__ARMEB__)
#error "No support for big-endian ARM"
#endif
#if __ARM_ARCH_6M__ || __ARM_ARCH_7M__ || __ARM_ARCH_7EM__
#error "No support for Cortex-M CPUs"
#endif
#if !(__ARM_EABI__ || LJ_TARGET_IOS)
#error "Only ARM EABI or iOS 3.0+ ABI is supported"
#endif
#elif LJ_TARGET_PPC || LJ_TARGET_PPCSPE
#if defined(_SOFT_FLOAT) || defined(_SOFT_DOUBLE)
#error "No support for PowerPC CPUs without double-precision FPU"
#endif
#if defined(_LITTLE_ENDIAN)
#error "No support for little-endian PowerPC"
#endif
#if defined(_LP64)
#error "No support for PowerPC 64 bit mode"
#endif
#elif LJ_TARGET_MIPS
#if defined(__mips_soft_float)
#error "No support for MIPS CPUs without FPU"
#endif
#endif
#endif

/* Enable or disable the dual-number mode for the VM. */
#if (LJ_ARCH_NUMMODE == LJ_NUMMODE_SINGLE && LUAJIT_NUMMODE == 2) || \
    (LJ_ARCH_NUMMODE == LJ_NUMMODE_DUAL && LUAJIT_NUMMODE == 1)
#error "No support for this number mode on this architecture"
#endif
#if LJ_ARCH_NUMMODE == LJ_NUMMODE_DUAL || \
    (LJ_ARCH_NUMMODE == LJ_NUMMODE_DUAL_SINGLE && LUAJIT_NUMMODE != 1) || \
    (LJ_ARCH_NUMMODE == LJ_NUMMODE_SINGLE_DUAL && LUAJIT_NUMMODE == 2)
#define LJ_DUALNUM    1
#else
#define LJ_DUALNUM    0
#endif

#if LJ_TARGET_IOS || LJ_TARGET_CONSOLE
/* Runtime code generation is restricted on iOS. Complain to Apple, not me. */
/* Ditto for the consoles. Complain to Sony or MS, not me. */
#ifndef LUAJIT_ENABLE_JIT
#define LJ_OS_NOJIT   1
#endif
#endif

/* Disable or enable the JIT compiler. */
#if defined(LUAJIT_DISABLE_JIT) || defined(LJ_ARCH_NOJIT) || defined(LJ_OS_NOJIT)
#define LJ_HASJIT   0
#else
#define LJ_HASJIT   1
#endif

/* Disable or enable the FFI extension. */
#if defined(LUAJIT_DISABLE_FFI) || defined(LJ_ARCH_NOFFI)
#define LJ_HASFFI   0
#else
#define LJ_HASFFI   1
#endif

#ifndef LJ_ARCH_HASFPU
#define LJ_ARCH_HASFPU    1
#endif
#ifndef LJ_ABI_SOFTFP
#define LJ_ABI_SOFTFP   0
#endif
#define LJ_SOFTFP   (!LJ_ARCH_HASFPU)

#if LJ_ARCH_ENDIAN == LUAJIT_BE
#define LJ_LE     0
#define LJ_BE     1
#define LJ_ENDIAN_SELECT(le, be)  be
#define LJ_ENDIAN_LOHI(lo, hi)    hi lo
#else
#define LJ_LE     1
#define LJ_BE     0
#define LJ_ENDIAN_SELECT(le, be)  le
#define LJ_ENDIAN_LOHI(lo, hi)    lo hi
#endif

#if LJ_ARCH_BITS == 32
#define LJ_32     1
#define LJ_64     0
#else
#define LJ_32     0
#define LJ_64     1
#endif

#ifndef LJ_TARGET_UNALIGNED
#define LJ_TARGET_UNALIGNED 0
#endif

/* Various workarounds for embedded operating systems. */
#if (defined(__ANDROID__) && !defined(LJ_TARGET_X86ORX64)) || defined(__symbian__) || LJ_TARGET_XBOX360
#define LUAJIT_NO_LOG2
#endif
#if defined(__symbian__)
#define LUAJIT_NO_EXP2
#endif

#if defined(LUAJIT_NO_UNWIND) || defined(__symbian__) || LJ_TARGET_IOS || LJ_TARGET_PS3
#define LJ_NO_UNWIND    1
#endif

/* Compatibility with Lua 5.1 vs. 5.2. */
#ifdef LUAJIT_ENABLE_LUA52COMPAT
#define LJ_52     1
#else
#define LJ_52     0
#endif

#endif


#ifndef _LJ_OBJ_H
#define _LJ_OBJ_H

/* -- Memory references (32 bit address space) ---------------------------- */

/* Memory size. */
typedef uint32_t MSize;

/* Memory reference */
typedef struct MRef {
  uint32_t ptr32;	/* Pseudo 32 bit pointer. */
} MRef;

#define mref(r, t)	((t *)(void *)(uintptr_t)(r).ptr32)

#define setmref(r, p)	((r).ptr32 = (uint32_t)(uintptr_t)(void *)(p))
#define setmrefr(r, v)	((r).ptr32 = (v).ptr32)

/* -- GC object references (32 bit address space) ------------------------- */

/* GCobj reference */
typedef struct GCRef {
  uint32_t gcptr32;	/* Pseudo 32 bit pointer. */
} GCRef;

/* Common GC header for all collectable objects. */
#define GCHeader	GCRef nextgc; uint8_t marked; uint8_t gct
/* This occupies 6 bytes, so use the next 2 bytes for non-32 bit fields. */

#define gcref(r)	((GCobj *)(uintptr_t)(r).gcptr32)
#define gcrefp(r, t)	((t *)(void *)(uintptr_t)(r).gcptr32)
#define gcrefu(r)	((r).gcptr32)
#define gcrefi(r)	((int32_t)(r).gcptr32)
#define gcrefeq(r1, r2)	((r1).gcptr32 == (r2).gcptr32)
#define gcnext(gc)	(gcref((gc)->gch.nextgc))

#define setgcref(r, gc)	((r).gcptr32 = (uint32_t)(uintptr_t)&(gc)->gch)
#define setgcrefi(r, i)	((r).gcptr32 = (uint32_t)(i))
#define setgcrefp(r, p)	((r).gcptr32 = (uint32_t)(uintptr_t)(p))
#define setgcrefnull(r)	((r).gcptr32 = 0)
#define setgcrefr(r, v)	((r).gcptr32 = (v).gcptr32)

/* IMPORTANT NOTE:
**
** All uses of the setgcref* macros MUST be accompanied with a write barrier.
**
** This is to ensure the integrity of the incremental GC. The invariant
** to preserve is that a black object never points to a white object.
** I.e. never store a white object into a field of a black object.
**
** It's ok to LEAVE OUT the write barrier ONLY in the following cases:
** - The source is not a GC object (NULL).
** - The target is a GC root. I.e. everything in global_State.
** - The target is a lua_State field (threads are never black).
** - The target is a stack slot, see setgcV et al.
** - The target is an open upvalue, i.e. pointing to a stack slot.
** - The target is a newly created object (i.e. marked white). But make
**   sure nothing invokes the GC inbetween.
** - The target and the source are the same object (self-reference).
** - The target already contains the object (e.g. moving elements around).
**
** The most common case is a store to a stack slot. All other cases where
** a barrier has been omitted are annotated with a NOBARRIER comment.
**
** The same logic applies for stores to table slots (array part or hash
** part). ALL uses of lj_tab_set* require a barrier for the stored value
** *and* the stored key, based on the above rules. In practice this means
** a barrier is needed if *either* of the key or value are a GC object.
**
** It's ok to LEAVE OUT the write barrier in the following special cases:
** - The stored value is nil. The key doesn't matter because it's either
**   not resurrected or lj_tab_newkey() will take care of the key barrier.
** - The key doesn't matter if the *previously* stored value is guaranteed
**   to be non-nil (because the key is kept alive in the table).
** - The key doesn't matter if it's guaranteed not to be part of the table,
**   since lj_tab_newkey() takes care of the key barrier. This applies
**   trivially to new tables, but watch out for resurrected keys. Storing
**   a nil value leaves the key in the table!
**
** In case of doubt use lj_gc_anybarriert() as it's rather cheap. It's used
** by the interpreter for all table stores.
**
** Note: In contrast to Lua's GC, LuaJIT's GC does *not* specially mark
** dead keys in tables. The reference is left in, but it's guaranteed to
** be never dereferenced as long as the value is nil. It's ok if the key is
** freed or if any object subsequently gets the same address.
**
** Not destroying dead keys helps to keep key hash slots stable. This avoids
** specialization back-off for HREFK when a value flips between nil and
** non-nil and the GC gets in the way. It also allows safely hoisting
** HREF/HREFK across GC steps. Dead keys are only removed if a table is
** resized (i.e. by NEWREF) and xREF must not be CSEd across a resize.
**
** The trade-off is that a write barrier for tables must take the key into
** account, too. Implicitly resurrecting the key by storing a non-nil value
** may invalidate the incremental GC invariant.
*/

/* -- Common type definitions --------------------------------------------- */

/* Types for handling bytecodes. Need this here, details in lj_bc.h. */
typedef uint32_t BCIns;  /* Bytecode instruction. */
typedef uint32_t BCPos;  /* Bytecode position. */
typedef uint32_t BCReg;  /* Bytecode register. */
typedef int32_t BCLine;  /* Bytecode line number. */

/* Internal assembler functions. Never call these directly from C. */
typedef void (*ASMFunction)(void);

/* Resizable string buffer. Need this here, details in lj_str.h. */
typedef struct SBuf {
  char *buf;		/* String buffer base. */
  MSize n;		/* String buffer length. */
  MSize sz;		/* String buffer size. */
} SBuf;

/* -- Tags and values ----------------------------------------------------- */

/* Frame link. */
typedef union {
  int32_t ftsz;		/* Frame type and size of previous frame. */
  MRef pcr;		/* Overlaps PC for Lua frames. */
} FrameLink;

/* Tagged value. */
typedef LJ_ALIGN(8) union TValue {
  uint64_t u64;		/* 64 bit pattern overlaps number. */
  lua_Number n;		/* Number object overlaps split tag/value object. */
  struct {
    LJ_ENDIAN_LOHI(
      union {
	GCRef gcr;	/* GCobj reference (if any). */
	int32_t i;	/* Integer value. */
      };
    , uint32_t it;	/* Internal object tag. Must overlap MSW of number. */
    )
  };
  struct {
    LJ_ENDIAN_LOHI(
      GCRef func;	/* Function for next frame (or dummy L). */
    , FrameLink tp;	/* Link to previous frame. */
    )
  } fr;
  struct {
    LJ_ENDIAN_LOHI(
      uint32_t lo;	/* Lower 32 bits of number. */
    , uint32_t hi;	/* Upper 32 bits of number. */
    )
  } u32;
} TValue;

typedef const TValue cTValue;

#define tvref(r)	(mref(r, TValue))

/* More external and GCobj tags for internal objects. */
#define LAST_TT		LUA_TTHREAD
#define LUA_TPROTO	(LAST_TT+1)
#define LUA_TCDATA	(LAST_TT+2)

/* Internal object tags.
**
** Internal tags overlap the MSW of a number object (must be a double).
** Interpreted as a double these are special NaNs. The FPU only generates
** one type of NaN (0xfff8_0000_0000_0000). So MSWs > 0xfff80000 are available
** for use as internal tags. Small negative numbers are used to shorten the
** encoding of type comparisons (reg/mem against sign-ext. 8 bit immediate).
**
**                  ---MSW---.---LSW---
** primitive types |  itype  |         |
** lightuserdata   |  itype  |  void * |  (32 bit platforms)
** lightuserdata   |ffff|    void *    |  (64 bit platforms, 47 bit pointers)
** GC objects      |  itype  |  GCRef  |
** int (LJ_DUALNUM)|  itype  |   int   |
** number           -------double------
**
** ORDER LJ_T
** Primitive types nil/false/true must be first, lightuserdata next.
** GC objects are at the end, table/userdata must be lowest.
** Also check lj_ir.h for similar ordering constraints.
*/
#define LJ_TNIL			(~0u)
#define LJ_TFALSE		(~1u)
#define LJ_TTRUE		(~2u)
#define LJ_TLIGHTUD		(~3u)
#define LJ_TSTR			(~4u)
#define LJ_TUPVAL		(~5u)
#define LJ_TTHREAD		(~6u)
#define LJ_TPROTO		(~7u)
#define LJ_TFUNC		(~8u)
#define LJ_TTRACE		(~9u)
#define LJ_TCDATA		(~10u)
#define LJ_TTAB			(~11u)
#define LJ_TUDATA		(~12u)
/* This is just the canonical number type used in some places. */
#define LJ_TNUMX		(~13u)

/* Integers have itype == LJ_TISNUM doubles have itype < LJ_TISNUM */
#if LJ_64
#define LJ_TISNUM		0xfffeffffu
#else
#define LJ_TISNUM		LJ_TNUMX
#endif
#define LJ_TISTRUECOND		LJ_TFALSE
#define LJ_TISPRI		LJ_TTRUE
#define LJ_TISGCV		(LJ_TSTR+1)
#define LJ_TISTABUD		LJ_TTAB

/* -- String object ------------------------------------------------------- */

/* String object header. String payload follows. */
typedef struct GCstr {
  GCHeader;
  uint8_t reserved;	/* Used by lexer for fast lookup of reserved words. */
  uint8_t unused;
  MSize hash;		/* Hash of string. */
  MSize len;		/* Size of string. */
} GCstr;

#define strref(r)	(&gcref((r))->str)
#define strdata(s)	((const char *)((s)+1))
#define strdatawr(s)	((char *)((s)+1))
#define strVdata(o)	strdata(strV(o))
#define sizestring(s)	(sizeof(struct GCstr)+(s)->len+1)

/* -- Userdata object ----------------------------------------------------- */

/* Userdata object. Payload follows. */
typedef struct GCudata {
  GCHeader;
  uint8_t udtype;	/* Userdata type. */
  uint8_t unused2;
  GCRef env;		/* Should be at same offset in GCfunc. */
  MSize len;		/* Size of payload. */
  GCRef metatable;	/* Must be at same offset in GCtab. */
  uint32_t align1;	/* To force 8 byte alignment of the payload. */
} GCudata;

/* Userdata types. */
enum {
  UDTYPE_USERDATA,	/* Regular userdata. */
  UDTYPE_IO_FILE,	/* I/O library FILE. */
  UDTYPE_FFI_CLIB,	/* FFI C library namespace. */
  UDTYPE__MAX
};

#define uddata(u)	((void *)((u)+1))
#define sizeudata(u)	(sizeof(struct GCudata)+(u)->len)

/* -- C data object ------------------------------------------------------- */

/* C data object. Payload follows. */
typedef struct GCcdata {
  GCHeader;
  uint16_t ctypeid;	/* C type ID. */
} GCcdata;

/* Prepended to variable-sized or realigned C data objects. */
typedef struct GCcdataVar {
  uint16_t offset;	/* Offset to allocated memory (relative to GCcdata). */
  uint16_t extra;	/* Extra space allocated (incl. GCcdata + GCcdatav). */
  MSize len;		/* Size of payload. */
} GCcdataVar;

#define cdataptr(cd)	((void *)((cd)+1))
#define cdataisv(cd)	((cd)->marked & 0x80)
#define cdatav(cd)	((GCcdataVar *)((char *)(cd) - sizeof(GCcdataVar)))
#define cdatavlen(cd)	check_exp(cdataisv(cd), cdatav(cd)->len)
#define sizecdatav(cd)	(cdatavlen(cd) + cdatav(cd)->extra)
#define memcdatav(cd)	((void *)((char *)(cd) - cdatav(cd)->offset))

/* -- Prototype object ---------------------------------------------------- */

#define SCALE_NUM_GCO	((int32_t)sizeof(lua_Number)/sizeof(GCRef))
#define round_nkgc(n)	(((n) + SCALE_NUM_GCO-1) & ~(SCALE_NUM_GCO-1))

typedef struct GCproto {
  GCHeader;
  uint8_t numparams;	/* Number of parameters. */
  uint8_t framesize;	/* Fixed frame size. */
  MSize sizebc;		/* Number of bytecode instructions. */
  GCRef gclist;
  MRef k;		/* Split constant array (points to the middle). */
  MRef uv;		/* Upvalue list. local slot|0x8000 or parent uv idx. */
  MSize sizekgc;	/* Number of collectable constants. */
  MSize sizekn;		/* Number of lua_Number constants. */
  MSize sizept;		/* Total size including colocated arrays. */
  uint8_t sizeuv;	/* Number of upvalues. */
  uint8_t flags;	/* Miscellaneous flags (see below). */
  uint16_t trace;	/* Anchor for chain of root traces. */
  /* ------ The following fields are for debugging/tracebacks only ------ */
  GCRef chunkname;	/* Name of the chunk this function was defined in. */
  BCLine firstline;	/* First line of the function definition. */
  BCLine numline;	/* Number of lines for the function definition. */
  MRef lineinfo;	/* Compressed map from bytecode ins. to source line. */
  MRef uvinfo;		/* Upvalue names. */
  MRef varinfo;		/* Names and compressed extents of local variables. */
} GCproto;

/* Flags for prototype. */
#define PROTO_CHILD		0x01	/* Has child prototypes. */
#define PROTO_VARARG		0x02	/* Vararg function. */
#define PROTO_FFI		0x04	/* Uses BC_KCDATA for FFI datatypes. */
#define PROTO_NOJIT		0x08	/* JIT disabled for this function. */
#define PROTO_ILOOP		0x10	/* Patched bytecode with ILOOP etc. */
/* Only used during parsing. */
#define PROTO_HAS_RETURN	0x20	/* Already emitted a return. */
#define PROTO_FIXUP_RETURN	0x40	/* Need to fixup emitted returns. */
/* Top bits used for counting created closures. */
#define PROTO_CLCOUNT		0x20	/* Base of saturating 3 bit counter. */
#define PROTO_CLC_BITS		3
#define PROTO_CLC_POLY		(3*PROTO_CLCOUNT)  /* Polymorphic threshold. */

#define PROTO_UV_LOCAL		0x8000	/* Upvalue for local slot. */
#define PROTO_UV_IMMUTABLE	0x4000	/* Immutable upvalue. */

#define proto_kgc(pt, idx) \
  check_exp((uintptr_t)(intptr_t)(idx) >= (uintptr_t)-(intptr_t)(pt)->sizekgc, \
	    gcref(mref((pt)->k, GCRef)[(idx)]))
#define proto_knumtv(pt, idx) \
  check_exp((uintptr_t)(idx) < (pt)->sizekn, &mref((pt)->k, TValue)[(idx)])
#define proto_bc(pt)		((BCIns *)((char *)(pt) + sizeof(GCproto)))
#define proto_bcpos(pt, pc)	((BCPos)((pc) - proto_bc(pt)))
#define proto_uv(pt)		(mref((pt)->uv, uint16_t))

#define proto_chunkname(pt)	(strref((pt)->chunkname))
#define proto_chunknamestr(pt)	(strdata(proto_chunkname((pt))))
#define proto_lineinfo(pt)	(mref((pt)->lineinfo, const void))
#define proto_uvinfo(pt)	(mref((pt)->uvinfo, const uint8_t))
#define proto_varinfo(pt)	(mref((pt)->varinfo, const uint8_t))

/* -- Upvalue object ------------------------------------------------------ */

typedef struct GCupval {
  GCHeader;
  uint8_t closed;	/* Set if closed (i.e. uv->v == &uv->u.value). */
  uint8_t immutable;	/* Immutable value. */
  union {
    TValue tv;		/* If closed: the value itself. */
    struct {		/* If open: double linked list, anchored at thread. */
      GCRef prev;
      GCRef next;
    };
  };
  MRef v;		/* Points to stack slot (open) or above (closed). */
  uint32_t dhash;	/* Disambiguation hash: dh1 != dh2 => cannot alias. */
} GCupval;

#define uvprev(uv_)	(&gcref((uv_)->prev)->uv)
#define uvnext(uv_)	(&gcref((uv_)->next)->uv)
#define uvval(uv_)	(mref((uv_)->v, TValue))

/* -- Function object (closures) ------------------------------------------ */

/* Common header for functions. env should be at same offset in GCudata. */
#define GCfuncHeader \
  GCHeader; uint8_t ffid; uint8_t nupvalues; \
  GCRef env; GCRef gclist; MRef pc

typedef struct GCfuncC {
  GCfuncHeader;
  lua_CFunction f;	/* C function to be called. */
  TValue upvalue[1];	/* Array of upvalues (TValue). */
} GCfuncC;

typedef struct GCfuncL {
  GCfuncHeader;
  GCRef uvptr[1];	/* Array of _pointers_ to upvalue objects (GCupval). */
} GCfuncL;

typedef union GCfunc {
  GCfuncC c;
  GCfuncL l;
} GCfunc;

#define FF_LUA		0
#define FF_C		1
#define isluafunc(fn)	((fn)->c.ffid == FF_LUA)
#define iscfunc(fn)	((fn)->c.ffid == FF_C)
#define isffunc(fn)	((fn)->c.ffid > FF_C)
#define funcproto(fn) \
  check_exp(isluafunc(fn), (GCproto *)(mref((fn)->l.pc, char)-sizeof(GCproto)))
#define sizeCfunc(n)	(sizeof(GCfuncC)-sizeof(TValue)+sizeof(TValue)*(n))
#define sizeLfunc(n)	(sizeof(GCfuncL)-sizeof(GCRef)+sizeof(GCRef)*(n))

/* -- Table object -------------------------------------------------------- */

/* Hash node. */
typedef struct Node {
  TValue val;		/* Value object. Must be first field. */
  TValue key;		/* Key object. */
  MRef next;		/* Hash chain. */
  MRef freetop;		/* Top of free elements (stored in t->node[0]). */
} Node;

LJ_STATIC_ASSERT(offsetof(Node, val) == 0);

typedef struct GCtab {
  GCHeader;
  uint8_t nomm;		/* Negative cache for fast metamethods. */
  int8_t colo;		/* Array colocation. */
  MRef array;		/* Array part. */
  GCRef gclist;
  GCRef metatable;	/* Must be at same offset in GCudata. */
  MRef node;		/* Hash part. */
  uint32_t asize;	/* Size of array part (keys [0, asize-1]). */
  uint32_t hmask;	/* Hash part mask (size of hash part - 1). */
} GCtab;

#define sizetabcolo(n)	((n)*sizeof(TValue) + sizeof(GCtab))
#define tabref(r)	(&gcref((r))->tab)
#define noderef(r)	(mref((r), Node))
#define nextnode(n)	(mref((n)->next, Node))

/* -- State objects ------------------------------------------------------- */

/* VM states. */
enum {
  LJ_VMST_INTERP,	/* Interpreter. */
  LJ_VMST_C,		/* C function. */
  LJ_VMST_GC,		/* Garbage collector. */
  LJ_VMST_EXIT,		/* Trace exit handler. */
  LJ_VMST_RECORD,	/* Trace recorder. */
  LJ_VMST_OPT,		/* Optimizer. */
  LJ_VMST_ASM,		/* Assembler. */
  LJ_VMST__MAX
};

#define setvmstate(g, st)	((g)->vmstate = ~LJ_VMST_##st)

/* Metamethods. ORDER MM */
#ifdef LJ_HASFFI
#define MMDEF_FFI(_) _(new)
#else
#define MMDEF_FFI(_)
#endif

#if LJ_52 || LJ_HASFFI
#define MMDEF_PAIRS(_) _(pairs) _(ipairs)
#else
#define MMDEF_PAIRS(_)
#define MM_pairs	255
#define MM_ipairs	255
#endif

#define MMDEF(_) \
  _(index) _(newindex) _(gc) _(mode) _(eq) _(len) \
  /* Only the above (fast) metamethods are negative cached (max. 8). */ \
  _(lt) _(le) _(concat) _(call) \
  /* The following must be in ORDER ARITH. */ \
  _(add) _(sub) _(mul) _(div) _(mod) _(pow) _(unm) \
  /* The following are used in the standard libraries. */ \
  _(metatable) _(tostring) MMDEF_FFI(_) MMDEF_PAIRS(_)

typedef enum {
#define MMENUM(name)	MM_##name,
MMDEF(MMENUM)
#undef MMENUM
  MM__MAX,
  MM____ = MM__MAX,
  MM_FAST = MM_len
} MMS;

/* GC root IDs. */
typedef enum {
  GCROOT_MMNAME,	/* Metamethod names. */
  GCROOT_MMNAME_LAST = GCROOT_MMNAME + MM__MAX-1,
  GCROOT_BASEMT,	/* Metatables for base types. */
  GCROOT_BASEMT_NUM = GCROOT_BASEMT + ~LJ_TNUMX,
  GCROOT_IO_INPUT,	/* Userdata for default I/O input file. */
  GCROOT_IO_OUTPUT,	/* Userdata for default I/O output file. */
  GCROOT_MAX
} GCRootID;

#define basemt_it(g, it)	((g)->gcroot[GCROOT_BASEMT+~(it)])
#define basemt_obj(g, o)	((g)->gcroot[GCROOT_BASEMT+itypemap(o)])
#define mmname_str(g, mm)	(strref((g)->gcroot[GCROOT_MMNAME+(mm)]))

typedef struct GCState {
  MSize total;		/* Memory currently allocated. */
  MSize threshold;	/* Memory threshold. */
  uint8_t currentwhite;	/* Current white color. */
  uint8_t state;	/* GC state. */
  uint8_t nocdatafin;	/* No cdata finalizer called. */
  uint8_t unused2;
  MSize sweepstr;	/* Sweep position in string table. */
  GCRef root;		/* List of all collectable objects. */
  MRef sweep;		/* Sweep position in root list. */
  GCRef gray;		/* List of gray objects. */
  GCRef grayagain;	/* List of objects for atomic traversal. */
  GCRef weak;		/* List of weak tables (to be cleared). */
  GCRef mmudata;	/* List of userdata (to be finalized). */
  MSize stepmul;	/* Incremental GC step granularity. */
  MSize debt;		/* Debt (how much GC is behind schedule). */
  MSize estimate;	/* Estimate of memory actually in use. */
  MSize pause;		/* Pause between successive GC cycles. */
} GCState;

/* Global state, shared by all threads of a Lua universe. */
typedef struct global_State {
  GCRef *strhash;	/* String hash table (hash chain anchors). */
  MSize strmask;	/* String hash mask (size of hash table - 1). */
  MSize strnum;		/* Number of strings in hash table. */
  lua_Alloc allocf;	/* Memory allocator. */
  void *allocd;		/* Memory allocator data. */
  GCState gc;		/* Garbage collector. */
  SBuf tmpbuf;		/* Temporary buffer for string concatenation. */
  Node nilnode;		/* Fallback 1-element hash part (nil key and value). */
  GCstr strempty;	/* Empty string. */
  uint8_t stremptyz;	/* Zero terminator of empty string. */
  uint8_t hookmask;	/* Hook mask. */
  uint8_t dispatchmode;	/* Dispatch mode. */
  uint8_t vmevmask;	/* VM event mask. */
  GCRef mainthref;	/* Link to main thread. */
  TValue registrytv;	/* Anchor for registry. */
  TValue tmptv, tmptv2;	/* Temporary TValues. */
  GCupval uvhead;	/* Head of double-linked list of all open upvalues. */
  int32_t hookcount;	/* Instruction hook countdown. */
  int32_t hookcstart;	/* Start count for instruction hook counter. */
  lua_Hook hookf;	/* Hook function. */
  lua_CFunction wrapf;	/* Wrapper for C function calls. */
  lua_CFunction panic;	/* Called as a last resort for errors. */
  volatile int32_t vmstate;  /* VM state or current JIT code trace number. */
  BCIns bc_cfunc_int;	/* Bytecode for internal C function calls. */
  BCIns bc_cfunc_ext;	/* Bytecode for external C function calls. */
  GCRef jit_L;		/* Current JIT code lua_State or NULL. */
  MRef jit_base;	/* Current JIT code L->base. */
  MRef ctype_state;	/* Pointer to C type state. */
  GCRef gcroot[GCROOT_MAX];  /* GC roots. */
} global_State;

#define mainthread(g)	(&gcref(g->mainthref)->th)
#define niltv(L) \
  check_exp(tvisnil(&G(L)->nilnode.val), &G(L)->nilnode.val)
#define niltvg(g) \
  check_exp(tvisnil(&(g)->nilnode.val), &(g)->nilnode.val)

/* Hook management. Hook event masks are defined in lua.h. */
#define HOOK_EVENTMASK		0x0f
#define HOOK_ACTIVE		0x10
#define HOOK_ACTIVE_SHIFT	4
#define HOOK_VMEVENT		0x20
#define HOOK_GC			0x40
#define hook_active(g)		((g)->hookmask & HOOK_ACTIVE)
#define hook_enter(g)		((g)->hookmask |= HOOK_ACTIVE)
#define hook_entergc(g)		((g)->hookmask |= (HOOK_ACTIVE|HOOK_GC))
#define hook_vmevent(g)		((g)->hookmask |= (HOOK_ACTIVE|HOOK_VMEVENT))
#define hook_leave(g)		((g)->hookmask &= ~HOOK_ACTIVE)
#define hook_save(g)		((g)->hookmask & ~HOOK_EVENTMASK)
#define hook_restore(g, h) \
  ((g)->hookmask = ((g)->hookmask & HOOK_EVENTMASK) | (h))

/* Per-thread state object. */
struct lua_State {
  GCHeader;
  uint8_t dummy_ffid;	/* Fake FF_C for curr_funcisL() on dummy frames. */
  uint8_t status;	/* Thread status. */
  MRef glref;		/* Link to global state. */
  GCRef gclist;		/* GC chain. */
  TValue *base;		/* Base of currently executing function. */
  TValue *top;		/* First free slot in the stack. */
  MRef maxstack;	/* Last free slot in the stack. */
  MRef stack;		/* Stack base. */
  GCRef openupval;	/* List of open upvalues in the stack. */
  GCRef env;		/* Thread environment (table of globals). */
  void *cframe;		/* End of C stack frame chain. */
  MSize stacksize;	/* True stack size (incl. LJ_STACK_EXTRA). */
};

#define G(L)			(mref(L->glref, global_State))
#define registry(L)		(&G(L)->registrytv)

/* Macros to access the currently executing (Lua) function. */
#define curr_func(L)		(&gcref((L->base-1)->fr.func)->fn)
#define curr_funcisL(L)		(isluafunc(curr_func(L)))
#define curr_proto(L)		(funcproto(curr_func(L)))
#define curr_topL(L)		(L->base + curr_proto(L)->framesize)
#define curr_top(L)		(curr_funcisL(L) ? curr_topL(L) : L->top)

/* -- GC object definition and conversions -------------------------------- */

/* GC header for generic access to common fields of GC objects. */
typedef struct GChead {
  GCHeader;
  uint8_t unused1;
  uint8_t unused2;
  GCRef env;
  GCRef gclist;
  GCRef metatable;
} GChead;

/* The env field SHOULD be at the same offset for all GC objects. */
LJ_STATIC_ASSERT(offsetof(GChead, env) == offsetof(GCfuncL, env));
LJ_STATIC_ASSERT(offsetof(GChead, env) == offsetof(GCudata, env));

/* The metatable field MUST be at the same offset for all GC objects. */
LJ_STATIC_ASSERT(offsetof(GChead, metatable) == offsetof(GCtab, metatable));
LJ_STATIC_ASSERT(offsetof(GChead, metatable) == offsetof(GCudata, metatable));

/* The gclist field MUST be at the same offset for all GC objects. */
LJ_STATIC_ASSERT(offsetof(GChead, gclist) == offsetof(lua_State, gclist));
LJ_STATIC_ASSERT(offsetof(GChead, gclist) == offsetof(GCproto, gclist));
LJ_STATIC_ASSERT(offsetof(GChead, gclist) == offsetof(GCfuncL, gclist));
LJ_STATIC_ASSERT(offsetof(GChead, gclist) == offsetof(GCtab, gclist));

typedef union GCobj {
  GChead gch;
  GCstr str;
  GCupval uv;
  lua_State th;
  GCproto pt;
  GCfunc fn;
  GCcdata cd;
  GCtab tab;
  GCudata ud;
} GCobj;

/* Macros to convert a GCobj pointer into a specific value. */
#define gco2str(o)	check_exp((o)->gch.gct == ~LJ_TSTR, &(o)->str)
#define gco2uv(o)	check_exp((o)->gch.gct == ~LJ_TUPVAL, &(o)->uv)
#define gco2th(o)	check_exp((o)->gch.gct == ~LJ_TTHREAD, &(o)->th)
#define gco2pt(o)	check_exp((o)->gch.gct == ~LJ_TPROTO, &(o)->pt)
#define gco2func(o)	check_exp((o)->gch.gct == ~LJ_TFUNC, &(o)->fn)
#define gco2cd(o)	check_exp((o)->gch.gct == ~LJ_TCDATA, &(o)->cd)
#define gco2tab(o)	check_exp((o)->gch.gct == ~LJ_TTAB, &(o)->tab)
#define gco2ud(o)	check_exp((o)->gch.gct == ~LJ_TUDATA, &(o)->ud)

/* Macro to convert any collectable object into a GCobj pointer. */
#define obj2gco(v)	((GCobj *)(v))

/* -- TValue getters/setters ---------------------------------------------- */

#ifdef LUA_USE_ASSERT
#include "lj_gc.h"
#endif

/* Macros to test types. */
#define itype(o)	((o)->it)
#define tvisnil(o)	(itype(o) == LJ_TNIL)
#define tvisfalse(o)	(itype(o) == LJ_TFALSE)
#define tvistrue(o)	(itype(o) == LJ_TTRUE)
#define tvisbool(o)	(tvisfalse(o) || tvistrue(o))
#if LJ_64
#define tvislightud(o)	(((int32_t)itype(o) >> 15) == -2)
#else
#define tvislightud(o)	(itype(o) == LJ_TLIGHTUD)
#endif
#define tvisstr(o)	(itype(o) == LJ_TSTR)
#define tvisfunc(o)	(itype(o) == LJ_TFUNC)
#define tvisthread(o)	(itype(o) == LJ_TTHREAD)
#define tvisproto(o)	(itype(o) == LJ_TPROTO)
#define tviscdata(o)	(itype(o) == LJ_TCDATA)
#define tvistab(o)	(itype(o) == LJ_TTAB)
#define tvisudata(o)	(itype(o) == LJ_TUDATA)
#define tvisnumber(o)	(itype(o) <= LJ_TISNUM)
#define tvisint(o)	(LJ_DUALNUM && itype(o) == LJ_TISNUM)
#define tvisnum(o)	(itype(o) < LJ_TISNUM)

#define tvistruecond(o)	(itype(o) < LJ_TISTRUECOND)
#define tvispri(o)	(itype(o) >= LJ_TISPRI)
#define tvistabud(o)	(itype(o) <= LJ_TISTABUD)  /* && !tvisnum() */
#define tvisgcv(o)	((itype(o) - LJ_TISGCV) > (LJ_TNUMX - LJ_TISGCV))

/* Special macros to test numbers for NaN, +0, -0, +1 and raw equality. */
#define tvisnan(o)	((o)->n != (o)->n)
#if LJ_64
#define tviszero(o)	(((o)->u64 << 1) == 0)
#else
#define tviszero(o)	(((o)->u32.lo | ((o)->u32.hi << 1)) == 0)
#endif
#define tvispzero(o)	((o)->u64 == 0)
#define tvismzero(o)	((o)->u64 == U64x(80000000,00000000))
#define tvispone(o)	((o)->u64 == U64x(3ff00000,00000000))
#define rawnumequal(o1, o2)	((o1)->u64 == (o2)->u64)

/* Macros to convert type ids. */
#if LJ_64
#define itypemap(o) \
  (tvisnumber(o) ? ~LJ_TNUMX : tvislightud(o) ? ~LJ_TLIGHTUD : ~itype(o))
#else
#define itypemap(o)	(tvisnumber(o) ? ~LJ_TNUMX : ~itype(o))
#endif

/* Macros to get tagged values. */
#define gcval(o)	(gcref((o)->gcr))
#define boolV(o)	check_exp(tvisbool(o), (LJ_TFALSE - (o)->it))
#if LJ_64
#define lightudV(o) \
  check_exp(tvislightud(o), (void *)((o)->u64 & U64x(00007fff,ffffffff)))
#else
#define lightudV(o)	check_exp(tvislightud(o), gcrefp((o)->gcr, void))
#endif
#define gcV(o)		check_exp(tvisgcv(o), gcval(o))
#define strV(o)		check_exp(tvisstr(o), &gcval(o)->str)
#define funcV(o)	check_exp(tvisfunc(o), &gcval(o)->fn)
#define threadV(o)	check_exp(tvisthread(o), &gcval(o)->th)
#define protoV(o)	check_exp(tvisproto(o), &gcval(o)->pt)
#define cdataV(o)	check_exp(tviscdata(o), &gcval(o)->cd)
#define tabV(o)		check_exp(tvistab(o), &gcval(o)->tab)
#define udataV(o)	check_exp(tvisudata(o), &gcval(o)->ud)
#define numV(o)		check_exp(tvisnum(o), (o)->n)
#define intV(o)		check_exp(tvisint(o), (int32_t)(o)->i)

/* Macros to set tagged values. */
#define setitype(o, i)		((o)->it = (i))
#define setnilV(o)		((o)->it = LJ_TNIL)
#define setboolV(o, x)		((o)->it = LJ_TFALSE-(uint32_t)(x))

static LJ_AINLINE void setlightudV(TValue *o, void *p)
{
#if LJ_64
  o->u64 = (uint64_t)p | (((uint64_t)0xffff) << 48);
#else
  setgcrefp(o->gcr, p); setitype(o, LJ_TLIGHTUD);
#endif
}

#if LJ_64
#define checklightudptr(L, p) \
  (((uint64_t)(p) >> 47) ? (lj_err_msg(L, LJ_ERR_BADLU), NULL) : (p))
#define setcont(o, f) \
  ((o)->u64 = (uint64_t)(void *)(f) - (uint64_t)lj_vm_asm_begin)
#else
#define checklightudptr(L, p)	(p)
#define setcont(o, f)		setlightudV((o), (void *)(f))
#endif

#define tvchecklive(L, o) \
  UNUSED(L), lua_assert(!tvisgcv(o) || \
  ((~itype(o) == gcval(o)->gch.gct) && !isdead(G(L), gcval(o))))

static LJ_AINLINE void setgcV(lua_State *L, TValue *o, GCobj *v, uint32_t itype)
{
  setgcref(o->gcr, v); setitype(o, itype); tvchecklive(L, o);
}

#define define_setV(name, type, tag) \
static LJ_AINLINE void name(lua_State *L, TValue *o, type *v) \
{ \
  setgcV(L, o, obj2gco(v), tag); \
}
define_setV(setstrV, GCstr, LJ_TSTR)
define_setV(setthreadV, lua_State, LJ_TTHREAD)
define_setV(setprotoV, GCproto, LJ_TPROTO)
define_setV(setfuncV, GCfunc, LJ_TFUNC)
define_setV(setcdataV, GCcdata, LJ_TCDATA)
define_setV(settabV, GCtab, LJ_TTAB)
define_setV(setudataV, GCudata, LJ_TUDATA)

#define setnumV(o, x)		((o)->n = (x))
#define setnanV(o)		((o)->u64 = U64x(fff80000,00000000))
#define setpinfV(o)		((o)->u64 = U64x(7ff00000,00000000))
#define setminfV(o)		((o)->u64 = U64x(fff00000,00000000))

static LJ_AINLINE void setintV(TValue *o, int32_t i)
{
#if LJ_DUALNUM
  o->i = (uint32_t)i; setitype(o, LJ_TISNUM);
#else
  o->n = (lua_Number)i;
#endif
}

static LJ_AINLINE void setint64V(TValue *o, int64_t i)
{
  if (LJ_DUALNUM && LJ_LIKELY(i == (int64_t)(int32_t)i))
    setintV(o, (int32_t)i);
  else
    setnumV(o, (lua_Number)i);
}

#if LJ_64
#define setintptrV(o, i)	setint64V((o), (i))
#else
#define setintptrV(o, i)	setintV((o), (i))
#endif

/* Copy tagged values. */
static LJ_AINLINE void copyTV(lua_State *L, TValue *o1, const TValue *o2)
{
  *o1 = *o2; tvchecklive(L, o1);
}

/* -- Number to integer conversion ---------------------------------------- */

#if LJ_SOFTFP
LJ_ASMF int32_t lj_vm_tobit(double x);
#endif

static LJ_AINLINE int32_t lj_num2bit(lua_Number n)
{
#if LJ_SOFTFP
  return lj_vm_tobit(n);
#else
  TValue o;
  o.n = n + 6755399441055744.0;  /* 2^52 + 2^51 */
  return (int32_t)o.u32.lo;
#endif
}

#if LJ_TARGET_X86 && !defined(__SSE2__)
#define lj_num2int(n)   lj_num2bit((n))
#else
#define lj_num2int(n)   ((int32_t)(n))
#endif

static LJ_AINLINE uint64_t lj_num2u64(lua_Number n)
{
#ifdef _MSC_VER
  if (n >= 9223372036854775808.0)  /* They think it's a feature. */
    return (uint64_t)(int64_t)(n - 18446744073709551616.0);
  else
#endif
    return (uint64_t)n;
}

static LJ_AINLINE int32_t numberVint(cTValue *o)
{
  if (LJ_LIKELY(tvisint(o)))
    return intV(o);
  else
    return lj_num2int(numV(o));
}

static LJ_AINLINE lua_Number numberVnum(cTValue *o)
{
  if (LJ_UNLIKELY(tvisint(o)))
    return (lua_Number)intV(o);
  else
    return numV(o);
}

/* -- Miscellaneous object handling --------------------------------------- */

/* Names and maps for internal and external object tags. */
LJ_DATA const char *const lj_obj_typename[1+LUA_TCDATA+1];
LJ_DATA const char *const lj_obj_itypename[~LJ_TNUMX+1];

#define lj_typename(o)	(lj_obj_itypename[itypemap(o)])

/* Compare two objects without calling metamethods. */
LJ_FUNC int lj_obj_equal(cTValue *o1, cTValue *o2);

#endif
