//-----------------------------------------------------------------------------
// DifSupport.cpp
//
// Copyright (c) 2017 The Platinum Team
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//-----------------------------------------------------------------------------

//Why? Why not?

#include <MBExtender/MBExtender.h>
#include <TorqueLib/console/console.h>
#include <TorqueLib/interior/interior.h>
#include <TorqueLib/dgl/materialList.h>
#include <TorqueLib/core/fileStream.h>
#include <TorqueLib/dgl/gBitmap.h>

MBX_MODULE(DifSupport);

bool initPlugin(MBX::Plugin &plugin)
{
	MBX_INSTALL(plugin, DifSupport);
	return true;
}

using namespace TGE;

template <typename T>
bool read(TGE::Stream &stream, T *into) {
	return stream._read(sizeof(T), into);
}

template <typename T>
bool read(TGE::Stream &stream, U32 size, T *into) {
	return stream._read(sizeof(T) * size, into);
}

template<>
bool read(TGE::Stream &stream, Point3F *into) {
	return read(stream, &(into->x)) &&
		read(stream, &(into->y)) &&
		read(stream, &(into->z));
}

bool readVectorSize(TGE::Stream &stream, U32 *size, bool *flag, U32 *param) {
	if (!read(stream, size)) {
		return false;
	}

	//Lots of index lists here that have U16 or U32 versions based on the sign bit.
	// The actual bytes of the interior have 0x80s at the ends (negative bit)
	// which seems to specify that these take a smaller type. They managed to
	// save ~50KB/interior, but was it worth the pain?

	//Params to use for the condition
	*flag = false;
	*param = 0;

	//Should we use the alternate version?
	if (*size & 0x80000000) {
		//Flip the sign bit
		*size ^= 0x80000000;
		*flag = true;

		//Extra U8 of data in each of these, almost never used but still there
		if (!read(stream, param))
			return false;
	}

	return true;
}

template<>
bool read(TGE::Stream &stream, PlaneF *into) {
	return read(stream, &(into->x)) &&
	read(stream, &(into->y)) &&
	read(stream, &(into->z)) &&
	read(stream, &(into->d));
}

template<>
bool read(TGE::Stream &stream, ColorF *into) {
	ColorI color;
	if (!read(stream, &color.red)) return false;
	if (!read(stream, &color.green)) return false;
	if (!read(stream, &color.blue)) return false;
	if (!read(stream, &color.alpha)) return false;
	*into = color;
	return true;
}

bool readLMapTexGen(TGE::Interior *thisptr, Stream& stream, PlaneF& planeX, PlaneF& planeY)
{
	F32 genX[4];
	F32 genY[4];

	for(U32 i = 0; i < 4; i++)
	{
		genX[i] = 0.0f;
		genY[i] = 0.0f;
	}

	U16 finalWord;
	read(stream, &finalWord);
	read(stream, &genX[3]);
	read(stream, &genY[3]);

	// Unpack the final word.
	U32 logScaleY = (finalWord >> 0) & ((1 << 6) - 1);
	U32 logScaleX = (finalWord >> 6) & ((1 << 6) - 1);
	U16 stEnc     = (finalWord >> 13) & 7;

	S32 sc, tc;
	switch(stEnc)
	{
		case 0: sc = 0; tc = 1; break;
		case 1: sc = 0; tc = 2; break;
		case 2: sc = 1; tc = 0; break;
		case 3: sc = 1; tc = 2; break;
		case 4: sc = 2; tc = 0; break;
		case 5: sc = 2; tc = 1; break;
	}

	U32 invScaleX = 1 << logScaleX;
	U32 invScaleY = 1 << logScaleY;

	genX[sc] = F32(1.0 / F64(invScaleX));
	genY[tc] = F32(1.0 / F64(invScaleY));

	planeX.x = genX[0];
	planeX.y = genX[1];
	planeX.z = genX[2];
	planeX.d = genX[3];
	planeY.x = genY[0];
	planeY.y = genY[1];
	planeY.z = genY[2];
	planeY.d = genY[3];

	return stream.getStatus() == Stream::Ok;
}

