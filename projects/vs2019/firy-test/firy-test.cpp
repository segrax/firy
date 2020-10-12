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

inline bool ends_with(std::string const& value, std::string const& ending)
{
	if (ending.size() > value.size()) return false;
	return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

bool testImageFile(const std::string pImageFile) {
	static auto raws = firy::gResources->directoryList(firy::gResources->getcwd() + "/_raw", "");

	auto image = firy::gFiry->openImage(pImageFile);
	if (!image) {
		std::cout << " Failed to detect image type\n";
		return false;;
	}

	if (image->filesystemNameGet() != "FIRY TEST DISK" && image->filesystemNameGet() != "FIRY TEST D") {
		std::cout << " Disk label fail\n";
	}

	// Loop each file found in the raw folder
	for (auto raw : raws) {
		auto rawcontent = firy::gResources->FileRead(raw);

		auto filenameIndex = raw.find_last_of("\\");
		std::string filename = raw.substr(filenameIndex + 1);

		auto file = image->filesystemFile(filename);
		if (!file) {
			std::cout << " FAILED: file: " << filename << " not found\n";
			return false;
		}

		auto content = file->read();
		
		// Compare content
		if (*content != *rawcontent) {
			std::cout << " FAILED: file: " << filename << "\n";
		}
	}
	return true;
}

bool testImages() {
	auto result = true;
	auto dirs = firy::gResources->directoryList(firy::gResources->getcwd() + "/images", "");
	
	std::cout << "Testing images\n";
	for (auto dir : dirs) {
		if (firy::gResources->isFile(dir)) {

			// Now we look for every file in raw
			std::cout << " " << dir << "\n";

			if(!testImageFile(dir))
				result = false;
		}
	}
	if(result)
		std::cout << "All ok\n";

	return result;
}

int main()
{
	//testImages();

	auto D64 = firy::gFiry->openImageFile<firy::images::cD64>("test_35tracks.d64");
	
	return 0;

	auto image = firy::gFiry->openImage("mine/Win98.img");
	auto path = image->filesystemPath("/");

	if (path) {
		for (auto& entry : path->mNodes) {
			std::cout << entry->mName << "\n";
		}
	}

	{
		auto D64 = firy::gFiry->openImageFile<firy::images::cD64>("mine/test.d64");
		auto test = D64->filesystemFile("CREEP");

		auto buffer = test->read();
		auto data = buffer->data();

		auto track = D64->trackRead(18);

		//firy::gResources->FileSave("d:\\test", track);

		//DumpImageBlocksFree<firy::images::cADF>("Robs_Workbench2.0.adf", "d:\\blocks", "EA");
	}


	{
		auto adf = firy::gFiry->openImageFile<firy::images::cADF>("mine/Robs_Workbench2.0.adf");
		auto path = adf->filesystemPath("/");

		int year, month, days;

		firy::images::adf::convertDaysToDate(adf->mRootBlock->days, &year, &month, &days);

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
		auto FAT = std::make_shared<firy::images::cFAT>(firy::gFiry->openLocalFile("mine/Microsoft.MS-DOS.6.2.Upgrade.1of3.img"));
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
		std::shared_ptr<firy::images::cFAT> FAT = std::make_shared<firy::images::cFAT>(firy::gFiry->openLocalFile("mine/EA_Engine_Assy.img"));
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
		auto FAT = std::make_shared<firy::images::cFAT>(firy::gFiry->openLocalFile("mine/Win98.img"));
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
