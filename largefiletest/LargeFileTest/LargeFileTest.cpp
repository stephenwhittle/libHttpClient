// LargeFileTest.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <fstream>
#include "httpClient/httpClient.h"
int main()
{
    HCInitialize(nullptr);
	HCTraceSetTraceToDebugger(true);
    HCCallHandle Call;
    HCHttpCallCreate(&Call);
    HCHttpCallRequestSetUrl(Call, "GET", "https://test.mod.io/mods/file/894/457d82275843860f6489ad8ed7325f06");
	XAsyncBlock* AsyncBlock = new XAsyncBlock();
	ZeroMemory(AsyncBlock, sizeof(XAsyncBlock));

	AsyncBlock->context = nullptr;
	AsyncBlock->queue = nullptr;
	AsyncBlock->callback = [](XAsyncBlock* InAsyncBlock)
	{
		delete(InAsyncBlock);
	};

	

	auto DataCallback =   [](void* context, bool canceled)
	{
		HCIntermediateCompletionContext* ActualContext = static_cast<HCIntermediateCompletionContext*>(context);
		if (canceled) 
		{
			HCIntermediateContextCloseContext(ActualContext);
			return; 
		}
		std::ofstream FileStream = std::ofstream("testfile.zip", std::ios::binary | std::ios::out);
		std::uint64_t Offset;
		if (SUCCEEDED(HCIntermediateContextGetOffset(ActualContext, &Offset)))
		{
			uint32_t DataSize;
			uint8_t* DataPtr = nullptr;
			if (SUCCEEDED(HCIntermediateContextGetData(ActualContext, &DataPtr, &DataSize)))
			{
				std::cout << "Data Received :" << DataSize << " : " << Offset << std::endl;
				/*for (int i = 0; i < DataSize; i++)
				{
					std::cout << *(DataPtr + i);
				}*/
			}
		}


		HCIntermediateContextCloseContext(ActualContext);
	};
	HCHttpCallRequestSetDataCallback(Call, DataCallback );
	HCHttpCallPerformAsync(Call, AsyncBlock);
	HRESULT Status = XAsyncGetStatus(AsyncBlock, false);
	while (true)
	{
		Sleep(10);
	}
}


