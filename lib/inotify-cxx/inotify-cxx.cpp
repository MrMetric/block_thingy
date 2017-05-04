/// inotify C++ interface implementation
/**
 * inotify C++ interface
 *
 * Copyright (C) 2006, 2007, 2009, 2012 Lukas Jelinek <lukas@aiken.cz>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of one of the following licenses:
 *
 * \li 1. X11-style license (see LICENSE-X11)
 * \li 2. GNU Lesser General Public License, version 2.1 (see LICENSE-LGPL)
 * \li 3. GNU General Public License, version 2  (see LICENSE-GPL)
 *
 * If you want to help with choosing the best license for you,
 * please visit http://www.gnu.org/licenses/license-list.html.
 *
 * Credits:
 *     Mike Frysinger (cleanup of includes)
 *     Christian Ruppert (new include to build with GCC 4.4+)
 *
 */

#include <algorithm>
#include <cstdio>
#include <errno.h>
#include <fcntl.h>
#include <fstream>
#include <iterator>
#include <sstream>
#include <sys/syscall.h>
#include <unistd.h>
#include <utility>
#include <vector>


// Use this if syscalls not defined
#ifndef __NR_inotify_init
#include <sys/inotify-syscalls.h>
#endif

#include "inotify-cxx.hpp"

InotifyException::InotifyException(const std::string& what_arg, int iErr, void* pSrc)
:
	std::runtime_error(what_arg),
	m_err(iErr),
	m_pSrc(pSrc)
{
}

InotifyException::~InotifyException()
{
}

int InotifyException::GetErrno() const
{
	return m_err;
}

void* InotifyException::GetSource() const
{
	return m_pSrc;
}

uint32_t InotifyEvent::GetMask() const
{
	return m_uMask;
}

bool InotifyEvent::IsType(const uint32_t value, const uint32_t type)
{
	return ((value & type) != 0) && ((~value & type) == 0);
}

bool InotifyEvent::IsType(const uint32_t type) const
{
	return IsType(m_uMask, type);
}

uint32_t InotifyEvent::GetCookie() const
{
	return m_uCookie;
}

std::string InotifyEvent::GetName() const
{
	return m_name;
}

InotifyWatch* InotifyEvent::GetWatch()
{
	return m_pWatch;
}

InotifyWatch::InotifyWatch(const std::string& path, uint32_t mask, bool enabled)
:
	m_path(path),
	m_uMask(mask),
	m_wd(-1),
	m_fEnabled(enabled)
{
}

std::string InotifyWatch::GetPath() const
{
	return m_path;
}

uint32_t InotifyWatch::GetMask() const
{
	return m_uMask;
}

void InotifyWatch::SetMask(uint32_t mask)
{
	if(m_wd != -1)
	{
		int wd = inotify_add_watch(m_pInotify->GetDescriptor(), m_path.c_str(), mask);
		if(wd != m_wd)
		{
			throw InotifyException(IN_EXC_MSG("changing mask failed"), wd == -1 ? errno : EINVAL, this);
		}
	}

	m_uMask = mask;
}

Inotify* InotifyWatch::GetInotify()
{
	return m_pInotify;
}

void InotifyWatch::SetEnabled(bool fEnabled)
{
	if(fEnabled == m_fEnabled)
	{
		return;
	}

	if(m_pInotify != nullptr)
	{
		if(fEnabled)
		{
			m_wd = inotify_add_watch(m_pInotify->GetDescriptor(), m_path.c_str(), m_uMask);
			if(m_wd == -1)
			{
				throw InotifyException(IN_EXC_MSG("enabling watch failed"), errno, this);
			}
			m_pInotify->m_watches.insert(IN_WATCH_MAP::value_type(m_wd, this));
		}
		else
		{
			if(inotify_rm_watch(m_pInotify->GetDescriptor(), m_wd) != 0)
			{
				throw InotifyException(IN_EXC_MSG("disabling watch failed"), errno, this);
			}
			m_pInotify->m_watches.erase(m_wd);
			m_wd = -1;
		}
	}

	m_fEnabled = fEnabled;
}

bool InotifyWatch::IsEnabled() const
{
	return m_fEnabled;
}

void InotifyWatch::__Disable()
{
	if(!m_fEnabled)
	{
		throw InotifyException(IN_EXC_MSG("event cannot occur on disabled watch"), EINVAL, this);
	}

	if(m_pInotify != nullptr)
	{
		m_pInotify->m_watches.erase(m_wd);
		m_wd = -1;
	}

	m_fEnabled = false;
}


