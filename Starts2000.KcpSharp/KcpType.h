#pragma once

#include "ikcp.h"

using namespace System;
using namespace System::Runtime::InteropServices;

namespace Starts2000 {
	namespace KcpSharp {
		[Flags]
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

		generic<class TUser>
		ref class Kcp;

		/// <summary>
		/// KCP output handler.
		/// </summary>
		/// <typeparam name="TUser">the type of KCP user token.</typeparam>
		/// <param name="buf">output buffer.</param>
		/// <param name="len">output buffer length.</param>
		/// <param name="kcp">kcp object.</param>
		/// <param name="user">user token.</param>
		/// <returns></returns>
		generic<class TUser>
		public delegate int KcpOutputHandler(Kcp<TUser>^ kcp, array<Byte>^ buffer, int len);

		[UnmanagedFunctionPointer(CallingConvention::Cdecl)]
		public delegate IntPtr KcpMallocHandler(UInt32 size);

		[UnmanagedFunctionPointer(CallingConvention::Cdecl)]
		public delegate void KcpFreeHandler(IntPtr buffferPtr);

		/// <summary>
		/// KCP log handler.
		/// </summary>
		/// <typeparam name="TUser">the type of KCP user token.</typeparam>
		/// <param name="log">log info string.</param>
		/// <param name="kcp">kcp object.</param>
		generic<class TUser>
		public delegate void KcpLogHandler(Kcp<TUser>^ kcp, String^ log);

		[UnmanagedFunctionPointer(CallingConvention::Cdecl)]
		delegate int InnerKcpOutputHandler(const char * buffer, int len, ikcpcb * kcp, void * user);

		[UnmanagedFunctionPointer(CallingConvention::Cdecl)]
		delegate void InnerKcpLogHandler([MarshalAs(UnmanagedType::LPStr)]String^ log, ikcpcb * kcp, void * user);
	}
}