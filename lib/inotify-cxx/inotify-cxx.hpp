/// inotify C++ interface header
/**
 * inotify C++ interface
 *
 * Copyright (C) 2006, 2007, 2009 Lukas Jelinek, <lukas@aiken.cz>
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
 *
 */


#pragma once

#include <deque>
#include <map>
#include <stdexcept>
#include <stdint.h>
#include <string>

// Please ensure that the following header file takes the right place
#include <sys/inotify.h>


/// Event struct size
#define INOTIFY_EVENT_SIZE (sizeof(inotify_event))

/// Event buffer length
#define INOTIFY_BUFLEN (1024 * (INOTIFY_EVENT_SIZE + 16))

/// Helper macro for creating exception messages.
/**
 * It prepends the message by the function name.
 */
#define IN_EXC_MSG(msg) (std::string(__PRETTY_FUNCTION__) + ": " + msg)

/// inotify capability/limit identifiers
enum class InotifyCapability
{
	max_events,    // max. events in the kernel queue
	max_instances, // max. inotify file descriptors per process
	max_watches,   // max. watches per file descriptor
};


// forward declaration
class InotifyWatch;
class Inotify;


/// Class for inotify exceptions
/**
 * This class allows to acquire information about exceptional
 * events. It makes easier to log or display error messages
 * and to identify problematic code locations.
 *
 * Although this class is basically thread-safe it is not intended
 * to be shared between threads.
 */
class InotifyException : public std::runtime_error
{
public:
	/**
	 * \param[in] iErr error number (see errno.h)
	 * \param[in] pSrc source
	 */
	InotifyException(const std::string& what_arg, int iErr = 0, void* pSrc = nullptr);
	virtual ~InotifyException();

	/// Returns the exception error number.
	/**
	 * If not applicable this value is 0 (zero).
	 *
	 * \return error number (standardized; see errno.h)
	 */
	int GetErrno() const;

	/// Returns the exception source.
	/**
	 * \return source
	 */
	void* GetSource() const;

protected:
	int m_err;            // error number
	mutable void* m_pSrc; // source
};


/// inotify event class
/**
 * It holds all information about inotify event and provides
 * access to its particular values.
 *
 * This class is not (and is not intended to be) thread-safe
 * and therefore it must not be used concurrently in multiple
 * threads.
 */
class InotifyEvent
{
public:
	/**
	 * Creates a plain event.
	 */
	InotifyEvent()
	:
		m_uMask(0),
		m_uCookie(0),
		m_pWatch(nullptr)
	{
	}

	/**
	 * Creates an event based on inotify event data.
	 * For null pointers it works the same way as InotifyEvent().
	 *
	 * \param[in] pEvt event data
	 * \param[in] pWatch inotify watch
	 */
	InotifyEvent(const inotify_event* pEvt, InotifyWatch* pWatch)
	:
		m_uMask(0),
		m_uCookie(0)
	{
		if(pEvt != nullptr)
		{
			m_uMask = pEvt->mask;
			m_uCookie = pEvt->cookie;
			if(pEvt->len > 0)
			{
				m_name = pEvt->name;
			}
			m_pWatch = pWatch;
		}
		else
		{
			m_pWatch = nullptr;
		}
	}

	uint32_t GetMask() const;

	/// Checks a value for the event type.
	/**
	 * \param[in] value checked value
	 * \param[in] type type which is checked for
	 * \return true = the value contains the given type, false = otherwise
	 */
	static bool IsType(uint32_t value, uint32_t type);

	/// Checks for the event type.
	/**
	 * \param[in] type type which is checked for
	 * \return true = event mask contains the given type, false = otherwise
	 */
	bool IsType(uint32_t type) const;

	uint32_t GetCookie() const;

	std::string GetName() const;

	InotifyWatch* GetWatch();

private:
	uint32_t m_uMask;           ///< mask
	uint32_t m_uCookie;         ///< cookie
	std::string m_name;         ///< name
	InotifyWatch* m_pWatch;     ///< source watch
};



/// inotify watch class
/**
 * It holds information about the inotify watch on a particular
 * inode.
 */
class InotifyWatch
{
public:
	/**
	 * Creates an inotify watch. Because this watch is
	 * inactive it has an invalid descriptor (-1).
	 *
	 * \param[in] path watched file path
	 * \param[in] mask mask for events
	 * \param[in] enabled events enabled yes/no
	 */
	InotifyWatch(const std::string& path, uint32_t mask, bool enabled = true);

