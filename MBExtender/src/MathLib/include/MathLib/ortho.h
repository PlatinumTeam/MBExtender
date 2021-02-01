//-----------------------------------------------------------------------------
// Copyright(c) 2015 The Platinum Team
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//-----------------------------------------------------------------------------

#pragma once

#include <TorqueLib/math/mPoint3.h>

///A 3-component (right, back, down) orthagonal matrix.
struct OrthoF {
    Point3F right;
    Point3F back;
    Point3F down;

    OrthoF() {
        right = Point3F();
        back = Point3F();
        down = Point3F();
    }
    OrthoF(const F32 &mod) {
        right = Point3F(mod);
        back = Point3F(mod);
        down = Point3F(mod);
    }

    OrthoF(const Point3F &right, const Point3F &back, const Point3F &down) {
        this->right = right;
        this->back = back;
        this->down = down;
    }

    OrthoF(const MatrixF &mat) {
        right = mat.getColumn3F(0);
        back = mat.getColumn3F(1);
        down = mat.getColumn3F(2);
    }

    void set(MatrixF &mat) const {
        mat.setColumn(0, right);
        mat.setColumn(1, back);
        mat.setColumn(2, down);
        mat.setColumn(3, Point4F(0, 0, 0, 1));
    }

    bool operator==(const OrthoF &other) { return right == other.right && back == other.back && down == other.down; }

    bool operator!=(const OrthoF &other) { return !operator==(other); }
};
