// firy-test.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "firy.hpp"
#include "images/d64.hpp"
#include "images/adf.hpp"
#include "images/fat.hpp"

#include <sstream>
#include <iomanip>

bool DumpBlocksFree(std::shared_ptr<firy::access::cBlocks> pDisk, const std::string& pBaseFileName) {

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

	std::shared_ptr<tImage> img = std::make_shared<tImage>(firy::gFiry->openLocalFile(pImage));
	img->filesystemPrepare();
	DumpBlocksFree(img, pTarget + "//" + pBaseTarget);
}

int main()
{
	
	auto aa = firy::gFiry->openImage("Robs_Workbench2.0.adf");


	return 0;
	{
		auto D64 = firy::gFiry->openImageFile<firy::images::cD64>("test.d64");
		auto test = D64->filesystemFile("CREEP");

		auto buffer = test->read();
		auto data = buffer->data();

		auto track = D64->trackRead(18);

		firy::gResources->FileSave("d:\\test", track);

		DumpImageBlocksFree<firy::images::cADF>("Robs_Workbench2.0.adf", "d:\\blocks", "EA");
	}


	{
		auto adf = firy::gFiry->openImageFile<firy::images::cADF>("Robs_Workbench2.0.adf");
		auto path = adf->filesystemPath("/");

		int year, month, days;

		firy::images::adf::adfDays2Date(adf->mRootBlock->days, &year, &month, &days);

		std::cout << "Year: " << year << " Month: " << month << " Day: " << days << "\n";
		std::cout << "Label: " << adf->filesystemNameGet() << "\n";

		for (auto& entry : path->mNodes) {
			std::cout << entry->mName << "\n";
		}

		auto file = adf->filesystemFile("/S/startup-sequence");
		auto data = file->read();
		std::cout << "\n\ncontent of /S/startup-sequence\n\n";
		std::cout << std::string((char*)data->data(), data->size());
	}

	{
		auto FAT = std::make_shared<firy::images::cFAT>(firy::gFiry->openLocalFile("Microsoft.MS-DOS.6.2.Upgrade.1of3.img"));
		FAT->filesystemPrepare();
		auto path = FAT->filesystemPath("/");

		if (path) {
			for (auto& entry : path->mNodes) {
				std::cout << entry->mName << "\n";
			}
		}
		auto file = FAT->filesystemFile("/SCANDISK.EXE");
		auto data = file->read();
	}
	{
		std::shared_ptr<firy::images::cFAT> FAT = std::make_shared<firy::images::cFAT>(firy::gFiry->openLocalFile("EA_Engine_Assy.img"));
		FAT->filesystemPrepare();
		auto path = FAT->filesystemPath("/");

		if (path) {
			for (auto& entry : path->mNodes) {
				std::cout << entry->mName << "\n";
			}
		}
		//DumpImageBlocksFree<firy::images::cFAT>("Prince.of.Persia.2.1of4.img", "d:\\blocks", "prince2");
//		auto file = FAT->filesystemFile("/SIRxPRINCE");
	//	auto data = file->read();
	}

	//DumpImageBlocksFree<firy::images::cFAT>("EA_Engine_Assy.img", "d:\\blocks", "EA");
	//DumpImageBlocksFree<firy::images::cFAT>("Prince.Of.Persia.img", "d:\\blocks", "PRINCE");
//DumpImageBlocksFree<firy::images::cADF>("d:\\blocks\\Robs_Workbench_2.adf", "d:\\blocks", "PRINCE");

	/*auto files = firy::gResources->directoryList(firy::gResources->getcwd(), "img");

	for (auto& file : files) {
		auto name = file.substr(file.find_last_of("\\") + 1);
		name = name.substr(0, name.size() - 4); // remove extension

		DumpImageBlocksFree<firy::images::cFAT>(file, "d:\\blocks", name);
	}*/


	{
		auto FAT = std::make_shared<firy::images::cFAT>(firy::gFiry->openLocalFile("Win98.img"));
		FAT->filesystemPrepare();
		auto path = FAT->filesystemPath("/");

		if (path) {
			for (auto& entry : path->mNodes) {
				std::cout << entry->mName << "\n";
			}
		}
				auto file = FAT->filesystemFile("/scandisk.log");
				auto data = file->read();
	}


	
}
