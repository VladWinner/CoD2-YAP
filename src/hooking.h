#pragma once
#include <safetyhook.hpp>
#include "MemoryMgr.h"

extern std::vector<std::unique_ptr<SafetyHookInline>> g_inlineHooks;
extern std::vector<std::unique_ptr<SafetyHookMid>> g_midHooks;
template<typename T, typename Fn>
SafetyHookInline* CreateInlineHook(T target, Fn destination, SafetyHookInline::Flags flags = SafetyHookInline::Default) {
    if (!target)
        return NULL;
    auto hook = std::make_unique<SafetyHookInline>(safetyhook::create_inline(target, destination, flags));
    auto* ptr = hook.get();
    g_inlineHooks.push_back(std::move(hook));
    return ptr;
}

template<typename T>
SafetyHookMid* CreateMidHook(T target, safetyhook::MidHookFn destination, safetyhook::MidHook::Flags flags = safetyhook::MidHook::Default) {
    if (!target)
        return NULL;
    auto hook = std::make_unique<SafetyHookMid>(safetyhook::create_mid(target, destination, flags));
    auto* ptr = hook.get();
    g_midHooks.push_back(std::move(hook));
    return ptr;
}
static double decodeX87(const uint8_t* bytes) {
    uint64_t mantissa = *(uint64_t*)bytes;
    uint16_t sign_exp = *(uint16_t*)(bytes + 8);

    int sign = (sign_exp >> 15) & 1;
    int exponent = sign_exp & 0x7FFF;

    if (exponent == 0) {
        if (mantissa == 0) return sign ? -0.0 : 0.0;
        return 0.0;
    }
    if (exponent == 0x7FFF) {
        return sign ? -INFINITY : INFINITY;
    }

    int bias = 16383;
    int exp_double = exponent - bias + 1023;

    if (exp_double <= 0) return 0.0;
    if (exp_double >= 0x7FF) return sign ? -INFINITY : INFINITY;

    uint64_t mantissa_52bit = (mantissa >> 11) & 0x000FFFFFFFFFFFFF;
    uint64_t double_bits = ((uint64_t)sign << 63) |
        ((uint64_t)exp_double << 52) |
        mantissa_52bit;

    double result;
    memcpy(&result, &double_bits, 8);
    return result;
}

// Encode double to x87 80-bit extended precision
static void encodeX87(uint8_t* bytes, double value) {
    uint64_t double_bits;
    memcpy(&double_bits, &value, 8);

    int sign = (double_bits >> 63) & 1;
    int exp_double = (double_bits >> 52) & 0x7FF;
    uint64_t mantissa_52bit = double_bits & 0x000FFFFFFFFFFFFF;

    // Handle special cases
    if (exp_double == 0) {
        // Zero or denormalized
        memset(bytes, 0, 10);
        if (sign) bytes[9] = 0x80;
        return;
    }
    if (exp_double == 0x7FF) {
        // Infinity or NaN
        *(uint64_t*)bytes = 0x8000000000000000ULL;
        *(uint16_t*)(bytes + 8) = 0x7FFF | (sign << 15);
        return;
    }

    // Convert exponent
    int exp_x87 = exp_double - 1023 + 16383;

    // Add implicit integer bit for x87
    uint64_t mantissa_64bit = 0x8000000000000000ULL | (mantissa_52bit << 11);

    *(uint64_t*)bytes = mantissa_64bit;
    *(uint16_t*)(bytes + 8) = (uint16_t)exp_x87 | (sign << 15);
}

#pragma pack(push, 1)
struct FXSAVE_AREA {
    uint16_t fcw;
    uint16_t fsw;
    uint8_t  ftw;
    uint8_t  reserved1;
    uint16_t fop;
    uint32_t fip;
    uint16_t fcs;
    uint16_t reserved2;
    uint32_t fdp;
    uint16_t fds;
    uint16_t reserved3;
    uint32_t mxcsr;
    uint32_t mxcsr_mask;

    union {
        struct {
            uint8_t st0[16];
            uint8_t st1[16];
            uint8_t st2[16];
            uint8_t st3[16];
            uint8_t st4[16];
            uint8_t st5[16];
            uint8_t st6[16];
            uint8_t st7[16];
        };
        uint8_t st[8][16];
    };

    uint8_t padding[352];
};
#pragma pack(pop)

class X87Context {
private:
    alignas(16) FXSAVE_AREA fpu_state;

public:
    X87Context() {
        _fxsave(&fpu_state);
    }

    // Read ST(n) - returns decoded double
    double readST(int n) const {
        if (n < 0 || n > 7) return 0.0;
        return decodeX87(fpu_state.st[n]);
    }

    // Write ST(n) - sets from double
    void writeST(int n, double value) {
        if (n < 0 || n > 7) return;
        encodeX87(fpu_state.st[n], value);
    }

    // Apply modified state back to FPU
    void apply() {
        _fxrstor(&fpu_state);
    }

    // Print all for debugging
    void printAllST() const {
        printf("=== X87 FPU Register Dump ===\n");
        for (int i = 0; i < 8; i++) {
            printf("ST(%d): ", i);
            for (int j = 0; j < 10; j++) {
                printf("%02X ", fpu_state.st[i][j]);
            }
            printf(" = %.17f\n", decodeX87(fpu_state.st[i]));
        }
    }
};
