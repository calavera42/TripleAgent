#pragma once

#include <queue>

#include "../include/trplagnt.h"

class RequestManager
{
private:
	std::queue<IRequest&> _requestQueue = {};

public:

	void Add(IRequest& r) { _requestQueue.push(r); };

	IRequest& GetNext() 
	{
		IRequest& req = _requestQueue.front();
		_requestQueue.pop();

		return req;
	}
};

