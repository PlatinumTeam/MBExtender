//-----------------------------------------------------------------------------
// Copyright (c) 2021 The Platinum Team
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
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//-----------------------------------------------------------------------------

#pragma once

#include <MBExtender/InteropMacros.h>
#include <TorqueLib/platform/platform.h>

namespace TGE
{
	class ResourceInstance;
	class Stream;
	class FileStream;

	typedef ResourceInstance* (*CreateResourceFn)(Stream &stream);

	class ResourceObject
	{
		BRIDGE_CLASS(ResourceObject);
	public:
		ResourceObject *prev, *next;

		ResourceObject *nextEntry;    ///< This is used by ResDictionary for its hash table.

		ResourceObject *nextResource;
		ResourceObject *prevResource;

		enum Flags {
			VolumeBlock = BIT(0),
			File = BIT(1),
			Added = BIT(2),
		};
		S32 flags;  ///< Set from Flags.

		const char *path;     ///< Resource path.
		const char *name;     ///< Resource name.

							  /// @name ZIP Archive
							  /// If the resource is stored in a zip file, these members are populated.
							  /// @{

							  ///
		const char *zipPath;  ///< Path of zip file.
		const char *zipName;  ///< Name of zip file.

		S32 fileOffset;            ///< Offset of data in zip file.
		S32 fileSize;              ///< Size on disk of resource block.
		S32 compressedFileSize;    ///< Actual size of resource data.
								   /// @}

		class ResourceInstance *mInstance;  ///< Pointer to actual object instance. If the object is not loaded,
											///  this may be NULL or garbage.
		S32 lockCount;                ///< Lock count; used to control load/unload of resource from memory.
		U32 crc;                      ///< CRC of resource.
	};

	class ResourceInstance
	{
		BRIDGE_CLASS(ResourceInstance);
	public:
		ResourceObject *mSourceResource;

		virtual ~ResourceInstance() = 0;
	};

	class ResManager
	{
		BRIDGE_CLASS(ResManager);
	public:
		MEMBERFN(Stream*, openStream, (const char *path), 0x407E37_win, 0x4E0D0_mac);
		MEMBERFN(Stream*, openStream, (ResourceObject *obj), 0x4079EB_win, 0x4DD50_mac);
		MEMBERFN(void, closeStream, (Stream *stream), 0x40388C_win, 0x4B910_mac);
		MEMBERFN(ResourceObject*, find, (const char *path), 0x404A61_win, 0x4C890_mac);
		MEMBERFN(ResourceObject*, load, (const char *path, bool computeCRC), 0x4057FE_win, 0x4E2D0_mac);
		MEMBERFN(U32, getSize, (const char *path), 0x405452_win, 0x4CDD0_mac);
		MEMBERFN(bool, getCrc, (const char *path, U32 &crc, U32 initialValue), 0x4074FA_win, 0x4E3A0_mac);
		MEMBERFN(void, searchPath, (const char *path), 0x40551A_win, 0x4D8F0_mac);
		MEMBERFN(bool, setModZip, (const char *path), 0x405CD1_win, 0x4BFD0_mac);
		MEMBERFN(void, freeResource, (ResourceObject *res), 0x403CE2_win, 0x4CF20_mac);
		MEMBERFN(bool, add, (const char *path, ResourceInstance *instance, bool extraLock), 0x404de5_win, 0x4c780_mac);
		MEMBERFN(bool, openFileForWrite, (FileStream &stream, const char *fileName, U32 accessMode), 0x407d3d_win, 0x4bcb0_mac);
		MEMBERFN(ResourceObject *, createResource, (const char *path, const char *file), 0x401B1D_win, 0x4BBC0_mac);
		MEMBERFN(ResourceObject *, findMatch, (const char *expression, const char **fn, ResourceObject *start), 0x4034B3_win, 0x4E510_mac);
		MEMBERFN(void, registerExtension, (const char *extension, CreateResourceFn createFn), 0x408F21_win, 0x4B810_mac);
		MEMBERFN(void, purge, (), 0x40215D_win, 0x4CFF0_mac);
		MEMBERFN(void, purge, (ResourceObject *obj), 0x408670_win, 0x4ceb0_mac);
		MEMBERFN(void, unlock, (TGE::ResourceObject *object), 0x40294b_win, 0x4c280_mac);
		MEMBERFN(void, setMissingFileLogging, (bool enabled), 0x40189d_win, 0x4ce00_mac);
		STATICFN(void, destroy, (), 0x406226_win, 0x4D700_mac);
	};

	GLOBALVAR(ResManager*, ResourceManager, 0x6A4064_win, 0x2DA560_mac);


	template <class T>
	class Resource
	{
		ResourceObject *obj;

		inline void _lock()
		{
			if (obj)
				obj->lockCount++;
		}

		inline void _unlock()
		{
			if (obj)
				ResourceManager->unlock( obj );
		}

	public:
		/// If assigned a ResourceObject, it's assumed to already have
		/// been locked, lock count is incremented only for copies or
		/// assignment from another Resource.
		Resource() : obj(NULL) { ; }
		Resource(ResourceObject *p) : obj(p) { ; }
		Resource(const Resource &res) : obj(res.obj) { _lock(); }
		~Resource() { unlock(); }  ///< Decrements the lock count on this object, and if the lock count is 0 afterwards,
		///< adds the object to the timeoutList for deletion on execution of purge().

		const char *getFilePath() const { return (obj ? obj->path : NULL); } ///< Returns the path of the file (without the actual name)
		const char *getFileName() const { return (obj ? obj->name : NULL); } ///< Returns the actual file name (without the path)

		Resource& operator= (ResourceObject *p) { _unlock(); obj = p; return *this; }
		Resource& operator= (const Resource &r) { _unlock(); obj = r.obj; _lock(); return *this; }

		U32 getCRC() { return (obj ? obj->crc : 0); }
		bool isNull()   const { return ((obj == NULL) || (obj->mInstance == NULL)); }
		operator bool() const { return ((obj != NULL) && (obj->mInstance != NULL)); }
		T* operator->()   { return (T*)obj->mInstance; }
		T& operator*()    { return *((T*)obj->mInstance); }
		operator T*() const    { return (obj) ? (T*)obj->mInstance : (T*)NULL; }
		const T* operator->() const  { return (const T*)obj->mInstance; }
		const T& operator*() const   { return *((const T*)obj->mInstance); }
		operator const T*() const    { return (obj) ? (const T*)obj->mInstance :  (const T*)NULL; }

		inline void unlock()
		{
			if (obj) {
				ResourceManager->unlock( obj );
				obj=NULL;
			}
		}

		inline void purge()
		{
			if (obj) {
				ResourceManager->unlock( obj );
				if (obj->lockCount == 0)
					ResourceManager->purge(obj);
				obj = NULL;
			}
		}
	};
}
