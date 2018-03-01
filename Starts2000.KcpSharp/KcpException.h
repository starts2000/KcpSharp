#pragma once

using namespace System;
using namespace System::Runtime::Serialization;

namespace Starts2000 {
	namespace KcpSharp {
		public ref class KcpException : public Exception
		{
		public:
			KcpException();
			KcpException(String^ message);
			KcpException(String^ message, Exception^ innerException);

		protected:
			KcpException(SerializationInfo^ info, StreamingContext context);
		};
	}
}