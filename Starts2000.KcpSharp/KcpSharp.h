#pragma once

#include "ikcp.h"

using namespace System;
using namespace System::Runtime::InteropServices;

namespace Starts2000 {
	namespace KcpSharp {
		[FlagsAttribute]
		public enum class KcpLogMask
		{
			Output = IKCP_LOG_OUTPUT,
			Input = IKCP_LOG_INPUT,
			Send = IKCP_LOG_SEND,
			Recv = IKCP_LOG_RECV,
			InData = IKCP_LOG_IN_DATA,
			InAck = IKCP_LOG_IN_ACK,
			InProbe = IKCP_LOG_IN_PROBE,
			InWins = IKCP_LOG_IN_WINS,
			OutData = IKCP_LOG_OUT_DATA,
			OutAck = IKCP_LOG_OUT_ACK,
			OutProbe = IKCP_LOG_OUT_PROBE,
			OutWins = IKCP_LOG_OUT_WINS,
			All = Output | Input | Send | Recv |
			InData | InAck | InProbe | InWins |
			OutData | OutAck | OutProbe | OutWins
		};

		/// <summary>
		/// KCP output handler.
		/// </summary>
		/// <typeparam name="TUser">user token type.</typeparam>
		/// <param name="buf">output buffer.</param>
		/// <param name="len">output buffer length.</param>
		/// <param name="kcp">KCP handle.</param>
		/// <param name="user">user token, type is <typeparamref name="TUser"/></param>
		/// <returns></returns>
		generic<typename TUser>
		public delegate int KcpOutputHandler(array<Byte>^ buf, int len, IntPtr kcp, TUser user);

		[UnmanagedFunctionPointerAttribute(CallingConvention::Cdecl)]
		public delegate IntPtr KcpMallocHandler(UInt32 size);

		[UnmanagedFunctionPointerAttribute(CallingConvention::Cdecl)]
		public delegate void KcpFreeHandler(IntPtr buffferPtr);

		/// <summary>
		/// KCP log handler.
		/// </summary>
		/// <typeparam name="TUser">user token type.</typeparam>
		/// <param name="log">log info string.</param>
		/// <param name="kcp">KCP handle.</param>
		/// <param name="user">user token, type is <typeparamref name="TUser"/></param>
		generic<typename TUser>
		public delegate void KcpLogHandler(String^ log, IntPtr kcp, TUser user);

		delegate int InnerOutputHandler(const char *buf, int len, ikcpcb* kcp, void* user);
		delegate void InnerLogHandler(const char *log, ikcpcb *kcp, void *user);

