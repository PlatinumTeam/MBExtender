//-----------------------------------------------------------------------------
// TypeInfo.h
//
// Copyright (c) 2015 The Platinum Team
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

#include <cstddef>
#include <MBExtender/InteropMacros.h>

namespace TGE {
	namespace TypeInfo {
		typedef void * ClassTypeInfo;

		//Type infos
		GLOBALVAR(ClassTypeInfo, SimObject, 0x656100_win, 0x2de080_mac);
		GLOBALVAR(ClassTypeInfo, NetObject, 0x65e8d0_win, 0x2e1680_mac);
		GLOBALVAR(ClassTypeInfo, SceneObject, 0x65E8F0_win, 0x2E15A0_mac);
		GLOBALVAR(ClassTypeInfo, ShapeBase, 0x663840_win, 0x2DEE80_mac);
		GLOBALVAR(ClassTypeInfo, Marble, 0x663860_win, 0x2E2180_mac);

		//Stealing MarbleBlast's dynamic cast right from underneath them
#ifdef _WIN32
		FN(void *, __dynamic_cast, (void *src, int unknown, ClassTypeInfo *src_type, ClassTypeInfo *dst_type, ptrdiff_t src2dst), 0x61F30C_win);
#else
		FN(void *, __dynamic_cast, (void *src, ClassTypeInfo *src_type, ClassTypeInfo *dst_type, ptrdiff_t src2dst), 0x34D83C_mac);
#endif
		/**
		 * Dynamic cast from F to T of an object, using the typeinfo pointers for
		 * those types, using MB's dynamic cast so we can actually convert them.
		 * @param src The source pointer to convert
		 * @param src_type A pointer to the __class_type_info for the source pointer
		 * @param dst_type A pointer to the __class_type_info for the destination pointer
		 * @param src2dst Pointer offset for virtual inheritence, usually 0
		 * @return The casted object, or NULL if it cannot be casted.
		 * @example TGE::TypeInfo::manual_dynamic_cast<TGE::Marble *>(thisptr, &TGE::TypeInfo::SimObject, &TGE::TypeInfo::Marble, 0);
		 */
		template <typename T, typename F>
		static inline T manual_dynamic_cast(F src, ClassTypeInfo *src_type, ClassTypeInfo *dst_type, ptrdiff_t src2dst) {
			//Don't call this on null pointers
			if (src == NULL)
				return NULL;

			//Please forgive me.
#ifdef _WIN32
			return static_cast<T>(__dynamic_cast(static_cast<void *>(src), 0, src_type, dst_type, src2dst));
#else
			return static_cast<T>(__dynamic_cast(static_cast<void *>(src), src_type, dst_type, src2dst));
#endif
		}
	}
}
