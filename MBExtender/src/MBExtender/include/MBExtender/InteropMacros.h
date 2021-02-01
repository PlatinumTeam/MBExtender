//-----------------------------------------------------------------------------
// Copyright (c) 2020 The Platinum Team
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//-----------------------------------------------------------------------------

#pragma once

#if !defined(_WIN32) && !defined(__APPLE__)
#    error "Only Windows and MacOS are supported."
#endif
#if !defined(_MSC_VER) && !defined(__clang__)
#    error "Only MSVC and Clang are supported."
#endif

#include <cstdint>

// Append arguments onto an argument list
#if defined(__clang__)
#    define MBX_CONCAT(...) , ##__VA_ARGS__
#elif defined(_MSC_VER)
#    define MBX_CONCAT(...) , __VA_ARGS__
#endif

// Return the first argument
#define MBX_FIRST(x, ...) x

// Token pasting
#define MBX_PASTE2(x, y) x##y
#define MBX_PASTE(x, y) MBX_PASTE2(x, y)

// Temporary names
#define MBX_TEMP(x) MBX_PASTE(x, __LINE__)

// stdcall
#if defined(_WIN32)
#    define MBX_STDCALL __stdcall
#elif defined(__APPLE__)
#    define MBX_STDCALL
#endif

// thiscall
#if defined(_WIN32)
#    define MBX_THISCALL __thiscall
#elif defined(__APPLE__)
#    define MBX_THISCALL
#endif

// fastcall
// MB doesn't actually use fastcall on Windows, so this only affects Mac
#if defined(_WIN32)
#    define MBX_FASTCALL
#elif defined(__APPLE__)
#    define MBX_FASTCALL __attribute__((regparm(3)))
#endif

// Per-platform compile-time address resolution system
// Most macros which have variadic argument lists accept one or more platform-specific addresses
// Addresses ending in _win will only be used on Windows (e.g. 0x123456_win)
// Addresses ending in _mac will only be used on Mac (e.g. 0x123456_mac)
// Addresses without any suffix will be used on every platform (e.g. 0x123456)
// MBX::Address<...>::value returns the first address which is valid for the current platform, or -1 if no address was found
// MBX_ADDRESS(type, ...) macro generates a reinterpret_cast for an address
// MBX_ASSERT_ADDRESS(name, ...) macro generates a static_assert which checks that the address is found
namespace {
#if defined(_WIN32)
constexpr uintptr_t operator"" _win(unsigned long long int addr) {
    return static_cast<uintptr_t>(addr);
}
constexpr uintptr_t operator"" _mac(unsigned long long int) {
    return static_cast<uintptr_t>(-1);
}
#elif defined(__APPLE__)
constexpr uintptr_t operator"" _mac(unsigned long long int addr) {
    return static_cast<uintptr_t>(addr);
}
constexpr uintptr_t operator"" _win(unsigned long long int) {
    return static_cast<uintptr_t>(-1);
}
#endif
}  // namespace

namespace MBX {
template <uintptr_t...>
struct Address {
    static constexpr uintptr_t value = static_cast<uintptr_t>(-1);
};

template <uintptr_t Addr, uintptr_t... Args>
struct Address<Addr, Args...> {
    static constexpr uintptr_t value = (Addr != static_cast<uintptr_t>(-1)) ? Addr : Address<Args...>::value;
};
}  // namespace MBX

