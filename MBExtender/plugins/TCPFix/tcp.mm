//-----------------------------------------------------------------------------
// tcp.mm
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

#include "tcp.h"
#include <TorqueLib/game/net/tcpObject.h>
#include <TorqueLib/console/console.h>
#include <MBExtender/MBExtender.h>
#include <unordered_map>
#include <sys/socket.h>
#include <netinet/in.h>

//#include "mach_override.h"

#import <Cocoa/Cocoa.h>
#import <Foundation/Foundation.h>

MBX_MODULE(TCPFix);

static std::unordered_map<void *, void *> gTCPSocketMap;
static std::unordered_map<void *, void *> gTCPObjectMap;

@implementation TCPInterface

-(void) connect:(const char *)address {
	if ([mSocket isConnected]) {
		DEBUG_ERRORF("Socket connecting to %s is already connected to %s", address, [mSocket connectedAddress].UTF8String);
		return;
	}

	DEBUG_PRINTF("Connecting to %s", address);
    // just standard converting C++ to objective-c string
    // and extracting string information
    NSString *string  = [NSString stringWithUTF8String:address];
    uint16_t port = [string substringFromIndex:[string rangeOfString:@":"].location + 1].intValue;
    string = [string substringToIndex:[string rangeOfString:@":"].location];

	mAddress = string;
	mPort = port;

	NSError *err;

	DEBUG_PRINTF("Trying to connect to %s:%d", string.UTF8String, port);

    if ([mSocket connectToHost:string onPort:port error:&err]) {
		DEBUG_PRINTF("OK looks like we connected.");
	} else {
		DEBUG_PRINTF("Nope, error. Error was: %s", [err description].UTF8String);
	}
}

-(void) send:(const char*)string {
    // convert the C styled string to NSString
    // Then convert from NSString to NSData
    NSString *str = [NSString stringWithUTF8String:string];
    NSData *data = [str dataUsingEncoding:NSUTF8StringEncoding];
    [mSocket writeData:data withTimeout:5 tag:0];
}

//@callback
-(void) socket:(GCDAsyncSocket *)sock didConnectToHost:(NSString *)host port:(uint16_t)port {
    // call onConnected on main thread
    [self performSelectorOnMainThread:@selector(scriptOnConnect) withObject:nil waitUntilDone:true];

	mBufferData = [NSMutableData new];

    // tell it to read until the next line is found.
    [mSocket readDataToData:[@"\n" dataUsingEncoding:NSUTF8StringEncoding] withTimeout:-1 tag:0];
}

//@callback
-(void) socketDidDisconnect:(GCDAsyncSocket *)sock withError:(NSError *)err {
    [self performSelectorOnMainThread: (err == nil ? @selector(scriptOnConnectFailed) : @selector(scriptOnDisconnect)) withObject:nil waitUntilDone:true];
}

//@callback
-(void) socket:(GCDAsyncSocket *)sock didReadData:(NSData *)data withTag:(long)tag {
    // done
    if ([data length] == 0)
        return;

    // parse it and send it off to C++ interface
    // since this is on a seperate thread, we have to force objective-c to run
    // this method on the main thread.
    NSString *string = [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding];
	NSString *stripped = [string stringByTrimmingCharactersInSet:[NSCharacterSet newlineCharacterSet]];

    [mSocket readDataToData:[@"\n" dataUsingEncoding:NSUTF8StringEncoding] withTimeout:-1 tag:0];
	[self performSelectorOnMainThread:@selector(scriptOnLine:) withObject:stripped waitUntilDone:false];
}

-(void) socketDidCloseReadStream:(GCDAsyncSocket *)sock {
	[self performSelectorOnMainThread:@selector(scriptOnDisconnect) withObject:nil waitUntilDone:false];
}

-(void)disconnect {
    // will fire onDisconnect callback
    [mSocket disconnect];
}

//-------------------------------------------------------------------------------
// callbacks from socket thread to main thread
//-------------------------------------------------------------------------------

-(void) scriptOnConnect {
//	DEBUG_PRINTF("HOLY SHIT WE CONNECTED");
    mCSock->onConnected();
}

-(void) scriptOnLine:(NSString *)line {
	mLines ++;
//	TGE::Con::printf("So far we've had %llu lines", mLines);
//	DEBUG_PRINTF("HOLY SHIT WE GOT A LINE OF LENGTH (not printing it because some lines are huge) %d", line.length);
    mCSock->processLine(line.UTF8String);
}

-(void) scriptOnDisconnect {
//	DEBUG_PRINTF("I DIDN'T EVEN REALIZE THAT ONDISCONNECT WAS A THING!");
    mCSock->onDisconnect();
}

-(void) scriptOnConnectFailed {
    mCSock->onConnectFailed();
}

//-------------------------------------------------------------------------------

-(id) init:(TCPSocket *)socket {
    // initialize parent constructor
	if (self = [super init]) {
		// assign C++ socket to objective-c
		mCSock = socket;

		socketQueue = dispatch_queue_create("com.platinumteam.injector.socketQueue", NULL);
		delegateQueue = dispatch_queue_create("com.platinumteam.injector.delegateQueue", NULL);

		mSocket = [[GCDAsyncSocket alloc] initWithDelegate:self delegateQueue:delegateQueue socketQueue:socketQueue];

		mLines = 0;
	}
    return self;
}

