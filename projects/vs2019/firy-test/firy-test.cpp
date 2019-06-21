// firy-test.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "firy.hpp"
#include "images/d64.hpp"
#include "images/adf.hpp"

int main()
{
	/*
	std::shared_ptr<firy::images::cD64> D64 = std::make_shared<firy::images::cD64>();

	D64->imageOpen("test.d64");
	D64->filesystemPrepare();
	auto test = D64->filesystemFile("CREEP");

	auto buffer = test->read();
	auto data = buffer->data();

	auto track = D64->trackRead(18);

	firy::gResources->FileSave("d:\\test", track);*/
	/*{
		std::shared_ptr<firy::images::cADF> ADF = std::make_shared<firy::images::cADF>();
		ADF->imageOpen("Mo_Utes.adf");
		ADF->filesystemPrepare();
		auto path = ADF->filesystemPath("/");
		std::cout << "Label: " << ADF->filesystemNameGet() << "\n";

		for (auto& entry : path->mNodes) {
			std::cout << entry->mName << "\n";
		}

		auto file = ADF->filesystemFile("/S/startup-sequence");
		auto data = file->read();
		std::cout << "\n\ncontent of /S/startup-sequence\n\n";
		std::cout << std::string((char*)data->data(), data->size());
	}*/
	/*
	{
	std::shared_ptr<firy::images::cADF> ADF = std::make_shared<firy::images::cADF>();
	ADF->imageOpen("test.adf");
	ADF->filesystemPrepare();
	auto path = ADF->filesystemPath("/");

	for (auto& entry : path->mNodes) {
		std::cout << entry->mName << "\n";
	}

	auto file = ADF->filesystemFile("/Solution");
	auto data = file->read();
	std::cout << std::string((char*)data->data(), data->size());
	}*/
	
	{
		std::shared_ptr<firy::images::cADF> ADF = std::make_shared<firy::images::cADF>();
		ADF->imageOpen("hardone.hdf");
		ADF->filesystemPrepare();
		auto path = ADF->filesystemPath("/Games/");

		if (path) {
			for (auto& entry : path->mNodes) {
				std::cout << entry->mName << "\n";
			}
		}

		//auto file = ADF->filesystemFile("/Solution");
		//auto data = file->read();
		//std::cout << std::string((char*)data->data(), data->size());
	}
}
