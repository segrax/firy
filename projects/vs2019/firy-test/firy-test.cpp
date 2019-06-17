// firy-test.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "firy.hpp"
#include "images/d64.hpp"

int main()
{
	std::shared_ptr<firy::images::cD64> D64 = std::make_shared<firy::images::cD64>();

	D64->imageOpen("test.d64");
	D64->filesystemPrepare();
	auto test = D64->filesystemFile("CREEP");

	auto buffer = test->read();
	auto data = buffer->data();
}
