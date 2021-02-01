//-----------------------------------------------------------------------------
// Copyright (c) 2017, The Platinum Team
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

#include "GraphicsExtension.h"

#include <MBExtender/MBExtender.h>
#include <TorqueLib/TypeInfo.h>

#include <TorqueLib/game/marble/marble.h>

MBX_MODULE(MemoizeCamera);

namespace
{
	bool Memoized; // true if the below values are valid
	F32 MemoizedPos;
	MatrixF MemoizedMat;
}

MBX_OVERRIDE_MEMBERFN(void, TGE::Marble::getCameraTransform, (TGE::Marble *thisPtr, F32 *pos, MatrixF *mat), originalGetCameraTransform)
{
	if (!Memoized)
	{
		originalGetCameraTransform(thisPtr, &MemoizedPos, &MemoizedMat);
		Memoized = true;
	}
	*pos = MemoizedPos;
	*mat = MemoizedMat;
}

MBX_OVERRIDE_MEMBERFN(void, TGE::SceneObject::setTransform, (TGE::SceneObject *thisptr, const MatrixF &transform), originalSetTransform)
{
	if (TGE::TypeInfo::manual_dynamic_cast<TGE::Marble *>(thisptr, &TGE::TypeInfo::SceneObject, &TGE::TypeInfo::Marble, 0)) {
		//It's a marble, update camera transform
		Memoized = false;
	}
	originalSetTransform(thisptr, transform);
}

MBX_ON_CLIENT_PROCESS(clearCameraCache, (uint32_t delta))
{
	Memoized = false;
}
