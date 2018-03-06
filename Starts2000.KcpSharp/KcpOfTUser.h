#pragma once

#include "KcpType.h"

namespace Starts2000 {
	namespace KcpSharp {
		/// <summary>
		/// KCP dotnet wrapper.
		/// KCP is A Fast and Reliable ARQ Protocol, refer to https://github.com/skywind3000/kcp.
		/// </summary>
		/// <typeparam name="TUser">the type of KCP user token.</typeparam>
		generic<class TUser>
		public ref class Kcp
		{
		public:
			/// <summary>
			/// create a new kcp control object.
			/// </summary>
			/// <param name="conv">must equal in two endpoint from the same connection.</param>
			/// <param name="user">user token <typeparamref name="TUser" />, will be passed to the output callback.</param>
			Kcp(UInt32 conv, TUser user);

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

			/// <summary>
			/// get or set KCP MTU size, default is 1400.
			/// </summary>
			property int Mtu {
				int get() {
					CheckKcp();
					return this->kcp->mtu;
				}
				void set(int value) {
					CheckKcp();
					ikcp_setmtu(this->kcp, value);
				}
			}

			/// <summary>
			/// set output callback, which will be invoked by kcp.
			/// </summary>
			property KcpOutputHandler<TUser>^ OutputHandler {
				void set(KcpOutputHandler<TUser>^ outputHandler) {
					this->SetOutput(outputHandler);
				}
			}

			/// <summary>
			/// get or set KCP log mask.
			/// </summary>
			property KcpLogMask LogMask {
				KcpLogMask get() {
					CheckKcp();
					return safe_cast<KcpLogMask>(this->kcp->logmask);
				}
				void set(KcpLogMask logMask) {
					CheckKcp();
					kcp->logmask = safe_cast<int>(logMask);
				}
			}

			/// <summary>
			/// set log callback, which will be invoked by kcp.
			/// </summary>
			property KcpLogHandler<TUser>^ LogHandler {
				void set(KcpLogHandler<TUser>^ logHandler) {
					this->SetLog(logHandler);
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
			KcpOutputHandler<TUser>^ outputHandler;
			KcpLogHandler<TUser> ^ logHandler;

			GCHandle outputHandlerGcHandle;
			GCHandle logHandlerGcHandle;

			static GCHandle mallocHandlerGCHandle;
			static GCHandle freeHandlerGCHandle;

			void CheckKcp();

			/// <summary>
			/// set log.
			/// </summary>
			/// <param name="loghandler">log handler.</param>
			void SetLog(KcpLogHandler<TUser>^ loghandler);

			/// <summary>
			/// set output callback, which will be invoked by kcp.
			/// </summary>
			/// <param name="outputHandler">output handler.</param>
			void SetOutput(KcpOutputHandler<TUser>^ outputHandler);

			int Output(const char * buffer, int len, ikcpcb * kcp, void * user);
			void Log(String^ log, ikcpcb * kcp, void * user);
		};
	}
}