@end

TCPSocket::TCPSocket() {
    // allocate mInterface and pass this pointer to store in objective-c
    mInterface = [[TCPInterface alloc] init:this];
}

TCPSocket::~TCPSocket() {
    disconnect();
    mInterface = nil;
}

void TCPSocket::connect(const char *address) {
    [mInterface connect:address];
}

void TCPSocket::send(const char *data) {
//	DEBUG_PRINTF("SOMETHING SOMETHING HERE GOES NOTHING SENDING %s", data);
    [mInterface send:data];
}

void TCPSocket::listen(unsigned short port) {

}

void TCPSocket::disconnect() {
    [mInterface disconnect];
}

//-----------------------------------------------------------------------------

void TCPSocket::onConnected() {
	DEBUG_PRINTF("Connected");
	TGE::TCPObject *obj = static_cast<TGE::TCPObject*>(gTCPObjectMap[this]);
	if (obj)
		TGE::Con::executef(obj, 1, "onConnected");
}

void TCPSocket::onConnectFailed() {
	TGE::TCPObject *obj = static_cast<TGE::TCPObject*>(gTCPObjectMap[this]);
	if (obj)
		TGE::Con::executef(obj, 1, "onConnectFailed");
}

void TCPSocket::processLine(const char *buffer) {
	TGE::TCPObject *obj = static_cast<TGE::TCPObject*>(gTCPObjectMap[this]);
	if (obj)
		TGE::Con::executef(obj, 2, "onLine", buffer);
}

void TCPSocket::onDisconnect() {
	TGE::TCPObject *obj = static_cast<TGE::TCPObject*>(gTCPObjectMap[this]);
	if (obj)
		TGE::Con::executef(obj, 1, "onDisconnect");
}

//------------------------------------------------------------------------------

/**
 * @override When a TCPObject is created, we need to match it up with a new TCPSocket.
 */
MBX_OVERRIDE_MEMBERFN(void, TGE::TCPObject::onAdd, (TGE::TCPObject *thisptr), originalOnAdd) {
	@autoreleasepool {
		TCPSocket *socket = new TCPSocket();
		DEBUG_PRINTF("Sockect cstr start");
		DEBUG_PRINTF("thisptr: %p socket: %p gTCPSocketMap: %p gTCPObjectMap: %p", thisptr, socket, &gTCPSocketMap, &gTCPObjectMap);
		gTCPSocketMap.insert(std::pair<void*, void*>(thisptr, socket));
		gTCPObjectMap.insert(std::pair<void*, void*>(socket, thisptr));
		gTCPSocketMap[thisptr] = socket;
		gTCPObjectMap[socket] = thisptr;
		DEBUG_PRINTF("Socket cstr done");
		DEBUG_PRINTF("Final map counts: %d %d", gTCPObjectMap.size(), gTCPSocketMap.size());
		originalOnAdd(thisptr);
	}
}

/**
 * @override When a TCPObject tries to connect to a server, use our library instead.
 * @arg server The server to which to connect
 */
MBX_OVERRIDE_MEMBERFN(void, TGE::TCPObject::connect, (TGE::TCPObject *thisptr, U32 argc, const char **argv), originalConnect) {
	@autoreleasepool {
		TCPSocket *sock = static_cast<TCPSocket*>(gTCPSocketMap[thisptr]);
		if (!sock) {
			DEBUG_PRINTF("OSX TCPObject::connect() :: TCPSocket deleted itself before calling connect.  Aborting operation.");
			return;
		}

		sock->connect(argv[2]);
	}
}

/**
 * @override When a TCPObject tries to send data, use our library instead.
 * @param data The data to send.
 */
MBX_OVERRIDE_MEMBERFN(void, TGE::TCPObject::send, (TGE::TCPObject *thisptr, U32 argc, const char **argv), originalSend) {
	@autoreleasepool {
		TCPSocket *sock = static_cast<TCPSocket*>(gTCPSocketMap[thisptr]);
		if (!sock) {
			DEBUG_PRINTF("OSX TCPObject::send() :: TCPSocket deleted itself before calling send.  Aborting operation.");
			return;
		}

		sock->send(argv[2]);
	}
}

/**
 * @override When an object is deleted, check to see if we need to clean up a TCPSocket.
 */
MBX_OVERRIDE_MEMBERFN(void, TGE::SimObject::deleteObject, (TGE::SimObject *thisptr), originalDeleteObject) {
	@autoreleasepool {
		// GET THE HELL OUT OF THERE
		TCPSocket *sock = static_cast<TCPSocket*>(gTCPSocketMap[thisptr]);

		if (sock) {
			gTCPSocketMap.erase(thisptr);
			gTCPObjectMap.erase(sock);

			// free memory
	//		delete sock;
	//		sock = NULL;
		}

		if (static_cast<TGE::SimObject*>(thisptr))
			originalDeleteObject(thisptr);
	}
}
