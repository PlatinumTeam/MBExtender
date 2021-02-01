//-----------------------------------------------------------------------------
// Interpolation.cpp
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

#include "MarbleGhostingFix.h"
#include <MBExtender/MBExtender.h>
#include <vector>

#include <TorqueLib/game/gameConnection.h>

MBX_MODULE(Interpolation);

bool gInterpolateMarbles = true;

MBX_CONSOLE_FUNCTION(enableInterpolation, void, 2, 2, "") {
	gInterpolateMarbles = atoi(argv[1]);
}

MBX_ON_CLIENT_PROCESS(interpolateMarbles, (uint32_t delta)) {
	if (!gInterpolateMarbles)
		return;

	std::vector<SimObjectId> toErase;
	//Look through all marbles
	for (std::vector<SimObjectId>::iterator it = gClientMarbles.begin(); it != gClientMarbles.end(); it ++) {
		U32 id = *it;
		//Try to find the marble's object
		TGE::SimObject *object = TGE::Sim::findObject(StringMath::print(id));
		//Not found?
		if (!object) {
			//Take it out of the list
			toErase.push_back(id);
			continue;
		}
		TGE::Marble *marble = static_cast<TGE::Marble *>(object);

		//Is this our marble?
		bool us = marble->getControllingClient() == TGE::NetConnection::getConnectionToServer();
		//Don't interpolate ourselves
		if (us) {
			continue;
		}

		//Various velocities that we should apply
		Point3D velocity = marble->getVelocity();
		Point3D angularVelocity = marble->getAngularVelocity();

		if (mIsNaN(velocity) || mIsNaN(angularVelocity)) {
			//Don't go anywhere near NaN.
			continue;
		}

		//Scale them by time, they are in u/s
		velocity *= ((F32)delta / 1000.0);
		angularVelocity *= ((F32)delta / 1000.0);

		//Find the marble's current transform and interpolate it
		MatrixF current = marble->getTransform();

		//Find the marble's heading, but scaled to its radius. This lets us do
		// collision detection at the edge of the marble.
		Point3F radiusDir(velocity.toPoint3F());
		radiusDir.normalize();
		radiusDir *= marble->getCollisionRadius();

		if (mIsNaN(radiusDir)) {
			//Eek! Squares are bad!
			continue;
		}

		Point3F start = current.getPosition();
		//End is start + velocity. Radius is added for the raycast only; it is
		// taken off after;
		Point3F end(start + velocity.toPoint3F() + radiusDir);

		if (mIsNaN(start) || mIsNaN(end)) {
			continue;
		}

		//Raycast to make sure we don't go through any surfaces
		TGE::RayInfo info;
		U32 mask = 0x2008; //InteriorObjectType | StaticShapeObjectType
		if (TGE::gClientContainer.castRay(start, end, mask, &info)) {

			// Check for NaN. Because fucking torque.
			if (info.t != info.t) {
				end -= radiusDir;
			} else {
				//Subtract the radius so we're at the edge
				end = info.point - radiusDir;
			}
		} else {
			//We add this to end above; subtract it now so we don't speed up
			end -= radiusDir;
		}

		current.setPosition(end);

		//And update the marble again
		marble->setTransform(current);
		marble->setRenderTransform(current);
	}

	//Remove all the extra marbles that don't exist
	for (std::vector<SimObjectId>::iterator it = toErase.begin(); it != toErase.end(); it ++) {
		gClientMarbles.erase(std::find(gClientMarbles.begin(), gClientMarbles.end(), *it));
	}
}
