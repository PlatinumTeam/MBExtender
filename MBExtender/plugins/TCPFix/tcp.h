//-----------------------------------------------------------------------------
// tcp.h
//
// Copyright (c) 2015 The Platinum Team
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

#import <Cocoa/Cocoa.h>
#include "GCDAsyncSocket.h"

class TCPSocket;

/**
 * An Objective-C bridging interface for the TCP library.
 */
@interface TCPInterface : NSObject<GCDAsyncSocketDelegate> {
@public
    GCDAsyncSocket *mSocket;
    TCPSocket *mCSock;

	dispatch_queue_t socketQueue;
	dispatch_queue_t delegateQueue;

	NSMutableData *mBufferData;

	NSString *mAddress;
	unsigned short mPort;

	uint64_t mLines;
}

/**
 * Connect to a server.
 * @arg address The address of the server for which to connect.
 */
-(void) connect:(const char*)address;
/**
 * Send a message through the socket.
 * @arg string The message to send.
 */
-(void) send:(const char*)string;
/**
 * Disconnect the socket.
 */
-(void) disconnect;
/**
 * Main-thread callback for bridging received lines to C++
 */
-(void) scriptOnLine:(NSString *)line;
/**
 * Main-thread callback for bridging connections to C++
 */
-(void) scriptOnConnect;
/**
 * Main-thread callback for bridging disconnects to C++
 */
-(void) scriptOnDisconnect;
/**
 * Main-thread callback for bridging connection failures to C++
 */
-(void) scriptOnConnectFailed;
/**
 * Initialize a new TCPInterface.
 * @arg socket The C++ socket bridge.
 */
-(id) init:(TCPSocket*)socket;

@end

/**
 * The C++ side of the bridging interface.
 */
class TCPSocket {
protected:
	/**
	 * @var mInterface The Objective-C briding interface object.
	 */
    TCPInterface *mInterface;

public:
	/**
	 * Create a new TCPSocket
	 */
    TCPSocket();
	/**
	 * Destroy a TCPSocket
	 */
    ~TCPSocket();

	/**
	 * Callback: Process a line from the server.
	 * @arg buffer The line to process.
	 */
    void processLine(const char *buffer);
	/**
	 * Callback: When the connection is accepted.
	 */
    void onConnected();
	/**
	 * Callback: When the connection fails.
	 */
    void onConnectFailed();
	/**
	 * Callback: When the connection disconnects.
	 */
    void onDisconnect();

	/**
	 * Send data to the socket.
	 * @arg data The data to send.
	 */
    void send(const char* data);
	/**
	 * Connect to a server.
	 * @arg address The address of the server for which to connect.
	 */
    void connect(const char* address);
	/**
	 * Listen on a port (unimplemented)
	 * @arg port The port on which to listen.
	 */
    void listen(unsigned short port);
	/**
	 * Disconnect the socket.
	 */
    void disconnect();
};

/**
 * Initialize the TCPSocket library.
 */
void initTCPSocket();
