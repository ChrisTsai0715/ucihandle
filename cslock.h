
#ifndef __CCSLOCK_H__
#define __CCSLOCK_H__
#ifdef __APPLE__
#define PTHREAD_MUTEX_RECURSIVE_NP 1
#endif

#ifndef  HAVE_MUTEX_LOCK 
#  define HAVE_MUTEX_LOCK 1
#endif

#ifdef __MINGW32__ // complie for QT mingw
#  define HAVE_MUTEX_LOCK 0
#endif

#ifndef WIN32
#  include <pthread.h>
#  include <sys/types.h>
#  include <errno.h>
#  include "interlock.h"
#else
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#  endif
# 
#  include <windows.h>
#  if HAVE_MUTEX_LOCK 
#    include <atlsecurity.h>
#  endif
#
#endif

class ILockItem
{
public:
	virtual long Lock() = 0;
	virtual long Unlock() = 0;
	virtual ~ILockItem(){}
	ILockItem(){};
private:
	
	ILockItem(const ILockItem&rhs){}
	ILockItem& operator=(const ILockItem& rhs) { return *this; };
};

#ifdef WIN32
  #ifndef __MINGW32__ // Windows compability
	typedef HANDLE pthread_t;
	typedef HANDLE pthread_mutex_t;
	typedef HANDLE pthread_cond_t;
	typedef DWORD pthread_key_t;
  #endif
#endif

#ifndef _AUTO_LOCK_NAME
#define _AUTO_LOCK_NAME CAutoLock
#endif

template <class T> 
class _AUTO_LOCK_NAME
{
public:
	_AUTO_LOCK_NAME(T& lock);
	~_AUTO_LOCK_NAME();
private:
	T& m_rLock;
};

template <class T>
_AUTO_LOCK_NAME<T>::_AUTO_LOCK_NAME(T& lock)
:m_rLock(lock)
{
	m_rLock.Lock();
}

template <class T>
_AUTO_LOCK_NAME<T>::~_AUTO_LOCK_NAME()
{
	m_rLock.Unlock();
}

template <class T> 
class CAutoLockEx
{
public:
	CAutoLockEx(T& lock,bool block = true,bool bManu = false);
	~CAutoLockEx();
	bool Locked(){return m_bLocked;}
	bool TryLock(){m_bLocked = m_rLock.TryLock();return m_bLocked;}
	void UnLock(){if(m_bLocked){ m_bLocked=false; m_rLock.Unlock();}}
private:
	T& m_rLock;
	bool m_bLocked;
};

template <class T>
CAutoLockEx<T>::CAutoLockEx(T& lock,bool block ,bool bManu )
:m_rLock(lock)
,m_bLocked(false)
{
	if(!bManu)
	{
		if (block == true)
		{
			m_rLock.Lock();
			m_bLocked = true;
		}
		else
		{
			m_bLocked = m_rLock.TryLock();
		}
	}
}

template <class T>
CAutoLockEx<T>::~CAutoLockEx()
{
	if(m_bLocked)
		m_rLock.Unlock();
}

#if HAVE_MUTEX_LOCK 
class CMutexLock
	:public ILockItem
{
public:
	CMutexLock(const char* name = 0)
	{
	#ifdef WIN32
		if(name)
		{
			SID_IDENTIFIER_AUTHORITY siaWorld=SECURITY_WORLD_SID_AUTHORITY;   
			PSID psidEveryone=NULL;
			if(!AllocateAndInitializeSid(&siaWorld,1,     
				SECURITY_WORLD_RID, 0, 0, 0, 0, 0, 0, 0,     
				&psidEveryone))   
			{   
				printf("AllocateAndInitializeSid()   failed   with   error   %d\n",GetLastError());  
			} 

			ATL::CSid ace(*((SID*)psidEveryone));
			ATL::CDacl dacl;
			dacl.AddAllowedAce(ace,0x10000000);

			ATL::CSecurityDesc sd;
			sd.SetDacl(dacl,false);

			ATL::CSecurityAttributes security(sd,false);
			m_hMutex  = CreateMutexA(&security, FALSE, name); 
			if(psidEveryone)   
				FreeSid(psidEveryone);  
		}
		else
		{
			m_hMutex  = CreateMutexA(NULL, FALSE, name); 
		}
	#else
		pthread_mutexattr_t attr;
		pthread_mutexattr_init(&attr);
		pthread_mutexattr_settype(&attr,PTHREAD_MUTEX_RECURSIVE_NP);
		pthread_mutex_init(&m_hMutex, &attr);
	#endif

	}
	~CMutexLock()
	{
		#ifdef WIN32
			CloseHandle(m_hMutex);
		#else
			pthread_mutex_destroy(&m_hMutex);
		#endif
		
		
	}
	bool TryLock() 
	{
		#ifdef WIN32
			return WaitForSingleObject(m_hMutex,0) == WAIT_OBJECT_0 ;
		#else
			return pthread_mutex_trylock(&m_hMutex) != EBUSY;
		#endif
	}
	long Lock()
	{
	#ifdef WIN32
		if(WaitForSingleObject(m_hMutex,INFINITE) == WAIT_OBJECT_0) 
			return 1;
		return 0;
	#else
		pthread_mutex_lock(&m_hMutex);
		return 1;
	#endif

	}
	long Unlock()
	{
		

	#ifdef WIN32
		ReleaseMutex(m_hMutex);
	#else
		pthread_mutex_unlock(&m_hMutex);
	#endif
		
		return 1;
	}

private:

	pthread_mutex_t  m_hMutex;            // Alias name of the mutex to be protected

};
typedef CMutexLock CMutexLockEx;
#endif

typedef CMutexLock CCrtSection;
typedef  _AUTO_LOCK_NAME<CCrtSection> CCSLock;


class CSingleEntrance
{
public:
	CSingleEntrance():	m_lIn(0)	{	}
	~CSingleEntrance()				{	}
	bool TryLock()					{	return InterlockedCompareExchange(&m_lIn,1,0) == 0;	}
	long Lock()						{	return TryLock();	}
	long Unlock()					{	InterlockedExchange(&m_lIn,0);return 0;}
	long state()const{return m_lIn;}
private:
	volatile long m_lIn;
};

#endif //__CCSLOCK_H__
