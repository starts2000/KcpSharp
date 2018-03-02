#pragma once

#include "test.h"

using namespace System;

namespace Starts2000 {
	namespace KcpSharp {
		public ref class KcpTest
		{
		public:
			KcpTest();
			KcpTest(int lostrate, int rttmi, int rttmax, int nmax);

			void Test(int mode);

		protected:
			~KcpTest();
			!KcpTest();

		private:
			LatencySimulator * vnet;
			int lostrate;
			int rttmi;
			int rttmax;
			int nmax;

			int UdpOutput(IntPtr buf, int len, IntPtr kcp, IntPtr user);
		};
	}
}