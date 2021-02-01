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

#include <MBExtender/CodeStream.h>

#include <memory>
#include <vector>

#include "TrampolineGenerator.h"

/// <summary>
/// Provides facilities for intercepting functions.
/// </summary>
class FuncInterceptor {
  public:
    FuncInterceptor(std::shared_ptr<MBX::CodeStream> stream, std::shared_ptr<CodeAllocator> allocator)
            : stream_{std::move(stream)}, trampolineGen_{std::move(allocator)} {}

    ~FuncInterceptor() { restoreAll(); }

    /// <summary>
    /// Intercepts a function, routing all calls to it to another function and returning a pointer which can be used to call the original code.
    /// </summary>
    /// <param name="func">The function to intercept.</param>
    /// <param name="newFunc">The new function to redirect callers to.</param>
    /// <returns>A pointer which can be used to call the original function, or <c>NULL</c> on failure.</returns>
    template <class T>
    T intercept(T func, T newFunc) {
        return reinterpret_cast<T>(interceptImpl(reinterpret_cast<void *>(func), reinterpret_cast<void *>(newFunc)));
    }

    /// <summary>
    /// Restores all intercepted functions.
    /// </summary>
    void restoreAll();

  private:
    enum class InterceptionStrategy {
        Thunk,       // The function originally just did a jump; previousPtr is the previous address of the jump
        Trampoline,  // The function was a normal function; previousPtr is the address of the trampoline
    };
    struct InterceptedFunction {
        InterceptionStrategy strategy;
        void *function;
        void *previousPtr;
        size_t overwrittenCodeSize;
    };
    std::vector<InterceptedFunction> intercepts_;

    void *interceptImpl(void *func, void *newFunc);

    std::shared_ptr<MBX::CodeStream> stream_;  // Stream used to write code
    TrampolineGenerator trampolineGen_;        // Function trampoline generator
};
