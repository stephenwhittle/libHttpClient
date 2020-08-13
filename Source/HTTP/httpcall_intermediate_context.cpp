#include "pch.h"
#include "httpcall.h"


STDAPI HCIntermediateContextGetData(
	_In_ HCIntermediateCompletionContext* context,
	_Out_ uint8_t** data,
	_Out_ uint32_t* length
) noexcept
try
{
	if (context == nullptr || length == nullptr)
	{
		return E_INVALIDARG;
	}
	else
	{
		if (data != nullptr)
		{
			*data = static_cast<uint8_t*>(context->m_data.get());
		}

		*length = context->m_dataSize;
	}
	return S_OK;
}
CATCH_RETURN()


STDAPI HCIntermediateContextGetOffset(
	_In_ HCIntermediateCompletionContext* context,
	_Out_ uint64_t* offset
) noexcept
try
{
	if (context == nullptr || offset == nullptr)
	{
		return E_INVALIDARG;
	}
	else
	{
		*offset = context->m_offset;
	}
	return S_OK;
}
CATCH_RETURN()


STDAPI HCIntermediateContextGetCall(
	_In_ HCIntermediateCompletionContext* context,
	_Out_ HCCallHandle* call
) noexcept
try
{
	if (context == nullptr)
	{
		return E_INVALIDARG;
	}
	else
	{
		call = &context->m_call;
	}
	return S_OK;
}
CATCH_RETURN()

STDAPI HCIntermediateContextCloseContext(
	_In_ HCIntermediateCompletionContext* context
) noexcept
try
{
	if (context == nullptr)
	{
		return E_INVALIDARG;
	}
	else
	{
		xbox::httpclient::Delete(context);
	}
	return S_OK;
}
CATCH_RETURN()