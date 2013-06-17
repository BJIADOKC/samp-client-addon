#pragma once



#include "screen.h"





addonScreen *gScreen;


extern addonFS *gFS;





void addonScreen::Get(std::string filename)
{
	gFS->RemoveFile(filename);

	DWORD addr = 0x5D0820; // screenshot take addr
	char *c_filename = new char[(filename.length() + 1)];
	
	strcpy(c_filename, filename.c_str());

	__asm
	{
		mov eax, [0xB6F97C]
		mov eax, [eax]
		push c_filename
		push eax
		call addr
		add esp, 8
	}

	delete[] c_filename;
}