bool readSurface(TGE::Interior *thisptr, TGE::Stream& stream, TGE::Interior::Surface& surface, TGE::Interior::TexGenPlanes& texgens, const bool tgeInterior, const U32 mFileVersion) {
	// If we end up reading any invalid data then odds are that we
	// are no longer correctly reading from the stream and have gotten
	// off because this is a TGE version 0 Interior so we bail.
	// That is why you will see checks all the way through
	read(stream, &surface.windingStart);

	if (surface.windingStart >= thisptr->mWindings.size())
		return false;

	if (mFileVersion >= 13) {
		U32 count;
		read(stream, &count);
		surface.windingCount = (U8)count;
	} else {
		read(stream, &surface.windingCount);
	}

	if (surface.windingStart + surface.windingCount > thisptr->mWindings.size())
		return false;

	read(stream, &surface.planeIndex);

	if (U32(surface.planeIndex & ~0x8000) >= thisptr->mPlanes.size())
		return false;

	read(stream, &surface.textureIndex);

	if (surface.textureIndex >= thisptr->mMaterialList->mMaterials.size())
		return false;

	read(stream, &surface.texGenIndex);

	if (surface.texGenIndex >= thisptr->mTexGenEQs.size())
		return false;

	read(stream, &surface.surfaceFlags);
	read(stream, &surface.fanMask);

	// If reading the lightmap texgen fails then most likely this is a
	// TGE version 0 Interior (it gets offset by the "unused" read below
	if (readLMapTexGen(thisptr, stream, texgens.planeX, texgens.planeY) == false)
		return false;

	read(stream, &surface.lightCount);
	read(stream, &surface.lightStateInfoStart);

	if (mFileVersion >= 13) {
		U32 offX, offY, sizeX, sizeY;
		read(stream, &offX);
		read(stream, &offY);
		read(stream, &sizeX);
		read(stream, &sizeY);

		surface.mapOffsetX = (U8)offX;
		surface.mapOffsetY = (U8)offY;
		surface.mapSizeX = (U8)sizeX;
		surface.mapSizeY = (U8)sizeY;
	} else {
		read(stream, &surface.mapOffsetX);
		read(stream, &surface.mapOffsetY);
		read(stream, &surface.mapSizeX);
		read(stream, &surface.mapSizeY);
	}

	if (!tgeInterior) {
		U8 unused;
		read(stream, &unused);
		if (mFileVersion >= 2 && mFileVersion <= 5) {
			U32 brushId;
			read(stream, &brushId);
		}
	}

	return true;
}

