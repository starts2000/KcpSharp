#include "KcpSharp.h"
#include "KcpException.h"
#include <memory.h>

Starts2000::KcpSharp::Kcp::Kcp(UInt32 conv, Object^ user)
{
	this->user = user;

	if (nullptr == this->user)
	{
		this->kcp = ikcp_create(conv, nullptr);
	}
	else
	{
		this->userGCHandle = GCHandle::Alloc(user);
		this->kcp = ikcp_create(conv, GCHandle::ToIntPtr(this->userGCHandle).ToPointer());
	}

	if (nullptr == this->kcp) {
		throw gcnew KcpException("kcp initialize failed!");
	}
}

Starts2000::KcpSharp::Kcp::~Kcp()
{
	if (this->outputHandlerGcHandle.IsAllocated) {
		this->outputHandlerGcHandle.Free();
	}

	if (this->logHandlerGcHandle.IsAllocated) {
		this->logHandlerGcHandle.Free();
	}
}

Starts2000::KcpSharp::Kcp::!Kcp()
{
	if (nullptr != kcp) {
		ikcp_release(kcp);
	}
}

void Starts2000::KcpSharp::Kcp::SetOutput(KcpOutputHandler ^ outputHandler)
{
	CheckKcp();

	if (this->outputHandlerGcHandle.IsAllocated) {
		this->outputHandlerGcHandle.Free();
	}

	this->outputHandlerGcHandle = GCHandle::Alloc(outputHandler);
	auto pOutput = static_cast<int(*)(const char *buf, int len, struct IKCPCB *kcp, void *user)>(
		Marshal::GetFunctionPointerForDelegate(outputHandler).ToPointer());
	ikcp_setoutput(kcp, pOutput);
}

int Starts2000::KcpSharp::Kcp::Recv(array<Byte>^ buffer, int len)
{
	CheckKcp();
	pin_ptr<void> pBuffer = &buffer[0];
	return ikcp_recv(kcp, static_cast<char *>(pBuffer), len);
}


int Starts2000::KcpSharp::Kcp::Send(array<Byte>^ buffer, int len)
{
	CheckKcp();
	pin_ptr<void> pBuffer = &buffer[0];
	return ikcp_send(kcp, static_cast<const char *>(pBuffer), len);
}


void Starts2000::KcpSharp::Kcp::Update(UInt32 current)
{
	CheckKcp();
	ikcp_update(kcp, current);
}


UInt32 Starts2000::KcpSharp::Kcp::Check(UInt32 current)
{
	CheckKcp();
	return ikcp_check(kcp, current);
}


int Starts2000::KcpSharp::Kcp::Input(array<Byte>^ data, int size)
{
	CheckKcp();
	pin_ptr<void> pData = &data[0];
	return ikcp_input(kcp, static_cast<const char *>(pData), size);
}


void Starts2000::KcpSharp::Kcp::Flush()
{
	CheckKcp();
	ikcp_flush(kcp);
}


int Starts2000::KcpSharp::Kcp::PeekSize()
{
	CheckKcp();
	return ikcp_peeksize(kcp);
}


int Starts2000::KcpSharp::Kcp::SetMtu(int mtu)
{
	CheckKcp();
	return ikcp_setmtu(kcp, mtu);
}


int Starts2000::KcpSharp::Kcp::SetWndSize(int sndwnd, int rcvwnd)
{
	CheckKcp();
	return ikcp_wndsize(kcp, sndwnd, rcvwnd);
}


int Starts2000::KcpSharp::Kcp::GetWaitSnd()
{
	CheckKcp();
	return ikcp_waitsnd(kcp);
}


int Starts2000::KcpSharp::Kcp::Nodelay(bool nodelay, int interval, int resend, bool nc)
{
	CheckKcp();
	return ikcp_nodelay(kcp, nodelay, interval, resend, nc);
}


void Starts2000::KcpSharp::Kcp::SetLogMask(KcpLogMask logMask)
{
	CheckKcp();
	kcp->logmask = safe_cast<int>(logMask);
}


void Starts2000::KcpSharp::Kcp::SetLog(KcpLogHandler ^ loghandler)
{
	CheckKcp();

	if (this->logHandlerGcHandle.IsAllocated) {
		this->logHandlerGcHandle.Free();
	}

	this->logHandlerGcHandle = GCHandle::Alloc(loghandler);

	auto pLog = static_cast<void(*)(const char *log, struct IKCPCB *kcp, void *user)>(
		Marshal::GetFunctionPointerForDelegate(loghandler).ToPointer());
	kcp->writelog = pLog;
}

void Starts2000::KcpSharp::Kcp::Allocator(KcpMallocHandler^ mallocHandler, KcpFreeHandler^ freeHandler)
{
	if (mallocHandlerGCHandle.IsAllocated) {
		mallocHandlerGCHandle.Free();
	}

	if (freeHandlerGCHandle.IsAllocated) {
		freeHandlerGCHandle.Free();
	}

	mallocHandlerGCHandle = GCHandle::Alloc(mallocHandler);
	freeHandlerGCHandle = GCHandle::Alloc(freeHandler);
	auto new_malloc = static_cast<void* (*)(size_t)>(
		Marshal::GetFunctionPointerForDelegate(mallocHandler).ToPointer());
	auto new_free = static_cast<void(*)(void*)>(
		Marshal::GetFunctionPointerForDelegate(freeHandler).ToPointer());
	ikcp_allocator(new_malloc, new_free);
}


UInt32 Starts2000::KcpSharp::Kcp::GetConv(IntPtr dataPtr)
{
	return ikcp_getconv(dataPtr.ToPointer());
}


UInt32 Starts2000::KcpSharp::Kcp::GetConv(array<Byte>^ data)
{
	pin_ptr<void> pData = &data[0];
	return ikcp_getconv(static_cast<const void *>(pData));
}


void Starts2000::KcpSharp::Kcp::CheckKcp()
{
	if (nullptr == this->kcp) {
		throw gcnew KcpException("kcp not initialized or initialize failed!");
	}
}
