//-----------------------------------------------------------------------------
// InteriorTriangleList.h
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

#include "TriangleList.h"

#include <TorqueLib/interior/interior.h>

class InteriorTriangleList : public TriangleList {
	/**
	 * Load triangle data from an Interior
	 * @param interior The Interior which will be referenced
	 * @param mat      The MaterialList which is used on the interior
	 * @param itr      The draw iterator containing section and vertex data
	 */
	void loadFromInterior(TGE::Interior *interior, TGE::MaterialList *mat, TGE::ItrFastDetail *itr, bool averageNormals);

public:
	/**
	 * Create a triangle list from an Interior
	 * @param interior The Interior which will be referenced
	 * @param mat      The MaterialList which is used on the interior
	 * @param itr      The draw iterator containing section and vertex data
	 */
	InteriorTriangleList(TGE::Interior *interior, TGE::MaterialList *mat, TGE::ItrFastDetail *itr, bool averageNormals);
};