bool newRead(TGE::Interior *thisptr, TGE::Stream &stream) {
	S32 i;
	bool flag;
	U32 param;

	// Version this stream.  We only load stream of the current version
	U32 mFileVersion;
	read(stream, &mFileVersion);
	if(mFileVersion > 14)
	{
		Con::errorf(ConsoleLogEntry::General, "Interior::read: incompatible file version found.");
		return false;
	}
	//We convert to v0-mbg
	thisptr->mFileVersion = 0;

	// Geometry factors...
	read(stream, &thisptr->mDetailLevel);

	read(stream, &thisptr->mMinPixels);

	read(stream, &thisptr->mBoundingBox.minExtents.x);
	read(stream, &thisptr->mBoundingBox.minExtents.y);
	read(stream, &thisptr->mBoundingBox.minExtents.z);
	read(stream, &thisptr->mBoundingBox.maxExtents.x);
	read(stream, &thisptr->mBoundingBox.maxExtents.y);
	read(stream, &thisptr->mBoundingBox.maxExtents.z);

	read(stream, &thisptr->mBoundingSphere.center.x);
	read(stream, &thisptr->mBoundingSphere.center.y);
	read(stream, &thisptr->mBoundingSphere.center.z);
	read(stream, &thisptr->mBoundingSphere.radius);

	read(stream, &thisptr->mHasAlarmState);
	read(stream, &thisptr->mNumLightStateEntries);

	// Now read in our data vectors.
	U32 vectorSize;

	// thisptr->mPlanes
	readVectorSize(stream, &vectorSize, &flag, &param);
	Point3F* normals = new Point3F[vectorSize];
	for(i = 0; i < vectorSize; i++)
		read(stream, &normals[i]);

	U16 index;
	readVectorSize(stream, &vectorSize, &flag, &param);
	thisptr->mPlanes.setSize(vectorSize);
	for(i = 0; i < thisptr->mPlanes.size(); i++)
	{
		read(stream, &index);
		read(stream, &thisptr->mPlanes[i].d);
		thisptr->mPlanes[i].x = normals[index].x;
		thisptr->mPlanes[i].y = normals[index].y;
		thisptr->mPlanes[i].z = normals[index].z;
	}

	delete [] normals;

	// thisptr->mPoints
	readVectorSize(stream, &vectorSize, &flag, &param);
	thisptr->mPoints.setSize(vectorSize);
	for(i = 0; i < thisptr->mPoints.size(); i++)
		read(stream, &thisptr->mPoints[i].point);

	// thisptr->mPointVisibility
	if (mFileVersion == 4) {
		//Fake it
		thisptr->mPointVisibility.setSize(vectorSize);
		thisptr->mPointVisibility.fill(0xFF);
	} else {
		readVectorSize(stream, &vectorSize, &flag, &param);
		thisptr->mPointVisibility.setSize(vectorSize);
		read(stream, vectorSize, thisptr->mPointVisibility.address());
	}

	// thisptr->mTexGenEQs
	readVectorSize(stream, &vectorSize, &flag, &param);
	thisptr->mTexGenEQs.setSize(vectorSize);
	for(i = 0; i < thisptr->mTexGenEQs.size(); i++)
	{
		read(stream, &thisptr->mTexGenEQs[i].planeX);
		read(stream, &thisptr->mTexGenEQs[i].planeY);
	}

	// thisptr->mBSPNodes;
	readVectorSize(stream, &vectorSize, &flag, &param);
	thisptr->mBSPNodes.setSize(vectorSize);
	for(i = 0; i < thisptr->mBSPNodes.size(); i++)
	{
		read(stream, &thisptr->mBSPNodes[i].planeIndex);
		if (mFileVersion >= 14) {
			U32 tmpFront, tmpBack;
			read(stream, &tmpFront);
			read(stream, &tmpBack);

			//Fuckers
			if ((tmpFront & 0x80000) != 0) {
				tmpFront = (tmpFront & ~0x80000) | 0x8000;
			}
			if ((tmpFront & 0x40000) != 0) {
				tmpFront = (tmpFront & ~0x40000) | 0x4000;
			}
			if ((tmpBack & 0x80000) != 0) {
				tmpBack = (tmpBack & ~0x80000) | 0x8000;
			}
			if ((tmpBack & 0x40000) != 0) {
				tmpBack = (tmpBack & ~0x40000) | 0x4000;
			}

			thisptr->mBSPNodes[i].frontIndex = tmpFront;
			thisptr->mBSPNodes[i].backIndex = tmpBack;
		} else {
			read(stream, &thisptr->mBSPNodes[i].frontIndex);
			read(stream, &thisptr->mBSPNodes[i].backIndex);
		}
		thisptr->mBSPNodes[i].terminalZone = 0;
	}

	// thisptr->mBSPSolidLeaves
	readVectorSize(stream, &vectorSize, &flag, &param);
	thisptr->mBSPSolidLeaves.setSize(vectorSize);
	for(i = 0; i < thisptr->mBSPSolidLeaves.size(); i++)
	{
		read(stream, &thisptr->mBSPSolidLeaves[i].surfaceIndex);
		read(stream, &thisptr->mBSPSolidLeaves[i].surfaceCount);
	}

	// MaterialList
	if(thisptr->mMaterialList != NULL)
		delete thisptr->mMaterialList;
	thisptr->mMaterialList = MaterialList::create();
	thisptr->mMaterialList->read(stream);

	// thisptr->mWindings
	readVectorSize(stream, &vectorSize, &flag, &param);
	thisptr->mWindings.setSize(vectorSize);
	for(i = 0; i < thisptr->mWindings.size(); i++)
	{
		if (param) {
			U16 index;
			read(stream, &index);
			thisptr->mWindings[i] = index;
		} else {
			read(stream, &thisptr->mWindings[i]);
		}
	}

	// thisptr->mWindingIndices
	readVectorSize(stream, &vectorSize, &flag, &param);
	thisptr->mWindingIndices.setSize(vectorSize);
	for(i = 0; i < thisptr->mWindingIndices.size(); i++)
	{
		read(stream, &thisptr->mWindingIndices[i].windingStart);
		read(stream, &thisptr->mWindingIndices[i].windingCount);
	}

	if (mFileVersion >= 12) {
		readVectorSize(stream, &vectorSize, &flag, &param);
		for (i = 0; i < vectorSize; i ++) {
			S32 dummy;
			read(stream, &dummy); //pointIndex0
			read(stream, &dummy); //pointIndex1
			read(stream, &dummy); //surfaceIndex0
			read(stream, &dummy); //surfaceIndex1
		}
	}

	// thisptr->mZones
	readVectorSize(stream, &vectorSize, &flag, &param);
	thisptr->mZones.setSize(vectorSize);
	for(i = 0; i < thisptr->mZones.size(); i++)
	{
		read(stream, &thisptr->mZones[i].portalStart);
		read(stream, &thisptr->mZones[i].portalCount);
		read(stream, &thisptr->mZones[i].surfaceStart);
		read(stream, &thisptr->mZones[i].surfaceCount);
		if (mFileVersion >= 12) {
			U32 dummy;
			read(stream, &dummy); //staticMeshStart
			read(stream, &dummy); //staticMeshCount
		}
		read(stream, &thisptr->mZones[i].flags);
		thisptr->mZones[i].zoneId = 0;
	}

	// thisptr->mZoneSurfaces
	readVectorSize(stream, &vectorSize, &flag, &param);
	thisptr->mZoneSurfaces.setSize(vectorSize);
	for(i = 0; i < thisptr->mZoneSurfaces.size(); i++)
		read(stream, &thisptr->mZoneSurfaces[i]);

	if (mFileVersion >= 12) {
		readVectorSize(stream, &vectorSize, &flag, &param);
		for (i = 0; i < vectorSize; i ++) {
			U32 dummy;
			read(stream, &dummy); //zoneStaticMesh
		}
	}

	//  thisptr->mZonePortalList;
	readVectorSize(stream, &vectorSize, &flag, &param);
	thisptr->mZonePortalList.setSize(vectorSize);
	for(i = 0; i < thisptr->mZonePortalList.size(); i++)
		read(stream, &thisptr->mZonePortalList[i]);

	// thisptr->mPortals
	readVectorSize(stream, &vectorSize, &flag, &param);
	thisptr->mPortals.setSize(vectorSize);
	for(i = 0; i < thisptr->mPortals.size(); i++)
	{
		read(stream, &thisptr->mPortals[i].planeIndex);
		read(stream, &thisptr->mPortals[i].triFanCount);
		read(stream, &thisptr->mPortals[i].triFanStart);
		read(stream, &thisptr->mPortals[i].zoneFront);
		read(stream, &thisptr->mPortals[i].zoneBack);
	}

	// mSurfaces
	readVectorSize(stream, &vectorSize, &flag, &param);
	thisptr->mSurfaces.setSize(vectorSize);
	thisptr->mLMTexGenEQs.setSize(vectorSize);

	// Couple of hoops to *attempt* to detect that we are loading
	// a TGE version 0 Interior and not a TGEA verison 0
	U32 surfacePos = stream.getPosition();
	bool tgeInterior = false;

	// First attempt to read this as though it isn't a TGE version 0 Interior
	for(i = 0; i < thisptr->mSurfaces.size(); i++)
	{
		// If we end up reading any invalid data in this loop then odds
		// are that we are no longer correctly reading from the stream
		// and have gotten off because this is a TGE version 0 Interior

		TGE::Interior::Surface& surface = thisptr->mSurfaces[i];

		if (readSurface(thisptr, stream, surface, thisptr->mLMTexGenEQs[i], false, mFileVersion) == false)
		{
			tgeInterior = true;
			break;
		}
	}

	// If this is a version 0 Interior and we failed to read it as a
	// TGEA version 0 Interior then attempt to read it as a TGE version 0
	if (mFileVersion == 0 && tgeInterior)
	{
		// Set our stream position back to the start of the surfaces
		stream.setPosition(surfacePos);

		// Try reading in the surfaces again
		for(i = 0; i < thisptr->mSurfaces.size(); i++)
		{
			TGE::Interior::Surface& surface = thisptr->mSurfaces[i];

			// If we fail on any of the surfaces then bail
			if (readSurface(thisptr, stream, surface, thisptr->mLMTexGenEQs[i], true, mFileVersion) == false)
				return false;
		}
	}
	// If we failed to read but this isn't a version 0 Interior
	// then something has gone horribly wrong
	else if (mFileVersion != 0 && tgeInterior)
		return false;

	if (mFileVersion >= 2 && mFileVersion <= 5) {
		//Edge data from MBU levels and beyond in some cases
		readVectorSize(stream, &vectorSize, &flag, &param);
		for (i = 0; i < vectorSize; i ++) {
			U32 dummy;
			read(stream, &dummy); //vertex0
			read(stream, &dummy); //vertex1
			read(stream, &dummy); //normal0
			read(stream, &dummy); //normal1
			if (mFileVersion >= 3) {
				read(stream, &dummy); //face0
				read(stream, &dummy); //face1
			}
		}

		//v4 has some extra points and indices, they're probably used with the edges
		// but I have no idea
		if (mFileVersion >= 4 && mFileVersion <= 5) {
			//Extra normals used in reading the edges?
			readVectorSize(stream, &vectorSize, &flag, &param);
			for (i = 0; i < vectorSize; i ++) {
				Point3F dummy;
				read(stream, &dummy);
			}

			//Looks like indcies of some sort, can't seem to make them out though

			//Unlike anywhere else, these actually take the param into account.
			// If it's read2 and param == 0, then they use U8s, if param == 1, they use U16s
			// Not really sure why, haven't seen this anywhere else.

			readVectorSize(stream, &vectorSize, &flag, &param);
			for (i = 0; i < vectorSize; i ++) {
				if (flag && param == 0) {
					U8 dummy;
					read(stream, &dummy);
				} else {
					U16 dummy;
					read(stream, &dummy);
				}
			}
		}
	}

	// NormalLMapIndices
	readVectorSize(stream, &vectorSize, &flag, &param);
	thisptr->mNormalLMapIndices.setSize(vectorSize);
	if (mFileVersion >= 13) {
		for (i = 0; i < vectorSize; i ++) {
			U32 dummy;
			read(stream, &dummy);
			thisptr->mNormalLMapIndices[i] = (U8)dummy;
		}
	} else {
		read(stream, thisptr->mNormalLMapIndices.size(), thisptr->mNormalLMapIndices.address());
	}

	if (mFileVersion == 4) {
		//Fake it
		thisptr->mAlarmLMapIndices.setSize(vectorSize);
		thisptr->mAlarmLMapIndices.fill(0xFF);
	} else {
		// AlarmLMapIndices
		readVectorSize(stream, &vectorSize, &flag, &param);
		thisptr->mAlarmLMapIndices.setSize(vectorSize);
		if (mFileVersion >= 13) {
			for (i = 0; i < vectorSize; i ++) {
				U32 dummy;
				read(stream, &dummy);
				thisptr->mAlarmLMapIndices[i] = (U8)dummy;
			}
		} else {
			read(stream, thisptr->mAlarmLMapIndices.size(), thisptr->mAlarmLMapIndices.address());
		}
	}

	// thisptr->mNullSurfaces
	readVectorSize(stream, &vectorSize, &flag, &param);
	thisptr->mNullSurfaces.setSize(vectorSize);
	for(i = 0; i < thisptr->mNullSurfaces.size(); i++)
	{
		read(stream, &thisptr->mNullSurfaces[i].windingStart);
		read(stream, &thisptr->mNullSurfaces[i].planeIndex);
		read(stream, &thisptr->mNullSurfaces[i].surfaceFlags);
		if (mFileVersion >= 13) {
			U32 dummy;
			read(stream, &dummy);
			thisptr->mNullSurfaces[i].windingCount = (U8)dummy;
		} else {
			read(stream, &thisptr->mNullSurfaces[i].windingCount);
		}
	}

	if (mFileVersion != 4) {
		// thisptr->mLightmaps
		readVectorSize(stream, &vectorSize, &flag, &param);
		thisptr->mLightmaps.setSize(vectorSize);
		thisptr->mLightmapKeep.setSize(vectorSize);
		for(i = 0; i < thisptr->mLightmaps.size(); i++)
		{
			thisptr->mLightmaps[i] = GBitmap::create();
			thisptr->mLightmaps[i]->readPNG(stream);
			if (!tgeInterior) {
				GBitmap *dummy = GBitmap::create();
				dummy->readPNG(stream);
				delete dummy;
			}

			read(stream, &thisptr->mLightmapKeep[i]);
		}
	}

	// thisptr->mSolidLeafSurfaces
	readVectorSize(stream, &vectorSize, &flag, &param);
	thisptr->mSolidLeafSurfaces.setSize(vectorSize);
	for(i = 0; i < thisptr->mSolidLeafSurfaces.size(); i++)
	{
		if (flag) {
			U16 dummy;
			read(stream, &dummy);
			thisptr->mSolidLeafSurfaces[i] = dummy;
		} else {
			read(stream, &thisptr->mSolidLeafSurfaces[i]);
		}
	}

	// thisptr->mAnimatedLights
	thisptr->mNumTriggerableLights = 0;
	readVectorSize(stream, &vectorSize, &flag, &param);
	thisptr->mAnimatedLights.setSize(vectorSize);
	for(i = 0; i < thisptr->mAnimatedLights.size(); i++)
	{
		read(stream, &thisptr->mAnimatedLights[i].nameIndex);
		read(stream, &thisptr->mAnimatedLights[i].stateIndex);
		read(stream, &thisptr->mAnimatedLights[i].stateCount);
		read(stream, &thisptr->mAnimatedLights[i].flags);
		read(stream, &thisptr->mAnimatedLights[i].duration);

		if((thisptr->mAnimatedLights[i].flags & BIT(0)) == 0) //AnimationAmbient
			thisptr->mNumTriggerableLights++;
	}

	// thisptr->mLightStates
	readVectorSize(stream, &vectorSize, &flag, &param);
	thisptr->mLightStates.setSize(vectorSize);
	for(i = 0; i < thisptr->mLightStates.size(); i++)
	{
		read(stream, &thisptr->mLightStates[i].red);
		read(stream, &thisptr->mLightStates[i].green);
		read(stream, &thisptr->mLightStates[i].blue);
		read(stream, &thisptr->mLightStates[i].activeTime);
		read(stream, &thisptr->mLightStates[i].dataIndex);
		read(stream, &thisptr->mLightStates[i].dataCount);
	}

	if (mFileVersion == 4) {
		thisptr->mStateData.setSize(0);
		thisptr->mStateDataBuffer.setSize(0);
		thisptr->mNameBuffer.setSize(0);
	} else {
		// thisptr->mStateData
		readVectorSize(stream, &vectorSize, &flag, &param);
		thisptr->mStateData.setSize(vectorSize);
		for(i = 0; i < thisptr->mStateData.size(); i++)
		{
			read(stream, &thisptr->mStateData[i].surfaceIndex);
			read(stream, &thisptr->mStateData[i].mapIndex);
			read(stream, &thisptr->mStateData[i].lightStateIndex);
		}

		// thisptr->mStateDataBuffer
		readVectorSize(stream, &vectorSize, &flag, &param);
		thisptr->mStateDataBuffer.setSize(vectorSize);
		U32 flags;
		read(stream, &flags);
		read(stream, thisptr->mStateDataBuffer.size(), thisptr->mStateDataBuffer.address());

		// thisptr->mNameBuffer
		readVectorSize(stream, &vectorSize, &flag, &param);
		thisptr->mNameBuffer.setSize(vectorSize);
		read(stream, thisptr->mNameBuffer.size(), thisptr->mNameBuffer.address());

		// thisptr->mSubObjects - removed!
		readVectorSize(stream, &vectorSize, &flag, &param);
		if (vectorSize != 0) {
			TGE::Con::errorf("Cannot read interior sub objects!");
			return false;
		}
	}

	// Convex hulls
	readVectorSize(stream, &vectorSize, &flag, &param);
	thisptr->mConvexHulls.setSize(vectorSize);
	for(i = 0; i < thisptr->mConvexHulls.size(); i++)
	{
		read(stream, &thisptr->mConvexHulls[i].hullStart);
		read(stream, &thisptr->mConvexHulls[i].hullCount);
		read(stream, &thisptr->mConvexHulls[i].minX);
		read(stream, &thisptr->mConvexHulls[i].maxX);
		read(stream, &thisptr->mConvexHulls[i].minY);
		read(stream, &thisptr->mConvexHulls[i].maxY);
		read(stream, &thisptr->mConvexHulls[i].minZ);
		read(stream, &thisptr->mConvexHulls[i].maxZ);
		read(stream, &thisptr->mConvexHulls[i].surfaceStart);
		read(stream, &thisptr->mConvexHulls[i].surfaceCount);
		read(stream, &thisptr->mConvexHulls[i].planeStart);
		read(stream, &thisptr->mConvexHulls[i].polyListPlaneStart);
		read(stream, &thisptr->mConvexHulls[i].polyListPointStart);
		read(stream, &thisptr->mConvexHulls[i].polyListStringStart);

		if (mFileVersion >= 12) {
			U8 dummy;
			read(stream, &dummy); //staticMesh
		}
	}

	readVectorSize(stream, &vectorSize, &flag, &param);
	thisptr->mConvexHullEmitStrings.setSize(vectorSize);
	read(stream, thisptr->mConvexHullEmitStrings.size(), thisptr->mConvexHullEmitStrings.address());

	readVectorSize(stream, &vectorSize, &flag, &param);
	thisptr->mHullIndices.setSize(vectorSize);
	for(i = 0; i < thisptr->mHullIndices.size(); i++) {
		if (flag) {
			U16 dummy;
			read(stream, &dummy);
			thisptr->mHullIndices[i] = dummy;
		} else {
			read(stream, &thisptr->mHullIndices[i]);
		}
	}

	readVectorSize(stream, &vectorSize, &flag, &param);
	thisptr->mHullPlaneIndices.setSize(vectorSize);
	for(i = 0; i < thisptr->mHullPlaneIndices.size(); i++)
		read(stream, &thisptr->mHullPlaneIndices[i]);

	readVectorSize(stream, &vectorSize, &flag, &param);
	thisptr->mHullEmitStringIndices.setSize(vectorSize);
	for(i = 0; i < thisptr->mHullEmitStringIndices.size(); i++) {
		if (flag) {
			U16 dummy;
			read(stream, &dummy);
			thisptr->mHullEmitStringIndices[i] = dummy;
		} else {
			read(stream, &thisptr->mHullEmitStringIndices[i]);
		}
	}

	readVectorSize(stream, &vectorSize, &flag, &param);
	thisptr->mHullSurfaceIndices.setSize(vectorSize);
	for(i = 0; i < thisptr->mHullSurfaceIndices.size(); i++) {
		if (flag) {
			U16 dummy;
			read(stream, &dummy);
			thisptr->mHullSurfaceIndices[i] = dummy;
		} else {
			read(stream, &thisptr->mHullSurfaceIndices[i]);
		}
	}

	readVectorSize(stream, &vectorSize, &flag, &param);
	thisptr->mPolyListPlanes.setSize(vectorSize);
	for(i = 0; i < thisptr->mPolyListPlanes.size(); i++)
		read(stream, &thisptr->mPolyListPlanes[i]);

	readVectorSize(stream, &vectorSize, &flag, &param);
	thisptr->mPolyListPoints.setSize(vectorSize);
	for(i = 0; i < thisptr->mPolyListPoints.size(); i++) {
		if (flag) {
			U16 dummy;
			read(stream, &dummy);
			thisptr->mPolyListPoints[i] = dummy;
		} else {
			read(stream, &thisptr->mPolyListPoints[i]);
		}
	}

	readVectorSize(stream, &vectorSize, &flag, &param);
	thisptr->mPolyListStrings.setSize(vectorSize);
	for(i = 0; i < thisptr->mPolyListStrings.size(); i++)
		read(stream, &thisptr->mPolyListStrings[i]);

	// Coord bins
	for(i = 0; i < 16 * 16; i++)
	{
		read(stream, &thisptr->mCoordBins[i].binStart);
		read(stream, &thisptr->mCoordBins[i].binCount);
	}
	readVectorSize(stream, &vectorSize, &flag, &param);
	thisptr->mCoordBinIndices.setSize(vectorSize);
	for(i = 0; i < thisptr->mCoordBinIndices.size(); i++)
		read(stream, &thisptr->mCoordBinIndices[i]);
	read(stream, &thisptr->mCoordBinMode);

	if (mFileVersion == 4) {
		thisptr->mBaseAmbient.set(0, 0, 0, 1);
		thisptr->mAlarmAmbient.set(0, 0, 0, 1);
	} else {
		// Ambient colors
		read(stream, &thisptr->mBaseAmbient);
		read(stream, &thisptr->mAlarmAmbient);

		if (mFileVersion >= 10) {
			readVectorSize(stream, &vectorSize, &flag, &param);
			if (vectorSize != 0) {
				TGE::Con::errorf("Can't read interior static meshes.");
				return false;
			}
		}

		if (mFileVersion >= 11) {
			readVectorSize(stream, &vectorSize, &flag, &param);
			for (i = 0; i < vectorSize; i ++) {
				Point3F dummy;
				read(stream, &dummy); //texNormal
			}
			readVectorSize(stream, &vectorSize, &flag, &param);
			for (i = 0; i < vectorSize; i ++) {
				S32 dummy;
				read(stream, &dummy); //T
				read(stream, &dummy); //N
				read(stream, &dummy); //B
			}
			readVectorSize(stream, &vectorSize, &flag, &param);
			for (i = 0; i < vectorSize; i ++) {
				U32 dummy;
				read(stream, &dummy); //texMatIndex
			}
		} else {
			U32 numTexNormals, numTexMatrices, numTexMatIndices;
			read(stream, &numTexNormals);
			read(stream, &numTexMatrices);
			read(stream, &numTexMatIndices);
		}

		U32 extendedLightMapData;
		read(stream, &extendedLightMapData);
		if (extendedLightMapData) {
			U32 lightMapBorderSize, dummy;
			read(stream, &lightMapBorderSize);
			read(stream, &dummy);
		}
	}

	// Setup the zone planes
	thisptr->setupZonePlanes();
	thisptr->truncateZoneTree();

	return(stream.getStatus() == Stream::Ok);
}

MBX_OVERRIDE_MEMBERFN(bool, Interior::read, (TGE::Interior *thisptr, TGE::Stream &stream), originalRead) {
	bool ret = newRead(thisptr, stream);
	return ret;
}

