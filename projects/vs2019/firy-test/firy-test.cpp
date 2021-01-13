// firy-test.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "firy.hpp"
#include "images/d64.hpp"
#include "images/adf.hpp"
#include "images/fat.hpp"

#include <sstream>
#include <iomanip>

bool DumpBlocksFree(firy::spImage pDisk, const std::string& pBaseFileName) {

	for (auto& block : pDisk->unitGetFree()) {
		auto data = pDisk->unitRead(block);
		if (!data) {

			return false;
		}

		std::stringstream filename;
		filename << pBaseFileName << "_";
		filename << std::setw(4) << block.block();
		filename << ".raw";

		firy::gResources->FileSave(filename.str(), data);
	}

	return true;
}

void DumpImageBlocksFree(const std::string& pImage, const std::string& pTarget, const std::string& pBaseTarget) {

	auto img = firy::gFiry->openImage(pImage);
	img->filesystemLoad();
	DumpBlocksFree(img, pTarget + "//" + pBaseTarget);
}

inline bool ends_with(std::string const& value, std::string const& ending)
{
	if (ending.size() > value.size()) return false;
	return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

bool testImage(firy::spImage pImage) {
	static auto raws = firy::gResources->directoryList(firy::gResources->getcwd() + "/_raw", "");

	if (pImage->filesystemNameGet() != "FIRY TEST DISK" && pImage->filesystemNameGet() != "FIRY TEST D") {
		std::cout << " Disk label fail\n";
	}

	// Loop each file found in the raw folder
	for (auto raw : raws) {
		auto rawcontent = firy::gResources->FileRead(raw);

		auto filenameIndex = raw.find_last_of("\\");
		std::string filename = raw.substr(filenameIndex + 1);

		auto file = pImage->filesystemFile(filename);
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

bool testImageFile(const std::string& pImageFile) {
	auto image = firy::gFiry->openImage(pImageFile);
	if (!image) {
		std::cout << " Failed to detect image type\n";
		return false;;
	}
	return testImage(image);
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

/**
 * Create an image, and inject all raw files
 */
template <class tType> auto createTestImage_InjectRaws() {
	static auto raws = firy::gResources->directoryList(firy::gResources->getcwd() + "/_raw", "");

	auto d64 = firy::gFiry->createImageFile<tType>("");
	d64->filesystemNameSet("FIRY TEST DISK");

	// Loop each file found in the raw folder
	for (auto raw : raws) {

		auto filenameIndex = raw.find_last_of("\\");
		std::string filename = raw.substr(filenameIndex + 1);

		auto file = d64->filesystemFileCreate(filename);
		file->mContent = firy::gResources->FileRead(raw);
		if (!d64->filesystemPath()->add(file)) {
			std::cout << "Failed to add file: " << filename << "\n";
		}
	}

	if (!d64->filesystemLoad()) {
		std::cout << "Failed\n";
	}
	return d64;
}

int main()
{
	//DumpImageBlocksFree("mine/Robs_Workbench2.0.adf", "d:\\blocks", "EA");
	/*
	auto D64 = firy::gFiry->openImageFile<firy::images::cADF>("D:\\Projects\\directory_opus_firy_plugin\\library\\firy\\test\\images\\amiga\\testdisk.adf");
	testImage(D64);
	auto newImageD = createTestImage_InjectRaws<firy::images::cD64>();
	testImage(newImageD);

	auto newImageA = createTestImage_InjectRaws<firy::images::cADF>();
	testImage(newImageA);
	*/
	if (!testImages())
		return 1;

	return 0;
	/*
	auto path = D64->filesystemPath();

	//auto a = std::make_shared<firy::images::d64::sFile>(D64->weak_from_this(), "TEST");
	auto file = D64->filesystemFileCreate("otherfile");
	file->mContent = firy::gResources->FileRead("otherfile");

	path->add(file);

	D64->filesystemLoad();

	D64->sourceSave();

	auto file2 = D64->filesystemFile("/otherfile");
	auto aa = file2->read();

	if (path) {
		for (auto& entry : path->mNodes) {
			std::cout << entry->nameGet() << "\n";
		}
	}


	// TODO:

	return 0;

	auto image = firy::gFiry->openImage("mine/Win98.img");
	path = image->filesystemPath();

	if (path) {
		for (auto& entry : path->mNodes) {
			std::cout << entry->nameGet() << "\n";
		}
	}

	{
		auto D64 = firy::gFiry->openImageFile<firy::images::cD64>("mine/test.d64");
		auto test = D64->filesystemFile("CREEP");

		auto buffer = test->read();
		auto data = buffer->data();

		auto track = D64->trackRead(18);

		//firy::gResources->FileSave("d:\\test", track);

		//DumpImageBlocksFree("Robs_Workbench2.0.adf", "d:\\blocks", "EA");
	}


	{
		auto adf = firy::gFiry->openImageFile<firy::images::cADF>("mine/Robs_Workbench2.0.adf");
		auto path = adf->filesystemPath("/");

		//int year, month, days;

		//firy::images::adf::convertDaysToDate(adf->mRootBlock->days, &year, &month, &days);

		//std::cout << "Year: " << year << " Month: " << month << " Day: " << days << "\n";
		//std::cout << "Label: " << adf->filesystemNameGet() << "\n";

		for (auto& entry : path->mNodes) {
			std::cout << entry->nameGet() << "\n";
		}

		auto file = adf->filesystemFile("/S/startup-sequence");
		auto data = file->read();
		std::cout << "\n\ncontent of /S/startup-sequence\n\n";
		std::cout << std::string((char*)data->data(), data->size());
	}

	{
		auto FAT = std::make_shared<firy::images::cFAT>(firy::gFiry->openLocalFile("mine/Microsoft.MS-DOS.6.2.Upgrade.1of3.img"));
		FAT->filesystemLoad();
		auto path = FAT->filesystemPath("/");

		if (path) {
			for (auto& entry : path->mNodes) {
				std::cout << entry->nameGet() << "\n";
			}
		}
		auto file = FAT->filesystemFile("/SCANDISK.EXE");
		auto data = file->read();
	}
	{
		std::shared_ptr<firy::images::cFAT> FAT = std::make_shared<firy::images::cFAT>(firy::gFiry->openLocalFile("mine/EA_Engine_Assy.img"));
		FAT->filesystemLoad();
		auto path = FAT->filesystemPath("/");

		if (path) {
			for (auto& entry : path->mNodes) {
				std::cout << entry->nameGet() << "\n";
			}
		}
		//DumpImageBlocksFree("Prince.of.Persia.2.1of4.img", "d:\\blocks", "prince2");
//		auto file = FAT->filesystemFile("/SIRxPRINCE");
	//	auto data = file->read();
	}

	//DumpImageBlocksFree("EA_Engine_Assy.img", "d:\\blocks", "EA");
	//DumpImageBlocksFree("Prince.Of.Persia.img", "d:\\blocks", "PRINCE");
//DumpImageBlocksFree("d:\\blocks\\Robs_Workbench_2.adf", "d:\\blocks", "PRINCE");

	/*auto files = firy::gResources->directoryList(firy::gResources->getcwd(), "img");

	for (auto& file : files) {
		auto name = file.substr(file.find_last_of("\\") + 1);
		name = name.substr(0, name.size() - 4); // remove extension

		DumpImageBlocksFree(file, "d:\\blocks", name);
	}


	{
		auto FAT = firy::gFiry->openImageFile<firy::images::cFAT>("mine/Win98.img");
		auto path = FAT->filesystemPath("/");

		if (path) {
			for (auto& entry : path->mNodes) {
				std::cout << entry->nameGet() << "\n";
			}
		}
				auto file = FAT->filesystemFile("/scandisk.log");
				auto data = file->read();
	}
	*/

	
}
