#include "KcpException.h"

Starts2000::KcpSharp::KcpException::KcpException() : Exception()
{
}

Starts2000::KcpSharp::KcpException::KcpException(String ^ message) : Exception(message)
{
}

Starts2000::KcpSharp::KcpException::KcpException(
	String ^ message, Exception ^ innerException) : Exception(message, innerException)
{
}

Starts2000::KcpSharp::KcpException::KcpException(
	SerializationInfo ^ info, StreamingContext context) : Exception(info, context)
{
}
