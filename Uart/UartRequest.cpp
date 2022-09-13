#include "UartRequest.h"

UartRequest::UartRequest()
{

}

UartRequest::~UartRequest()
{

}

void UartRequest::requestNewMessage(int id)
{
    sendMessage(id);
}
