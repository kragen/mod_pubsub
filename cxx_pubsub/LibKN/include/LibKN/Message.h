/*
Copyright (c) 2000-2003 KnowNow, Inc.  All rights reserved.

@KNOWNOW_LICENSE_START@

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

1. Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in
the documentation and/or other materials provided with the
distribution.

3. The name "KnowNow" is a trademark of KnowNow, Inc. and may not
be used to endorse or promote any product without prior written
permission from KnowNow, Inc.

THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESSED OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL KNOWNOW, INC. OR ITS CONTRIBUTORS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

@KNOWNOW_LICENSE_END@
*/

#if !defined(LIBKN_MESSAGE_H)
#define LIBKN_MESSAGE_H

#include <LibKN\Defs.h>
#include <LibKN\HttpParamString.h>
#include <LibKN\CS.h>

/** 
 * This class provides a set of field-value pairs.
 */
class Message : public CCriticalSection
{
public:
	typedef LockImpl<Message> Lock;
	typedef map<wstring, wstring> Container;

	/**
	 * Creates an empty Message.
	 */
	Message();

	/**
	 * Creates a copy of a Message.
	 */
	Message(const Message& rhs);

	/**
	 * Destroys the message. Provides cleanup.
	 */
	~Message();

	/**
	 * Deep copies a Message.
	 * All field-value pairs are copied.
	 */
	Message& operator=(const Message& rhs);

	/**
	 * Sets the value <em>v</em> for the field <em>f</em>. 
	 * If the field already exists, the value is overwritten.
	 */
	void Set(const wstring& f, const wstring& v);

	/**
	 * Sets the value <em>v</em> for the field <em>f</em>. 
	 * If the field already exists, the value is overwritten.
	 * This function is provided as a convenience function. Calls the wide version internally.
	 */
	void Set(const string& f, const string& v);

	/**
	 * Removes the field from the message.
	 * If the field does not exist, nothing happens.
	 */
	void Remove(const wstring& f);

	/**
	 * Removes the field from the message.
	 * If the field does not exist, nothing happens.
	 * This function is provided as a convenience function. Calls the wide version internally.
	 */
	void Remove(const string& f);

	/**
	 * Retrieves the value for the field.
	 * \return True if the field exist and the value is copied to <em>v</em>. 
	 * Otherwise, returns False and <em>v</em> is untouched.
	 */
	bool Get(const wstring& f, wstring& v) const;
	
	/**
	 * Retrieves the value for the field.
	 * \return True if the field exist and the value is copied to <em>v</em>. 
	 * Otherwise, returns False and <em>v</em> is untouched.
	 * This function is provided as a convenience function. Calls the wide version internally.
	 */
	bool Get(const string& f, wstring& v) const;

	/**
	 * This function empties the message. Makes it ready for reuse.
	 */
	void Empty();

	/**
	 * \return True if this message is empty. Otherwise False.
	 */
	bool IsEmpty() const;

	/**
	 * \return The underlying container for easy enumerating.
	 */
	const Container& GetContainer() const;

	/**
	 * \return True if any of the fields/values have changed since the last time GetAsHttpParam() was called.
	 * This provides a caching optimization.
	 */
	bool HasItemsChanged() const;

	/**
	 * \return True if all of the field/value pairs of this message is equal to the other message.
	 */
	bool operator==(const Message& rhs);

	/**
	 * \return Retrieve the Http-encoded version of this message.
	 * Used by the transport class to send to the server.
	 */
	const string& GetAsHttpParam() const;

private:
	void ConvertToHttpParam() const;

	Container m_Container;

	bool m_ItemsHaveChanged;
	HttpParamString m_HttpParamString;
};

#endif
