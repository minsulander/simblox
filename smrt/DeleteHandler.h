/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield 
 *
 * This library is open source and may be redistributed and/or modified under  
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or 
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * OpenSceneGraph Public License for more details.
*/

#ifndef SMRT_DELETEHANDLER
#define SMRT_DELETEHANDLER 1

#include <smrt/Referenced.h>

#include <list>

namespace smrt {


/** Class for override the default delete behavior so that users can implement their own object
  * deletion schemes.  This might be done to help implement protection of multiple threads from deleting
  * objects unintentionally.
  * Note, the DeleteHandler cannot itself be reference counted, otherwise it
  * would be responsible for deleting itself!
  * An static auto_ptr<> is used internally in Referenced.cpp to manage the 
  * DeleteHandler's memory.*/
class SMRT_EXPORT DeleteHandler
{
    public:

        typedef std::pair<int, const smrt::Referenced*> FrameNumberObjectPair;
        typedef std::list<FrameNumberObjectPair> ObjectsToDeleteList;

        DeleteHandler(int numberOfFramesToRetainObjects=0);

        virtual ~DeleteHandler() { flushAll(); }

        /** Set the number of frames to retain objects that are have been requested for deletion.
          * When set to zero objects are deleted immediately, by set to 1 there are kept around for an extra frame etc.
          * The ability to retain obejcts for several frames is useful to prevent premature deletion when objects
          * are stil be used the graphics threads that are using double buffering of rendering data structures with
          * non ref_ptr<> pointers to scene graph elements.*/        
        void setNumFramesToRetainObjects(int numberOfFramesToRetainObjects) {  _numFramesToRetainObjects = numberOfFramesToRetainObjects; }

        int getNumFramesToRetainObjects() const { return _numFramesToRetainObjects; }

        /** Set the current frame numberso that subsequent deletes get tagged as associated with this frame.*/
        void setFrameNumber(int frameNumber) { _currentFrameNumber = frameNumber; }

        /** Get the current frame number.*/
        int getFrameNumber() const { return _currentFrameNumber; }

        inline void doDelete(const Referenced* object) { delete object; }

        /** Flush objects that ready to be fully deleted.*/
        virtual void flush();

        /** Flush all objects that the DeleteHandler holds.
          * Note, this should only be called if there are no threads running with non ref_ptr<> pointers, such as graphics threads.*/
        virtual void flushAll();

        /** Request the deletion of an object. 
          * Depending on users implementation of DeleteHandler, the delete of the object may occur 
          * straight away or be delayed until doDelete is called.
          * The default implementation does a delete straight away.*/
        virtual void requestDelete(const smrt::Referenced* object);

    protected:

        DeleteHandler(const DeleteHandler&) {}
        DeleteHandler operator = (const DeleteHandler&) { return *this; }

        int                 _numFramesToRetainObjects;
        int                 _currentFrameNumber;
        OpenThreads::Mutex  _mutex;
        ObjectsToDeleteList _objectsToDelete;

};

}

#endif
