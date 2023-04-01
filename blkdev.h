#ifndef __BLKDEVSIM__H__
#define __BLKDEVSIM__H__

#include <string>
#include <map>

class BlockDeviceSimulator {
public:
	BlockDeviceSimulator(std::string fname);
	~BlockDeviceSimulator();

	void read(int addr, int size, char *ans);
	void write(int addr, int size, const char *data);

	static const int DEVICE_SIZE = 1024 * 1024;
	std::map<std::string, int> inodes;
	std::map<std::string, int> sizes;
	unsigned long sizeofHeader;
private:
	int fd;
	unsigned char *filemap;
};

#endif // __BLKDEVSIM__H__
