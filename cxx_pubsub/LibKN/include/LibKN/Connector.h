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

#if !defined(LIBKN_CONNECTOR_H)
#define LIBKN_CONNECTOR_H

#include <LibKN\Defs.h>
#include <LibKN\Message.h>
#include <LibKN\Transport.h>
#include <LibKN\IListener.h>
#include <LibKN\IRequestStatusHandler.h>
#include <LibKN\IConnectionStatusHandler.h>
#include <LibKN\RandRHandler.h>
#include <LibKN\Journal.h>
#include <LibKN\CS.h>
#include <LibKN\OfflineQ.h>

/**
 * The Connector class is the main class to communicate with the server.
 */
class Connector : public CCriticalSection, public ITransport
{
friend class Journal;
friend class SimpleParser;
friend class OfflineQueue;

public:
	typedef LockImpl<Connector> Lock;

	/**
	 * Initializes the object to the disconnected state.
	 */
	Connector();
	~Connector();

	/**
	 * \return True if the Connector is connected. False otherwise.
	 */
	bool IsConnected();

	/**
	 * Connects to the server.
	 * \return True if successfully connects. False otherwise.
	 */
	bool Open(const ITransport::Parameters& p);

	/**
	 * \return The parameters used to connect. If not connected, then the data will be empty.
	 */
	const Parameters& GetParameters() const;

	/**
	 * Closes the connection. Returns the connector to the disconnected state.
	 * \return True if the connection closes properly. Otherwise false.
	 */
	bool Close();

	/**
	 * This method ensures the connector is successfully connected to the server. It must be called
	 * after calling Open() at least once.
	 * \return True if successfully connected.
	 */
	bool EnsureConnected();

	/**
	 * Publishes the message to the server.
	 * \return True if it was successfully sent.
	 */
	bool Publish(const Message& msg, IRequestStatusHandler* sh);

	/**
	 * Subscribes to a topic.
	 * \return The subscription identifier for the topic. This is an empty string if the Subscribe call failed.
	 */
	wstring Subscribe(const wstring& topic, IListener* listener, const Message& options, IRequestStatusHandler* sh);

	/**
	 * Unsubscribes to a previously subscribed topic.
	 * \return True if successfully unsubscribed.
	 */
	bool Unsubscribe(const wstring& rid, IRequestStatusHandler* sh);
	
	/**
	 * Indicate interest in the connection status.
	 * The handler will be called whenever the connection status changes.
	 */
	void AddConnectionStatusHandler(IConnectionStatusHandler* sh);

	/**
	 * Indicate disinterest in the connection status.
	 * The handler will no longer be called whenever the connection status changes.
	 */
	void RemoveConnectionStatusHandler(IConnectionStatusHandler* sh);

	/**
	 * Get the route id for the topic. Use this method in conjunction with SubscribeRouteId() to simulate the Subscribe() method.
	 * This method allows separation of Subscribe's tasks.
	 * \return The subscription identifier for the topic. This is an empty string if the Subscribe call failed.
	 */
	wstring GetRouteId(const wstring& topic, Message& msg) const;

	/**
	 * Subscribe to the route id. Use this method in conjunction with GetRouteId() to simulate the Subscribe() method.
	 * This method allows separation of Subscribe's tasks.
	 * \return True if the subscription was successful. False otherwise.
	 */
	bool SubscribeRouteId(Message& msg, const wstring& rid, const wstring& topic, IListener* listener, IRequestStatusHandler* sh);

private:
	void OnFireEvent(const Message& msg);
	void OnConnectionStatus(const Message& msg);
	void OnConnectionStatusImpl(const Message& msg);

private:
	typedef map<wstring, IListener*> Listeners;
	typedef list<IConnectionStatusHandler*> ConnectionStatusHandlers;

	Transport& GetTransport();
	RandRHandler& GetRandR();

	wstring GetSubscriptionId() const;
	pair<wstring, wstring> ParseRouteId(const wstring& rid);
	wstring Route(const wstring& from, const wstring& to, IListener* listener, IRequestStatusHandler* sh);
	wstring Route(Message& msg, const wstring& rid, const wstring& from, const wstring& to, IListener* listener, IRequestStatusHandler* sh);
	bool Post(const Message& msg, IRequestStatusHandler* sh);
	wstring GetRouteIdImpl(const wstring& topic, Message& msg) const;

	void FlushQueue();

	Transport m_Transport;
	RandRHandler m_RandR;
	Journal m_Journal;
	Listeners m_Listeners;
	ConnectionStatusHandlers m_ConnectionStatusHandlers;
	OfflineQueue m_OfflineQueue;

private:
	// Cannot copy
	//
	Connector& operator=(const Connector& rhs);
};

#endif
