// firy-test.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "firy.hpp"
#include "images/d64.hpp"
#include "images/adf.hpp"
#include "images/fat.hpp"

#include <sstream>
#include <iomanip>

bool DumpBlocksFree(std::shared_ptr<firy::interfaces::cBlocks> pDisk, const std::string& pBaseFileName) {

	for (auto& block : pDisk->blocksFree()) {
		auto data = pDisk->blockRead(block);
		if (!data) {

			return false;
		}
		bool found = false;

		for (auto bb : *data) {
			if (bb) {
				if(bb != 0xF6)
					found = true;
				break;
			}
		}

		if (found) {
			std::stringstream filename;
			filename << pBaseFileName << "_";
			filename << std::setw(4) << block;
			filename << ".raw";

			firy::gResources->FileSave(filename.str(), data);
		}
	}

	return true;
}

template <class tImage> void DumpImageBlocksFree(const std::string& pImage, const std::string& pTarget, const std::string& pBaseTarget) {

	std::shared_ptr<tImage> img = std::make_shared<tImage>();
	img->imageOpen(pImage);
	img->filesystemPrepare();
	DumpBlocksFree(img, pTarget + "//" + pBaseTarget);
}

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
	/*
	DumpImageBlocksFree<firy::images::cADF>("Robs_Workbench2.0.adf", "d:\\blocks", "EA");


	{
		std::shared_ptr<firy::images::cADF> ADF = std::make_shared<firy::images::cADF>();
		ADF->imageOpen("Robs_Workbench2.0.adf");
		ADF->filesystemPrepare();
		auto path = ADF->filesystemPath("/");

		int year, month, days;

		firy::images::adf::adfDays2Date(ADF->mRootBlock->days, &year, &month, &days);

		std::cout << "Year: " << year << " Month: " << month << " Day: " << days << "\n";
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
	/*
	auto files = firy::gResources->directoryList(firy::gResources->getcwd(), "adf");

	for (auto& file : files) {
		std::shared_ptr<firy::images::cADF> ADF = std::make_shared<firy::images::cADF>();
		ADF->imageOpen(file);
		if (!ADF->filesystemPrepare()) {
			std::cout << file << " failed\n";
			continue;
		}

		auto name = file.substr(file.find_last_of("\\") + 1);
		name = name.substr(0, name.size() - 4); // remove extension

		dumpfreeblocks(ADF, "d://blocks//" + name);
	}*/

	/*{
		std::shared_ptr<firy::images::cADF> ADF = std::make_shared<firy::images::cADF>();
		ADF->imageOpen("Mo_Utes.adf");
		//ADF->imageOpen("hardone.hdf");
		ADF->filesystemPrepare();

		dumpfreeblocks(ADF, "d://track");

		auto path = ADF->filesystemPath("/Games/");

		if (path) {
			for (auto& entry : path->mNodes) {
				std::cout << entry->mName << "\n";
			}
		}

		//auto file = ADF->filesystemFile("/Solution");
		//auto data = file->read();
		//std::cout << std::string((char*)data->data(), data->size());
	}*/
	/*
	{
		std::shared_ptr<firy::images::cFAT> FAT = std::make_shared<firy::images::cFAT>();
		FAT->imageOpen("Prince.of.Persia.2.1of4.img");
		FAT->filesystemPrepare();
		auto path = FAT->filesystemPath("/");

		if (path) {
			for (auto& entry : path->mNodes) {
				std::cout << entry->mName << "\n";
			}
		}
		DumpImageBlocksFree<firy::images::cFAT>("Prince.of.Persia.2.1of4.img", "d:\\blocks", "prince2");
//		auto file = FAT->filesystemFile("/SIRxPRINCE");
	//	auto data = file->read();
	}*/

	//DumpImageBlocksFree<firy::images::cFAT>("EA_Engine_Assy.img", "d:\\blocks", "EA");
	//DumpImageBlocksFree<firy::images::cFAT>("Prince.Of.Persia.img", "d:\\blocks", "PRINCE");

	/*auto files = firy::gResources->directoryList(firy::gResources->getcwd(), "img");

	for (auto& file : files) {
		auto name = file.substr(file.find_last_of("\\") + 1);
		name = name.substr(0, name.size() - 4); // remove extension

		DumpImageBlocksFree<firy::images::cFAT>(file, "d:\\blocks", name);
	}*/


	{
		std::shared_ptr<firy::images::cFAT> FAT = std::make_shared<firy::images::cFAT>();
		FAT->imageOpen("Win95.img");
		FAT->filesystemPrepare();
		auto path = FAT->filesystemPath("/");

		if (path) {
			for (auto& entry : path->mNodes) {
				std::cout << entry->mName << "\n";
			}
		}
		//		auto file = FAT->filesystemFile("/SIRxPRINCE");
			//	auto data = file->read();
	}


	
}