Inotify::Inotify()
{
	m_fd = inotify_init();
	if(m_fd == -1)
	{
		throw InotifyException(IN_EXC_MSG("inotify init failed"), errno, nullptr);
	}
}

Inotify::~Inotify()
{
	RemoveAll();
	close(m_fd);
}

void Inotify::Add(InotifyWatch* pWatch)
{
	// this path already watched - go away
	if(FindWatch(pWatch->GetPath()) != nullptr)
	{
		throw InotifyException(IN_EXC_MSG("path already watched"), EBUSY, this);
	}

	// for enabled watch
	if(pWatch->IsEnabled())
	{
		// try to add watch to kernel
		int wd = inotify_add_watch(m_fd, pWatch->GetPath().c_str(), pWatch->GetMask());

		// adding failed - go away
		if(wd == -1)
		{
			throw InotifyException(IN_EXC_MSG("adding watch failed"), errno, this);
		}

		// this path already watched (but defined another way)
		InotifyWatch* pW = FindWatch(wd);
		if(pW != nullptr)
		{
			// try to recover old watch because it may be modified - then go away
			if(inotify_add_watch(m_fd, pW->GetPath().c_str(), pW->GetMask()) < 0)
			{
				throw InotifyException(IN_EXC_MSG("watch collision detected and recovery failed"), errno, this);
			}
			else
			{
				// recovery failed - go away
				throw InotifyException(IN_EXC_MSG("path already watched (but defined another way)"), EBUSY, this);
			}
		}

		pWatch->m_wd = wd;
		m_watches.insert(IN_WATCH_MAP::value_type(pWatch->m_wd, pWatch));
	}

	m_paths.insert(IN_WP_MAP::value_type(pWatch->m_path, pWatch));
	pWatch->m_pInotify = this;
}

void Inotify::Add(InotifyWatch& rWatch)
{
	Add(&rWatch);
}

void Inotify::Remove(InotifyWatch* pWatch)
{
	// invalid descriptor - this case shouldn't occur - go away
	if(m_fd == -1)
	{
		throw InotifyException(IN_EXC_MSG("invalid file descriptor"), EBUSY, this);
	}

	// for enabled watch
	if(pWatch->m_wd != -1)
	{
		// removing watch failed - go away
		if(inotify_rm_watch(m_fd, pWatch->m_wd) == -1)
		{
			throw InotifyException(IN_EXC_MSG("removing watch failed"), errno, this);
		}
		m_watches.erase(pWatch->m_wd);
		pWatch->m_wd = -1;
	}

	m_paths.erase(pWatch->m_path);
	pWatch->m_pInotify = nullptr;
}

void Inotify::Remove(InotifyWatch& rWatch)
{
	Remove(&rWatch);
}

void Inotify::RemoveAll()
{
	IN_WP_MAP::iterator it = m_paths.begin();
	while(it != m_paths.end())
	{
		InotifyWatch* pW = (*it).second;
		if(pW->m_wd != -1)
		{
			inotify_rm_watch(m_fd, pW->m_wd);
			pW->m_wd = -1;
		}
		pW->m_pInotify = nullptr;
		++it;
	}

	m_watches.clear();
	m_paths.clear();
}

std::size_t Inotify::GetWatchCount() const
{
	return m_paths.size();
}

std::size_t Inotify::GetEnabledCount() const
{
	return m_watches.size();
}

void Inotify::WaitForEvents(const bool fNoIntr)
{
	ssize_t len = 0;

	do
	{
		len = read(m_fd, m_buf, INOTIFY_BUFLEN);
	}
	while(fNoIntr && len == -1 && errno == EINTR);

	if(len == -1)
	{
		if(errno != EWOULDBLOCK && errno != EINTR)
		{
			throw InotifyException(IN_EXC_MSG("reading events failed"), errno, this);
		}
		return;
	}

	for(ssize_t i = 0; i < len;)
	{
		inotify_event* pEvt = reinterpret_cast<inotify_event*>(&m_buf[i]);
		InotifyWatch* pW = FindWatch(pEvt->wd);
		if(pW != nullptr)
		{
			InotifyEvent evt(pEvt, pW);
			if(InotifyEvent::IsType(pW->GetMask(), IN_ONESHOT)
			|| InotifyEvent::IsType(evt.GetMask(), IN_IGNORED))
			{
				pW->__Disable();
			}
			m_events.emplace_back(evt);
		}
		i += INOTIFY_EVENT_SIZE + pEvt->len;
	}
}

