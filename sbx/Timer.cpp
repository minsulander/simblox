
// NOTE: This was originally copied from the OpenSceneGraph project - http://www.openscenegraph.org
// Original copyright header retained below.

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

#include "Timer.h"
#include "Log.h"

#include <stdio.h>
#include <string.h>
#include <iostream>

using namespace sbx;

// follows are the constructors of the Timer class, once version
// for each OS combination.  The order is WIN32, FreeBSD, Linux, IRIX,
// and the rest of the world.
//
// all the rest of the timer methods are implemented within the header.


Timer* Timer::instance()
{
    static Timer s_timer;
    return &s_timer;
}

#ifdef WIN32

    #include <sys/types.h>
    #include <fcntl.h>
    #include <windows.h>
    #include <winbase.h>
    Timer::Timer()
    {
        LARGE_INTEGER frequency;
        if(QueryPerformanceFrequency(&frequency))
        {
            _secsPerTick = 1.0/(double)frequency.QuadPart;
        }
        else
        {
            _secsPerTick = 1.0;
			dout(ERROR) <<"Error: Timer::Timer() unable to use QueryPerformanceFrequency, "<<std::endl;
			dout(ERROR) <<"timing code will be wrong, Windows error code: "<<GetLastError()<<std::endl;
        }
        
        setStartTick();        
    }

    Timer_t Timer::tick() const
    {
        LARGE_INTEGER qpc;
        if (QueryPerformanceCounter(&qpc))
        {
            return qpc.QuadPart;
        }
        else
        {
            dout(ERROR) <<"Timer::Timer() unable to use QueryPerformanceCounter, "<<std::endl;
            dout(ERROR) <<"timing code will be wrong, Windows error code: "<<GetLastError()<<std::endl;
            return 0;
        }
    }

	void Timer::sleep(unsigned long useconds)

	{
		/* On Windows, round to the nearest millisecond, with a
		* minimum of 1 millisecond if usleep was called with a
		* a non-zero value. */
		if (useconds > 500)
			Sleep ((useconds+500)/1000);
		else if (useconds > 0)
			Sleep (1);
		else
			Sleep (0);
	}


#else

    #include <sys/time.h>

    Timer::Timer( void )
    {
        _secsPerTick = (1.0 / (double) 1000000);

        setStartTick();        
    }

    Timer_t Timer::tick() const
    {
        struct timeval tv;
        gettimeofday(&tv, NULL);
        return ((sbx::Timer_t)tv.tv_sec)*1000000+(sbx::Timer_t)tv.tv_usec;
    }

	void Timer::sleep(unsigned long useconds)

	{

		//unsigned int sec = useconds / 1000000;
		usleep(useconds);

	}

#endif