	std::string GetPath() const;

	uint32_t GetMask() const;

	/// Sets the watch event mask.
	/**
	 * If the watch is active (added to an instance of Inotify)
	 * this method may fail due to unsuccessful re-setting
	 * the watch in the kernel.
	 *
	 * \param[in] mask event mask
	 *
	 * \throw InotifyException thrown if changing fails
	 */
	void SetMask(uint32_t mask);

	/// Returns the appropriate inotify class instance.
	/**
	 * \return inotify instance
	 */
	Inotify* GetInotify();

	/// Enables/disables the watch.
	/**
	 * If the watch is active (added to an instance of Inotify)
	 * this method may fail due to unsuccessful re-setting
	 * the watch in the kernel.
	 *
	 * Re-setting the current state has no effect.
	 *
	 * \param[in] fEnabled set enabled yes/no
	 *
	 * \throw InotifyException thrown if enabling/disabling fails
	 */
	void SetEnabled(bool fEnabled);

	/// Checks whether the watch is enabled.
	/**
	 * \return true = enables, false = disabled
	 */
	bool IsEnabled() const;

private:
	friend class Inotify;

	std::string m_path;   ///< watched file path
	uint32_t m_uMask;     ///< event mask
	int32_t m_wd;         ///< watch descriptor
	Inotify* m_pInotify;  ///< inotify object
	bool m_fEnabled;      ///< events enabled yes/no

	/// Disables the watch (due to removing by the kernel).
	/**
	 * This method must be called after receiving an event.
	 * It ensures the watch object is consistent with the kernel
	 * data.
	 */
	void __Disable();
};


/// Mapping from watch descriptors to watch objects.
using IN_WATCH_MAP = std::map<int32_t, InotifyWatch*>;

/// Mapping from paths to watch objects.
using IN_WP_MAP = std::map<std::string, InotifyWatch*>;


/// inotify class
/**
 * It holds information about the inotify device descriptor
 * and manages the event queue.
 */
class Inotify
{
public:
	/**
	 * Creates and initializes an instance of inotify communication
	 * object (opens the inotify device).
	 *
	 * \throw InotifyException thrown if inotify isn't available
	 */
	Inotify();
	~Inotify();

	/// Adds a new watch.
	/**
	 * \param[in] pWatch inotify watch
	 *
	 * \throw InotifyException thrown if adding failed
	 */
	void Add(InotifyWatch* pWatch);

	/// Adds a new watch.
	/**
	 * \param[in] rWatch inotify watch
	 *
	 * \throw InotifyException thrown if adding failed
	 */
	void Add(InotifyWatch& rWatch);

	/// Removes a watch.
	/**
	 * If the given watch is not present it does nothing.
	 *
	 * \param[in] pWatch inotify watch
	 *
	 * \throw InotifyException thrown if removing failed
	 */
	void Remove(InotifyWatch* pWatch);

	/// Removes a watch.
	/**
	 * If the given watch is not present it does nothing.
	 *
	 * \param[in] rWatch inotify watch
	 *
	 * \throw InotifyException thrown if removing failed
	 */
	void Remove(InotifyWatch& rWatch);

	/// Removes all watches.
	void RemoveAll();

	/// Returns the count of watches.
	/**
	 * This is the total count of all watches (regardless whether
	 * enabled or not).
	 *
	 * \return count of watches
	 *
	 * \sa GetEnabledCount()
	 */
	std::size_t GetWatchCount() const;

	/// Returns the count of enabled watches.
	/**
	 * \return count of enabled watches
	 *
	 * \sa GetWatchCount()
	 */
	std::size_t GetEnabledCount() const;

	/// Waits for inotify events.
	/**
	 * It waits until one or more events occur. When called
	 * in nonblocking mode it only retrieves occurred events
	 * to the internal queue and exits.
	 *
	 * \param[in] fNoIntr if true it re-calls the system call after a handled signal
	 *
	 * \throw InotifyException thrown if reading events failed
	 *
	 * \sa SetNonBlock()
	 */
	void WaitForEvents(bool fNoIntr = false);

	/// Returns the count of received and queued events.
	/**
	 * This number is related to the events in the queue inside
	 * this object, not to the events pending in the kernel.
	 *
	 * \return count of events
	 */
	std::size_t GetEventCount() const;

