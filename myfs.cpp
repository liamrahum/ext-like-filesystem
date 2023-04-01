#include "myfs.h"
#include <cstddef>
#include <string.h>
#include <iostream>
#include <math.h>
#include <sstream>
#include <fstream>
#include <string>



//using std::cout;
//using std::endl;
//using std::cin;
using std::string;

const char *MyFs::MYFS_MAGIC = "MYFS";

MyFs::MyFs(BlockDeviceSimulator *blkdevsim_):blkdevsim(blkdevsim_) {
	struct myfs_header header;
	blkdevsim->read(0, sizeof(header), (char *)&header);

	if (strncmp(header.magic, MYFS_MAGIC, sizeof(header.magic)) != 0 ||
	    (header.version != CURR_VERSION)) {
		std::cout << "Did not find myfs instance on blkdev" << std::endl;
		std::cout << "Creating..." << std::endl;
		format();
		std::cout << "Finished!" << std::endl;
	}
	//blkdevsim->sizeofHeader = sizeof(header);
}

void MyFs::format() {

	// put the header in place
	struct myfs_header header;
	strncpy(header.magic, MYFS_MAGIC, sizeof(header.magic));
	header.version = CURR_VERSION;
	//blkdevsim->sizeofHeader = sizeof(header);
	blkdevsim->write(0, sizeof(header), (const char*)&header);
	
	const char table[INODESTABLELEN] = {0};
	blkdevsim->write(0, INODESTABLELEN, table); //Inodes
	blkdevsim->write(INODESTABLELEN, INODESTABLELEN, table); //Sizes


}

std::map<std::string, int> MyFs::getInodes(int addr)
{
	std::map<std::string, int> res;
	char table[INODESTABLELEN] = {0};
	
	blkdevsim->read(addr, INODESTABLELEN, table);
	std::stringstream tableString(table);
	string currentLine;
	
	std::stringstream innerLine;
	string property;
	bool isFirst = true;
	string fName;
	while(std::getline(tableString, currentLine, '~'))
	{	
		innerLine = std::stringstream(currentLine);
		while(std::getline(innerLine, property, '|'))
		{
			isFirst = false;
			if(isFirst)
			{
				fName = property;
				continue;
			}
			res[fName]= std::atoi(property.c_str());
		}
		isFirst = true;
	};
	return res;
}

void MyFs::setInodes(std::map<std::string, int> inodes, int addr)
{
	int current = addr;
	for(auto it = inodes.begin(); it != inodes.end(); it++)
	{
		blkdevsim->write(current, it->first.length() + 1, (it->first + "~").c_str());
		current += it->first.length() + 1;
		blkdevsim->write(current, std::to_string(it->second).length() + 1, (std::to_string(it->second) + "|").c_str());
		current += std::to_string(it->second).length() + 1;
	}
}

void MyFs::create_file(std::string path_str, bool directory) {

	if(directory)
		throw std::runtime_error("not implemented");
	int current = 0;
	for(auto it = blkdevsim->inodes.begin(); it != blkdevsim->inodes.end(); it++)
	{
		if(it->second > current)
			current = it->second + 1;
	}
	blkdevsim->inodes[path_str] = current;
	blkdevsim->sizes[path_str] = 0;
	setInodes(blkdevsim->inodes, 0);
	setInodes(blkdevsim->sizes, INODESTABLELEN);

}

std::string MyFs::get_content(std::string path_str) {
	//throw std::runtime_error("not implemented");
	char ans[256] = {0};
	blkdevsim->read(2*INODESTABLELEN + blkdevsim->inodes[path_str],blkdevsim->sizes[path_str], ans);
	
	return std::string(ans);	
}

void MyFs::set_content(std::string path_str, std::string content) {
	//throw std::runtime_error("not implemented");

	int currentSize = blkdevsim->sizes[path_str];
	int difference = content.length() - currentSize;
	//std::cout << difference << std::endl;
	blkdevsim->sizes[path_str] = content.length();
	int currentInode = blkdevsim->inodes[path_str];
	for(auto it = blkdevsim->inodes.begin(); it != blkdevsim->inodes.end(); it++)
		if(it->second > currentInode)
		{
			//std::cout << get_content(it->first) << std::endl;
			blkdevsim->write(2*INODESTABLELEN + it->second + difference, get_content(it->first).length(), get_content(it->first).c_str());
			it->second += difference;
		}

	blkdevsim->write(2*INODESTABLELEN+ blkdevsim->inodes[path_str], content.length(), content.c_str());

	setInodes(blkdevsim->inodes, 0);
	setInodes(blkdevsim->sizes, INODESTABLELEN);	

}

MyFs::dir_list MyFs::list_dir(std::string path_str) {
	dir_list ans;
	string name;
	int size;
	for(auto it = blkdevsim->inodes.begin(); it != blkdevsim->inodes.end(); it++)
	{
		name = it->first;
		size = blkdevsim->sizes[it->first];
		//size = it->second; //(Debug - get inode instead)
		ans.push_back({name, false, size});
	}
	
	return ans;
}