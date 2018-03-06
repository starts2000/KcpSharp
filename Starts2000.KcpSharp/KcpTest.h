#pragma once

#include "test.h"
#include "KcpOfTUser.h"

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

			int UdpOutput(Kcp<int>^ kcp, array<Byte>^ buf, int len);
		};
	}
}