	/// Extracts a queued inotify event.
	/**
	 * The extracted event is removed from the queue.
	 * If the pointer is null, it does nothing.
	 *
	 * \param[in,out] pEvt event object
	 *
	 * \throw InotifyException thrown if the provided pointer is null
	 */
	bool PopEvent(InotifyEvent* pEvt);

	/// Extracts a queued inotify event.
	/**
	 * The extracted event is removed from the queue.
	 *
	 * \param[in,out] rEvt event object
	 *
	 * \throw InotifyException thrown only in very anomalous cases
	 */
	bool PopEvent(InotifyEvent& rEvt);

	/// Extracts a queued inotify event (without removing).
	/**
	 * The extracted event stays in the queue.
	 * If the pointer is null, it does nothing.
	 *
	 * \param[in,out] pEvt event object
	 *
	 * \throw InotifyException thrown if the provided pointer is null
	 */
	bool PeekEvent(InotifyEvent* pEvt);

	/// Extracts a queued inotify event (without removing).
	/**
	 * The extracted event stays in the queue.
	 *
	 * \param[in,out] rEvt event object
	 *
	 * \throw InotifyException thrown only in very anomalous cases
	 */
	bool PeekEvent(InotifyEvent& rEvt);

	/// Searches for a watch by a watch descriptor.
	/**
	 * It tries to find a watch by the given descriptor.
	 *
	 * \param[in] iDescriptor watch descriptor
	 * \return pointer to a watch; null if no such watch exists
	 */
	InotifyWatch* FindWatch(int iDescriptor);

	/// Searches for a watch by a filesystem path.
	/**
	 * It tries to find a watch by the given filesystem path.
	 *
	 * \param[in] rPath filesystem path
	 * \return pointer to a watch; null if no such watch exists
	 *
	 * \attention The path must be exactly identical to the one
	 *            used for the searched watch. Be careful about
	 *            absolute/relative and case-insensitive paths.
	 */
	InotifyWatch* FindWatch(const std::string& rPath);

	bool HasWatch(const std::string& path) const;

	/// Returns the file descriptor.
	/**
	 * The descriptor can be used in standard low-level file
	 * functions (poll(), select(), fcntl() etc.).
	 *
	 * \return valid file descriptor or -1 for inactive object
	 *
	 * \sa SetNonBlock()
	 */
	int GetDescriptor() const;

	/// Enables/disables non-blocking mode.
	/**
	 * Use this mode if you want to monitor the descriptor
	 * (acquired thru GetDescriptor()) in functions such as
	 * poll(), select() etc.
	 *
	 * Non-blocking mode is disabled by default.
	 *
	 * \param[in] fNonBlock enable/disable non-blocking mode
	 *
	 * \throw InotifyException thrown if setting mode failed
	 *
	 * \sa GetDescriptor(), SetCloseOnExec()
	 */
	void SetNonBlock(bool fNonBlock);

	/// Enables/disables closing on exec.
	/**
	 * Enable this if you want to close the descriptor when
	 * executing another program. Otherwise, the descriptor
	 * will be inherited.
	 *
	 * Closing on exec is disabled by default.
	 *
	 * \param[in] fClOnEx enable/disable closing on exec
	 *
	 * \throw InotifyException thrown if setting failed
	 *
	 * \sa GetDescriptor(), SetNonBlock()
	 */
	void SetCloseOnExec(bool fClOnEx);

	/// Acquires a particular inotify capability/limit.
	/**
	 * \param[in] cap capability/limit identifier
	 * \return capability/limit value
	 * \throw InotifyException thrown if the given value cannot be acquired
	 */
	static uint32_t GetCapability(InotifyCapability cap);

	/// Modifies a particular inotify capability/limit.
	/**
	 * \param[in] cap capability/limit identifier
	 * \param[in] val new capability/limit value
	 * \throw InotifyException thrown if the given value cannot be set
	 * \attention Using this function requires root privileges.
	 *            Beware of setting extensive values - it may seriously
	 *            affect system performance and/or stability.
	 */
	static void SetCapability(InotifyCapability cap, uint32_t val);

private:
	int m_fd;                            // file descriptor
	IN_WATCH_MAP m_watches;              // watches (by descriptors)
	IN_WP_MAP m_paths;                   // watches (by paths)
	unsigned char m_buf[INOTIFY_BUFLEN]; // buffer for events
	std::deque<InotifyEvent> m_events;   // event queue

	friend class InotifyWatch;

	static std::string GetCapabilityPath(InotifyCapability cap);
};
