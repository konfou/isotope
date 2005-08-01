#ifndef __GTIMER_H__
#define __GTIMER_H__

#ifdef WIN32

// The IsItTimeYet() method will return true when it's time to refresh the frame
class GTimer
{
protected:
	__int64 m_nClock;
	__int64 m_nTicksPerFrame;

public:
	GTimer(int nFramesPerSecond);
	bool IsItTimeYet();
};


#endif // WIN32

#endif // __GTIMER_H__
