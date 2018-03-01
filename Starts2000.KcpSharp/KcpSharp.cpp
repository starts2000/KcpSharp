#include "KcpSharp.h"
#include "KcpException.h"
#include <memory.h>

generic<typename TUser>
Starts2000::KcpSharp::Kcp<TUser>::Kcp(UInt32 conv, TUser user)
{
	this->user = user;
	this->kcp = ikcp_create(conv, nullptr);

	if (nullptr == this->kcp) {
		throw gcnew KcpException("kcp initialize failed!");
	}
}

generic<typename TUser>
Starts2000::KcpSharp::Kcp<TUser>::~Kcp()
{
	if (this->userGCHandle.IsAllocated) {
		this->userGCHandle.Free();
	}

	if (this->outputHandlerGcHandle.IsAllocated) {
		this->outputHandlerGcHandle.Free();
	}

	if (this->logHandlerGcHandle.IsAllocated) {
		this->logHandlerGcHandle.Free();
	}

	this->outputHandler = nullptr;
	this->logHandler = nullptr;
}

generic<typename TUser>
Starts2000::KcpSharp::Kcp<TUser>::!Kcp()
{
	if (nullptr != kcp) {
		ikcp_release(kcp);
	}
}

generic<typename TUser>
void Starts2000::KcpSharp::Kcp<TUser>::SetOutput(KcpOutputHandler<TUser> ^ outputHandler)
{
	CheckKcp();

	if (this->outputHandlerGcHandle.IsAllocated) {
		this->outputHandlerGcHandle.Free();
	}

	this->outputHandler = outputHandler;

	auto output = gcnew InnerOutputHandler(this, &Kcp::Output);
	this->outputHandlerGcHandle = GCHandle::Alloc(output);
	auto pOutput = static_cast<int(*)(const char *buf, int len, struct IKCPCB *kcp, void *user)>(
		Marshal::GetFunctionPointerForDelegate(output).ToPointer());
	ikcp_setoutput(kcp, pOutput);
}

generic<typename TUser>
int Starts2000::KcpSharp::Kcp<TUser>::Output(const char *buf, int len, ikcpcb* kcp, void* user) {
	if (nullptr != outputHandler) {
		auto buffer = gcnew array<Byte>(len);
		pin_ptr<Byte> pBuffer = &buffer[0];
		memcpy(pBuffer, buf, len);

		return outputHandler(buffer, len, static_cast<IntPtr>(kcp), this->user);
	}

	return 0;
}

generic<typename TUser>
int Starts2000::KcpSharp::Kcp<TUser>::Recv(array<Byte>^ buffer, int len)
{
	CheckKcp();
	pin_ptr<void> pBuffer = &buffer[0];
	return ikcp_recv(kcp, static_cast<char *>(pBuffer), len);
}

generic<typename TUser>
int Starts2000::KcpSharp::Kcp<TUser>::Send(array<Byte>^ buffer, int len)
{
	CheckKcp();
	pin_ptr<void> pBuffer = &buffer[0];
	return ikcp_send(kcp, static_cast<const char *>(pBuffer), len);
}

generic<typename TUser>
void Starts2000::KcpSharp::Kcp<TUser>::Update(UInt32 current)
{
	CheckKcp();
	ikcp_update(kcp, current);
}

generic<typename TUser>
UInt32 Starts2000::KcpSharp::Kcp<TUser>::Check(UInt32 current)
{
	CheckKcp();
	return ikcp_check(kcp, current);
}

generic<typename TUser>
int Starts2000::KcpSharp::Kcp<TUser>::Input(array<Byte>^ data, int size)
{
	CheckKcp();
	pin_ptr<void> pData = &data[0];
	return ikcp_input(kcp, static_cast<const char *>(pData), size);
}

generic<typename TUser>
void Starts2000::KcpSharp::Kcp<TUser>::Flush()
{
	CheckKcp();
	ikcp_flush(kcp);
}

generic<typename TUser>
int Starts2000::KcpSharp::Kcp<TUser>::PeekSize()
{
	CheckKcp();
	return ikcp_peeksize(kcp);
}

generic<typename TUser>
int Starts2000::KcpSharp::Kcp<TUser>::SetMtu(int mtu)
{
	CheckKcp();
	return ikcp_setmtu(kcp, mtu);
}

generic<typename TUser>
int Starts2000::KcpSharp::Kcp<TUser>::SetWndSize(int sndwnd, int rcvwnd)
{
	CheckKcp();
	return ikcp_wndsize(kcp, sndwnd, rcvwnd);
}

generic<typename TUser>
int Starts2000::KcpSharp::Kcp<TUser>::GetWaitSnd()
{
	CheckKcp();
	return ikcp_waitsnd(kcp);
}

generic<typename TUser>
int Starts2000::KcpSharp::Kcp<TUser>::Nodelay(bool nodelay, int interval, int resend, bool nc)
{
	CheckKcp();
	return ikcp_nodelay(kcp, nodelay, interval, resend, nc);
}

generic<typename TUser>
void Starts2000::KcpSharp::Kcp<TUser>::SetLogMask(KcpLogMask logMask)
{
	CheckKcp();
	kcp->logmask = safe_cast<int>(logMask);
}

generic<typename TUser>
void Starts2000::KcpSharp::Kcp<TUser>::SetLog(KcpLogHandler<TUser> ^ loghandler)
{
	CheckKcp();

	if (this->logHandlerGcHandle.IsAllocated) {
		this->logHandlerGcHandle.Free();
	}

	this->logHandler = logHandler;

	auto log = gcnew InnerLogHandler(this, &Kcp::Log);
	this->logHandlerGcHandle = GCHandle::Alloc(log);

	auto pLog = static_cast<void(*)(const char *log, struct IKCPCB *kcp, void *user)>(
		Marshal::GetFunctionPointerForDelegate(log).ToPointer());
	kcp->writelog = pLog;
}

generic<typename TUser>
void Starts2000::KcpSharp::Kcp<TUser>::Log(const char * log, ikcpcb * kcp, void * user)
{
	if (nullptr != this->logHandler) {
		auto strLog = gcnew String(log);

		this->logHandler(strLog, static_cast<IntPtr>(kcp), this->user);
	}
}

generic<typename TUser>
void Starts2000::KcpSharp::Kcp<TUser>::Allocator(KcpMallocHandler^ mallocHandler, KcpFreeHandler^ freeHandler)
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

generic<typename TUser>
UInt32 Starts2000::KcpSharp::Kcp<TUser>::GetConv(IntPtr dataPtr)
{
	return ikcp_getconv(dataPtr.ToPointer());
}

generic<typename TUser>
UInt32 Starts2000::KcpSharp::Kcp<TUser>::GetConv(array<Byte>^ data)
{
	pin_ptr<void> pData = &data[0];
	return ikcp_getconv(static_cast<const void *>(pData));
}

generic<typename TUser>
void Starts2000::KcpSharp::Kcp<TUser>::CheckKcp()
{
	if (nullptr == this->kcp) {
		throw gcnew KcpException("kcp not initialized or initialize failed!");
	}
}
