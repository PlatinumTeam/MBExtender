//-----------------------------------------------------------------------------
// Copyright (c) 2012 GarageGames, LLC
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
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//-----------------------------------------------------------------------------

// NOTE: methods on the EaseF convenience class

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif // _CRT_SECURE_NO_WARNINGS

#include "mMath.h"
#include "mEase.h"

#include <cstdio>

EaseF::EaseF()
{
	dir = 0;
	type = 0;
	param[0] = param[1] = -1.0f;
}

EaseF::EaseF(const EaseF &ease)
{
	this->dir = ease.dir;
	this->type = ease.type;
	this->param[0] = ease.param[0];
	this->param[1] = ease.param[1];
}

EaseF::EaseF(const S32 dir, const S32 type)
{
	this->dir = dir;
	this->type = type;
	this->param[0] = this->param[1] = -1.0f;
}

EaseF::EaseF(const S32 dir, const S32 type, F32 param[2])
{
	this->dir = dir;
	this->type = type;
	this->param[0] = param[0];
	this->param[1] = param[1];
}

void EaseF::set(const S32 dir, const S32 type)
{
	this->dir = dir;
	this->type = type;
	this->param[0] = this->param[1] = -1.0f;
}

void EaseF::set(const S32 dir, const S32 type, F32 param[2])
{
	this->dir = dir;
	this->type = type;
	this->param[0] = param[0];
	this->param[1] = param[1];
}

void EaseF::set(const S32 dir, const S32 type, F32 param0, F32 param1)
{
	this->dir = dir;
	this->type = type;
	this->param[0] = param0;
	this->param[1] = param1;
}

void EaseF::set(const char *s)
{
   sscanf(s,"%d %d %f %f",&dir,&type,&param[0],&param[1]);
}

F32 EaseF::getValue(F32 t, F32 b, F32 c, F32 d) const
{
	F32 value = 0;

	if (type == Ease::Linear)
	{
		value = mLinearTween(t,b, c, d);
	}
	else if (type == Ease::Quadratic)
	{
		if (dir == Ease::In)
		   value = mEaseInQuad(t,b, c, d);
		else if (dir == Ease::Out)
		   value = mEaseOutQuad(t,b, c, d);
		else if (dir == Ease::InOut)
		   value = mEaseInOutQuad(t,b, c, d);
	}
	else if (type == Ease::Cubic)
	{
		if (dir == Ease::In)
		   value = mEaseInCubic(t,b, c, d);
		else if (dir == Ease::Out)
		   value = mEaseOutCubic(t,b, c, d);
		else if (dir == Ease::InOut)
		   value = mEaseInOutCubic(t,b, c, d);
	}
	else if (type == Ease::Quartic)
	{
		if (dir == Ease::In)
		   value = mEaseInQuart(t,b, c, d);
		else if (dir == Ease::Out)
		   value = mEaseOutQuart(t,b, c, d);
		else if (dir == Ease::InOut)
		   value = mEaseInOutQuart(t,b, c, d);
	}
	else if (type == Ease::Quintic)
	{
		if (dir == Ease::In)
		   value = mEaseInQuint(t,b, c, d);
		else if (dir == Ease::Out)
		   value = mEaseOutQuint(t,b, c, d);
		else if (dir == Ease::InOut)
		   value = mEaseInOutQuint(t,b, c, d);
	}
	else if (type == Ease::Sinusoidal)
	{
		if (dir == Ease::In)
		   value = mEaseInSine(t,b, c, d);
		else if (dir == Ease::Out)
		   value = mEaseOutSine(t,b, c, d);
		else if (dir == Ease::InOut)
		   value = mEaseInOutSine(t,b, c, d);
	}
	else if (type == Ease::Exponential)
	{
		if (dir == Ease::In)
		   value = mEaseInExpo(t,b, c, d);
		else if (dir == Ease::Out)
		   value = mEaseOutExpo(t,b, c, d);
		else if (dir == Ease::InOut)
		   value = mEaseInOutExpo(t,b, c, d);
	}
	else if (type == Ease::Circular)
	{
		if (dir == Ease::In)
		   value = mEaseInCirc(t,b, c, d);
		else if (dir == Ease::Out)
		   value = mEaseOutCirc(t,b, c, d);
		else if (dir == Ease::InOut)
		   value = mEaseInOutCirc(t,b, c, d);
	}
	else if (type == Ease::Elastic)
	{
		if (dir == Ease::In)
		   value = mEaseInElastic(t,b, c, d, param[0], param[1]);
		else if (dir == Ease::Out)
		   value = mEaseOutElastic(t,b, c, d, param[0], param[1]);
		else if (dir == Ease::InOut)
		   value = mEaseInOutElastic(t,b, c, d, param[0], param[1]);
	}
	else if (type == Ease::Back)
	{
		if (dir == Ease::In)
		   value = mEaseInBack(t,b, c, d, param[0]);
		else if (dir == Ease::Out)
		   value = mEaseOutBack(t,b, c, d, param[0]);
		else if (dir == Ease::InOut)
		   value = mEaseInOutBack(t,b, c, d, param[0]);
	}
	else if (type == Ease::Bounce)
	{
		if (dir == Ease::In)
		   value = mEaseInBounce(t,b, c, d);
		else if (dir == Ease::Out)
		   value = mEaseOutBounce(t,b, c, d);
		else if (dir == Ease::InOut)
		   value = mEaseInOutBounce(t,b, c, d);
	}
	else
	{
		// what ?
	}

	return value;
}

// < pg