		/// <summary>
		/// KCP dotnet wrapper.
		/// KCP is A Fast and Reliable ARQ Protocol, refer to https://github.com/skywind3000/kcp.
		/// </summary>
		/// <typeparam name="TUser">KCP user token type.</typeparam>
		generic<typename TUser>
		public ref class Kcp
		{
		public:
			/// <summary>
			/// create a new kcp control object.
			/// </summary>
			/// <param name="conv">must equal in two endpoint from the same connection.</param>
			/// <param name="user">user token, will be passed to the output callback.</param>
			Kcp(UInt32 conv, TUser user);

			/// <summary>
			/// set output callback, which will be invoked by kcp.
			/// </summary>
			/// <param name="outputHandler">output handler.</param>
			void SetOutput(KcpOutputHandler<TUser>^ outputHandler);

			/// <summary>
			/// user/upper level recv.
			/// </summary>
			/// <param name="buffer">receive buffer</param>
			/// <param name="len">receive buffer size.</param>
			/// <returns>size, below zero for EAGAIN.</returns>
			int Recv(array<Byte>^ buffer, int len);

			/// <summary>
			/// user/upper level send.
			/// </summary>
			/// <param name="buffer">send buffer</param>
			/// <param name="len">send buffer size.</param>
			/// <returns>size, below zero for error.</returns>
			int Send(array<Byte>^ buffer, int len);

			/// <summary>
			/// update state(call it repeatedly, every 10ms - 100ms), or you can ask
			/// <see cref="Check(UInt32)"/> when to call it again(without <see cref="Input(array&lt;Byte&gt;^, int)"/> / <see cref="Send(array&lt;Byte&gt;^, int)"/> calling).
			/// </summary>
			/// <param name="current">current timestamp in millisec.</param>
			void Update(UInt32 current);

			/// <summary>
			/// Determine when should you invoke <see cref="Update(UInt32)"/>:
			/// returns when you should invoke ikcp_update in millisec, if there 
			/// is no <see cref="Input(array&lt;Byte&gt;^, int)"/> / <see cref="Send(array&lt;Byte&gt;^, int)"/> calling.
			/// you can call ikcp_update in that time, instead of call update repeatly.
			/// Important to reduce unnacessary ikcp_update invoking. use it to 
			/// schedule <see cref="Update(UInt32)"/> (eg. implementing an epoll-like mechanism, 
			/// or optimize <see cref="Update(UInt32)"/> when handling massive kcp connections)
			/// </summary>
			/// <param name="current">current timestamp in millisec.</param>
			/// <returns>should invoke <see cref="Update(UInt32)"/> in millisec.</returns>
			UInt32 Check(UInt32 current);

			/// <summary>
			/// when you received a low level packet(eg.UDP packet), call it.
			/// </summary>
			/// <param name="data">a low level data packet.</param>
			/// <param name="size">data size.</param>
			/// <returns></returns>
			int Input(array<Byte>^ data, int size);

			/// <summary>
			/// flush pending data.
			/// </summary>
			void Flush();

			/// <summary>
			/// check the size of next message in the recv queue.
			/// </summary>
			/// <returns>message size.</returns>
			int PeekSize();

			/// <summary>
			/// change MTU size, default is 1400.
			/// </summary>
			/// <returns></returns>
			int SetMtu(int mtu);

			/// <summary>
			/// set maximum window size: sndwnd=32, rcvwnd=32 by default.
			/// </summary>
			/// <param name="sndwnd">send window size.</param>
			/// <param name="rcvwnd">receive window size.</param>
			/// <returns></returns>
			int SetWndSize(int sndwnd, int rcvwnd);

			/// <summary>
			/// get how many packet is waiting to be sent.
			/// </summary>
			/// <returns></returns>
			int GetWaitSnd();

			/// <summary>
			/// fastest: Nodelay(1, 20, 2, 1).
			/// </summary>
			/// <param name="nodelay">false:disable(default), true:enable.</param>
			/// <param name="interval">internal update timer interval in millisec, default is 100ms.</param>
			/// <param name="resend"> 0:disable fast resend(default), 1:enable fast resend.</param>
			/// <param name="nc">false:normal congestion control(default), true:disable congestion control.</param>
			/// <returns></returns>
			int Nodelay(bool nodelay, int interval, int resend, bool nc);

			/// <summary>
			/// set log mask.
			/// </summary>
			/// <param name="logMask"><see cref="KcpLogMask" /></param>
			void SetLogMask(KcpLogMask logMask);

			/// <summary>
			/// set log.
			/// </summary>
			/// <param name="loghandler">log handler.</param>
			void SetLog(KcpLogHandler<TUser>^ loghandler);

			/// <summary>
			/// setup allocator.
			/// </summary>
			/// <param name="mallocHandler"><see cref="KcpMallocHandler" /></param>
			/// <param name="freeHandler"><see cref="KcpFreeHandler" /></param>
			static void Allocator(KcpMallocHandler^ mallocHandler, KcpFreeHandler^ freeHandler);

			/// <summary>
			/// read conv.
			/// </summary>
			/// <param name="dataPtr"><see cref="IntPtr" /></param>
			static UInt32 GetConv(IntPtr dataPtr);

			/// <summary>
			/// read conv.
			/// </summary>
			/// <param name="data"></param>
			static UInt32 GetConv(array<Byte>^ data);

			/// <summary>
			/// get KCP user token.
			/// </summary>
			property TUser User {
				TUser get() {
					return this->user;
				}
			}

		protected:
			~Kcp();
			!Kcp();

		internal:
			property ikcpcb * IKcp {
				ikcpcb * get() {
					return kcp;
				}
			}

		private:
			ikcpcb * kcp;
			TUser user;

			GCHandle userGCHandle;
			GCHandle outputHandlerGcHandle;
			GCHandle logHandlerGcHandle;

			KcpOutputHandler<TUser> ^ outputHandler;
			KcpLogHandler<TUser> ^ logHandler;

			static GCHandle mallocHandlerGCHandle;
			static GCHandle freeHandlerGCHandle;

			void CheckKcp();
			int Output(const char * buf, int len, ikcpcb* kcp, void* user);
			void Log(const char *log, ikcpcb* kcp, void *user);
		};
	}
}