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
#ifndef SMRT_REFERENCED_H
#define SMRT_REFERENCED_H 1

#include <smrt/Export.h>

#ifndef NO_OPENTHREADS
#include <OpenThreads/ScopedLock>
#include <OpenThreads/Mutex>
#else
#include <smrt/NoThreads.h>
#endif

namespace smrt {

// forward declare, declared after Referenced below.
class DeleteHandler;
class Observer;

/** Base class from providing referencing counted objects.*/
class SMRT_EXPORT Referenced
{

    public:


        Referenced(); 
        
        explicit Referenced(bool threadSafeRefUnref); 

        Referenced(const Referenced&);

        inline Referenced& operator = (const Referenced&) { return *this; }

        /** Set whether to use a mutex to ensure ref() and unref() are thread safe.*/
        virtual void setThreadSafeRefUnref(bool threadSafe);

        /** Get whether a mutex is used to ensure ref() and unref() are thread safe.*/
        bool getThreadSafeRefUnref() const { return _refMutex!=0; }

        /** Increment the reference count by one, indicating that 
            this object has another pointer which is referencing it.*/
        inline void ref() const;
        
        /** Decrement the reference count by one, indicating that 
            a pointer to this object is referencing it.  If the
            reference count goes to zero, it is assumed that this object
            is no longer referenced and is automatically deleted.*/
        inline void unref() const;
        
        /** Decrement the reference count by one, indicating that 
            a pointer to this object is referencing it.  However, do
            not delete it, even if ref count goes to 0.  Warning, unref_nodelete() 
            should only be called if the user knows exactly who will
            be resonsible for, one should prefer unref() over unref_nodelete() 
            as the later can lead to memory leaks.*/
        void unref_nodelete() const;
        
        /** Return the number pointers currently referencing this object. */
        inline int referenceCount() const { return _refCount; }

        /** Add a Observer that is observering this object, notify the Observer when this object gets deleted.*/
        void addObserver(Observer* observer);

        /** Add a Observer that is observering this object, notify the Observer when this object gets deleted.*/
        void removeObserver(Observer* observer);

    public:

        /** Set whether reference counting should be use a mutex to create thread reference counting.*/
        static void setThreadSafeReferenceCounting(bool enableThreadSafeReferenceCounting);
        
        /** Get whether reference counting is active.*/
        static bool getThreadSafeReferenceCounting();

        friend class DeleteHandler;

        /** Set a DeleteHandler to which deletion of all referenced counted objects
          * will be delegated to.*/
        static void setDeleteHandler(DeleteHandler* handler);

        /** Get a DeleteHandler.*/
        static DeleteHandler* getDeleteHandler();

       
    protected:
    
        virtual ~Referenced();
        
        void deletUsingDeleteHandler() const;
        
        mutable OpenThreads::Mutex*     _refMutex;

        mutable int                     _refCount;
        
        void*                          _observers;
        
};

inline void Referenced::ref() const
{
    if (_refMutex)
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(*_refMutex); 
        ++_refCount;
    }
    else
    {
        ++_refCount;
    }
}

inline void Referenced::unref() const
{
    bool needDelete = false;
    if (_refMutex)
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(*_refMutex); 
        --_refCount;
        needDelete = _refCount<=0;
    }
    else
    {
        --_refCount;
        needDelete = _refCount<=0;
    }

    if (needDelete)
    {
        if (getDeleteHandler()) deletUsingDeleteHandler();
        else delete this;
    }
}


}

#endif //define SMRT_REFERENCED_H