#define MBX_ADDRESS(type, ...) reinterpret_cast<type>(::MBX::Address<__VA_ARGS__>::value)
#define MBX_ASSERT_ADDRESS(name, ...)                                               \
    static_assert(::MBX::Address<__VA_ARGS__>::value != static_cast<uintptr_t>(-1), \
                  "This platform does not have an address defined for " #name)

// Macro for quickly declaring a function pointer that points to a static address
#define FN(rettype, name, args, ...)                                       \
    MBX_ASSERT_ADDRESS(name, __VA_ARGS__);                                 \
    namespace {                                                            \
    rettype(*const name) args = MBX_ADDRESS(rettype(*) args, __VA_ARGS__); \
    }

// Macro for declaring a function pointer which uses the stdcall convention
#define STDCALLFN(rettype, name, args, ...)                                                        \
    MBX_ASSERT_ADDRESS(name, __VA_ARGS__);                                                         \
    namespace {                                                                                    \
    rettype(MBX_STDCALL *const name) args = MBX_ADDRESS(rettype(MBX_STDCALL *) args, __VA_ARGS__); \
    }

// Macro for declaring a function pointer which uses the fastcall convention on Mac
#define FASTCALLFN(rettype, name, args, ...)                                                         \
    MBX_ASSERT_ADDRESS(name, __VA_ARGS__);                                                           \
    namespace {                                                                                      \
    rettype(MBX_FASTCALL *const name) args = MBX_ADDRESS(rettype(MBX_FASTCALL *) args, __VA_ARGS__); \
    }

// Macros for declaring overloaded function pointers
//
// Usage example:
// OVERLOAD_PTR {
//     OVERLOAD_FN(int, (int x, int y), 0x12345);
//     OVERLOAD_FN(int, (int z), 0x23456);
// } foo;
#define OVERLOAD_PTR static struct

#define OVERLOAD_FN_(rettype, args, ...)                                        \
    MBX_ASSERT_ADDRESS(overload, __VA_ARGS__);                                  \
                                                                                \
  private:                                                                      \
    typedef rettype(*MBX_PASTE(t_, MBX_FIRST(__VA_ARGS__))) args;               \
                                                                                \
  public:                                                                       \
    inline operator MBX_PASTE(t_, MBX_FIRST(__VA_ARGS__))() const {             \
        return MBX_ADDRESS(MBX_PASTE(t_, MBX_FIRST(__VA_ARGS__)), __VA_ARGS__); \
    }
// Huge hack to make this work with Intellisense
// If Intellisense is parsing the file, we show it that a () operator exists so that it can help with parameters
// However, when the compiler actually reads it, it'll only see a typecast operator
#if defined(__INTELLISENSE__)
#    define OVERLOAD_FN(rettype, args, ...)      \
        OVERLOAD_FN_(rettype, args, __VA_ARGS__) \
        inline rettype operator() args const {}
#else
#    define OVERLOAD_FN(rettype, args, ...) OVERLOAD_FN_(rettype, args, __VA_ARGS__)
#endif

namespace MBX {
class Placeholder {
    Placeholder() = delete;
};
}  // namespace MBX

// Must be at the beginning of every class definition that uses bridging macros (e.g. MEMBERFN)
// Defines things necessary for the other class-related macros to work.
// Defines a static alloc() function, which allocates an uninitialized instance of the object using operator new.
// Defines a static create() function, which is a shortcut for calling alloc() and then a constructor.
#define BRIDGE_CLASS(name)                                                             \
  private:                                                                             \
    typedef name self_type;                                                            \
    name() = delete;                                                                   \
    name(const name &) = delete;                                                       \
    name(name &&) = delete;                                                            \
    name &operator=(const name &) = delete;                                            \
    name &operator=(name &&) = delete;                                                 \
    void ctor(::MBX::Placeholder &); /* Necessary on MacOS for some reason */          \
  public:                                                                              \
    static name *alloc() { return static_cast<name *>(::operator new(sizeof(name))); } \
    template <class... A>                                                              \
    static name *create(A... args) {                                                   \
        name *x = alloc();                                                             \
        x->ctor(args...);                                                              \
        return x;                                                                      \
    }                                                                                  \
                                                                                       \
  private:

// Defines an undefined virtual function
#define UNDEFVIRT(name) virtual void z_undef_##name##_() = 0

namespace MBX {
// Helper struct for resolving address overloads
template <class T>
struct Overload {
    typedef T type;
};
}  // namespace MBX

// Declares an address constant which can be looked up by name and type
// (used by the other macros - generally you don't need to use this directly)
#define MBX_DECLARE_ADDRESS(name, type, ...) \
    MBX_ASSERT_ADDRESS(name, __VA_ARGS__);   \
    static auto name##_Address(::MBX::Overload<type>)->type { return MBX_ADDRESS(type, __VA_ARGS__); }

// Utility macros for bridging functions when a pointer can't be used
#if defined(__clang__)
#    define MBX_TRAMPOLINE_ATTR __attribute__((__naked__, __noinline__))
#    define MBX_JUMP(target) __asm__ __volatile__("jmpl *%0 \n\t" : : "a"((target)))
#elif defined(_MSC_VER)
// warning C4731: frame pointer register 'ebp' modified by inline assembly code
// clang-format off
#    pragma warning(disable : 4731)
#    define MBX_TRAMPOLINE_ATTR __declspec(noinline)
#    define MBX_JUMP(target)       \
        void *target_ = (target);  \
        __asm { mov eax, target_ } \
        __asm { mov esp, ebp }     \
        __asm { pop ebp }          \
        __asm { jmp eax }
// clang-format on
#endif

// Defines a class method bridge
#define MEMBERFN(rettype, name, args, ...)                                                        \
    MBX_DECLARE_ADDRESS(name, rettype(MBX_THISCALL *)(self_type * MBX_CONCAT args), __VA_ARGS__); \
    MBX_TRAMPOLINE_ATTR rettype name args { MBX_JUMP(MBX_ADDRESS(void *, __VA_ARGS__)); }

// Defines a static method bridge
#define STATICFN(rettype, name, args, ...)                   \
    MBX_DECLARE_ADDRESS(name, rettype(*) args, __VA_ARGS__); \
    MBX_TRAMPOLINE_ATTR static rettype name args { MBX_JUMP(MBX_ADDRESS(void *, __VA_ARGS__)); }

// Defines a ctor() function for a constructor
#define CONSTRUCTOR(args, ...) MEMBERFN(void, ctor, args, __VA_ARGS__)

// Defines a destructor wrapper (don't use this for virtual classes, just use MEMBERFN)
#define DESTRUCTOR(clazz, ...)             \
    MEMBERFN(void, dtor, (), __VA_ARGS__); \
    MBX_TRAMPOLINE_ATTR ~clazz() { MBX_JUMP(MBX_ADDRESS(void *, __VA_ARGS__)); }

// Defines a virtual destructor with an address
// (Just declare a virtual destructor normally if you don't have an address)
#define DESTRUCTOR_VIRT(clazz, ...)        \
    MEMBERFN(void, dtor, (), __VA_ARGS__); \
    virtual ~clazz()

// Defines a global variable
#define GLOBALVAR(type, name, ...)         \
    MBX_ASSERT_ADDRESS(name, __VA_ARGS__); \
    static type &name = *MBX_ADDRESS(type *, __VA_ARGS__)

// Defines a function which uses the thiscall convention
#if defined(__clang__)
#    define THISFN(rettype, name, args) static rettype MBX_THISCALL name args
#elif defined(_MSC_VER)
// clang-format off
#    define THISFN(rettype, name, args)                                                         \
        static rettype __fastcall name##_this_impl(void *MBX_CONCAT args);                      \
        namespace {                                                                             \
        __declspec(naked) void name##_this_trampoline args {                                    \
            __asm { mov edx, ecx }                                                              \
            __asm { jmp name##_this_impl }                                                      \
        }                                                                                       \
        }                                                                                       \
        typedef rettype(MBX_THISCALL *name##_this_ptr) args;                                    \
        namespace {                                                                             \
        const name##_this_ptr name = reinterpret_cast<name##_this_ptr>(name##_this_trampoline); \
        }                                                                                       \
        static rettype __fastcall name##_this_impl(void *MBX_CONCAT args)
// clang-format on
#endif

// Defines a getter for a class field
#define GETTERFN(type, name, ...)                                                     \
    MBX_ASSERT_ADDRESS(name, __VA_ARGS__);                                            \
    type const &name() const {                                                        \
        return *reinterpret_cast<type const *>(reinterpret_cast<const char *>(this) + \
                                               ::MBX::Address<__VA_ARGS__>::value);   \
    }

// Defines a setter for a class field
#define SETTERFN(type, name, ...)                                                                                    \
    MBX_ASSERT_ADDRESS(name, __VA_ARGS__);                                                                           \
    void name(type newValue) {                                                                                       \
        *reinterpret_cast<type *>(reinterpret_cast<uint32_t>(this) + ::MBX::Address<__VA_ARGS__>::value) = newValue; \
    }

// Defines a reference to a field (faster than getter/setter methods!)
#define FIELD(type, name, ...)                                                                                 \
    MBX_ASSERT_ADDRESS(name, __VA_ARGS__);                                                                     \
    type &name() {                                                                                             \
        return *reinterpret_cast<type *>(reinterpret_cast<char *>(this) + ::MBX::Address<__VA_ARGS__>::value); \
    }                                                                                                          \
    type const &name() const {                                                                                 \
        return *reinterpret_cast<type const *>(reinterpret_cast<const char *>(this) +                          \
                                               ::MBX::Address<__VA_ARGS__>::value);                            \
    }