std::size_t Inotify::GetEventCount() const
{
	return m_events.size();
}

bool Inotify::PopEvent(InotifyEvent* pEvt)
{
	if(pEvt == nullptr)
	{
		throw InotifyException(IN_EXC_MSG("null pointer to event"), EINVAL, this);
	}

	bool b = !m_events.empty();
	if(b)
	{
		*pEvt = m_events.front();
		m_events.pop_front();
	}

	return b;
}

bool Inotify::PopEvent(InotifyEvent& rEvt)
{
	return PopEvent(&rEvt);
}

bool Inotify::PeekEvent(InotifyEvent* pEvt)
{
	if(pEvt == nullptr)
	{
		throw InotifyException(IN_EXC_MSG("null pointer to event"), EINVAL, this);
	}

	const bool b = !m_events.empty();
	if(b)
	{
		*pEvt = m_events.front();
	}

	return b;
}

bool Inotify::PeekEvent(InotifyEvent& rEvt)
{
	return PeekEvent(&rEvt);
}

InotifyWatch* Inotify::FindWatch(int iDescriptor)
{
	IN_WATCH_MAP::iterator it = m_watches.find(iDescriptor);
	return it == m_watches.cend() ? nullptr : it->second;
}

InotifyWatch* Inotify::FindWatch(const std::string& rPath)
{
	IN_WP_MAP::iterator it = m_paths.find(rPath);
	return it == m_paths.cend() ? nullptr : it->second;
}

bool Inotify::HasWatch(const std::string& path) const
{
	return m_paths.find(path) != m_paths.cend();
}

int Inotify::GetDescriptor() const
{
	return m_fd;
}

void Inotify::SetNonBlock(bool fNonBlock)
{
	if(m_fd == -1)
	{
		throw InotifyException(IN_EXC_MSG("invalid file descriptor"), EBUSY, this);
	}

	int res = fcntl(m_fd, F_GETFL);
	if(res == -1)
	{
		throw InotifyException(IN_EXC_MSG("cannot get inotify flags"), errno, this);
	}

	if(fNonBlock)
	{
		res |= O_NONBLOCK;
	}
	else
	{
		res &= ~O_NONBLOCK;
	}

	if(fcntl(m_fd, F_SETFL, res) == -1)
	{
		throw InotifyException(IN_EXC_MSG("cannot set inotify flags"), errno, this);
	}
}

void Inotify::SetCloseOnExec(bool fClOnEx)
{
	if(m_fd == -1)
	{
		throw InotifyException(IN_EXC_MSG("invalid file descriptor"), EBUSY, this);
	}

	int res = fcntl(m_fd, F_GETFD);
	if(res == -1)
	{
		throw InotifyException(IN_EXC_MSG("cannot get inotify flags"), errno, this);
	}

	if(fClOnEx)
	{
		res |= FD_CLOEXEC;
	}
	else
	{
		res &= ~FD_CLOEXEC;
	}

	if(fcntl(m_fd, F_SETFD, res) == -1)
	{
		throw InotifyException(IN_EXC_MSG("cannot set inotify flags"), errno, this);
	}
}

uint32_t Inotify::GetCapability(InotifyCapability cap)
{
	std::ifstream f(GetCapabilityPath(cap));
	if(!f)
	{
		throw InotifyException(IN_EXC_MSG("cannot get capability"), errno);
	}

	uint32_t val;
	if(!(f >> val))
	{
		throw InotifyException(IN_EXC_MSG("cannot get capability"), EIO);
	}

	return val;
}

void Inotify::SetCapability(InotifyCapability cap, uint32_t val)
{
	std::ofstream f(GetCapabilityPath(cap));
	if(!f)
	{
		throw InotifyException(IN_EXC_MSG("cannot set capability"), errno);
	}

	if(!(f << val))
	{
		throw InotifyException(IN_EXC_MSG("cannot set capability"), EIO);
	}
}

std::string Inotify::GetCapabilityPath(InotifyCapability cap)
{
	std::string path = "/proc/sys/fs/inotify/";

	switch(cap)
	{
		case InotifyCapability::max_events:
			return path + "max_queued_events";
		case InotifyCapability::max_instances:
			return path + "max_user_instances";
		case InotifyCapability::max_watches:
			return path + "max_user_watches";
	}

	throw InotifyException(IN_EXC_MSG("unknown capability type"), EINVAL);
}
