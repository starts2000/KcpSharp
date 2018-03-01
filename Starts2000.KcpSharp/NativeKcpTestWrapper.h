#pragma once

#include "NativeKcpTest.h"

namespace Starts2000 {
	namespace KcpSharp {
		public ref class NativeKcpTestWrapper
		{
		public:
			NativeKcpTestWrapper() : NativeKcpTestWrapper(10, 60, 125, 1000) {

			}

			NativeKcpTestWrapper(int lostrate, int rttmi, int rttmax, int nmax) {
				test = new NativeKcpTest(lostrate, rttmi, rttmax, nmax);
			}

			void Test(int mode) {
				test->Test(mode);
			}

		protected:
			~NativeKcpTestWrapper() {

			}

			!NativeKcpTestWrapper() {
				if (this->test) {
					delete this->test;
				}
			}

		private:
			NativeKcpTest* test;
		};
	}
}