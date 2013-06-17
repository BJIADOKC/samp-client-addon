#pragma once



#include "addon.h"
#include <dirent.h>





class addonFS
{

public:

	void RemoveFile(std::string file);
	std::vector<std::string> ListDirectory(std::string folder);

private:

	struct dirent *entry;
	DIR *directory;
};