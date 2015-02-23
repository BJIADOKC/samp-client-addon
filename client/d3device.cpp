#pragma once



#include "client.hpp"





boost::shared_ptr<addonD3Device> gD3Device;


extern boost::shared_ptr<addonDebug> gDebug;





addonD3Device::addonD3Device()
{
	gDebug->traceLastFunction("addonD3Device::addonD3Device() at 0x?????");
	gDebug->Log("Called D3Device constructor");

	originalRender = NULL;
	hookedRender = NULL;
	originalDevice = NULL;
	hookedDevice = NULL;

	renderInstance = NULL;
}



addonD3Device::~addonD3Device()
{
	gDebug->traceLastFunction("addonD3Device::~addonD3Device() at 0x?????");
	gDebug->Log("Called D3Device destructor");
}



void addonD3Device::setRender(IDirect3D9 *render, bool original)
{
	//gDebug->traceLastFunction("addonD3Device::setRender(render = 0x%x, original = %s) at 0x%x", (int)render, ((original) ? ("true") : ("false")), &addonD3Device::setRender);

	boost::unique_lock<boost::shared_mutex> lockit(renMutex);

	if(original)
		originalRender = render;
	else
		hookedRender = render;
}



void addonD3Device::setDevice(IDirect3DDevice9 *device, bool original)
{
	//gDebug->traceLastFunction("addonD3Device::setDevice(device = 0x%x) at 0x%x", (int)device, ((original) ? ("true") : ("false")), &addonD3Device::setDevice);

	boost::unique_lock<boost::shared_mutex> lockit(devMutex);

	if(original)
		originalDevice = device;
	else
		hookedDevice = device;
}



void addonD3Device::Screenshot(std::string filename)
{
	gDebug->traceLastFunction("addonD3Device::Screenshot(filename = '%s') at 0x%x", filename.c_str(), &addonD3Device::Screenshot);

	boost::filesystem::path file(filename);

	if(boost::filesystem::exists(file))
	{
		boost::system::error_code error;

		boost::filesystem::remove(file, error);

		if(error)
			gDebug->Log("Cannot remove file %s: %s (Error code: %i)", filename.c_str(), error.message().c_str(), error.value());
	}

	IDirect3DSurface9 *pSurface;

	boost::shared_lock<boost::shared_mutex> lockit(devMutex);

	originalDevice->CreateOffscreenPlainSurface(GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &pSurface, NULL);
	originalDevice->GetBackBuffer(NULL, NULL, D3DBACKBUFFER_TYPE_MONO, &pSurface);

	D3DXSaveSurfaceToFile(filename.c_str(), D3DXIFF_PNG, pSurface, NULL, NULL);
	pSurface->Release();

	gDebug->Log("Screenshot saved to %s", filename.c_str());
}



void addonD3Device::initRender()
{
	boost::unique_lock<boost::shared_mutex> lockit(d3Mutex);

	if(!renderInstance)
	{
		D3DXCreateFont(hookedDevice, 18, NULL, FW_BOLD, NULL, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, TEXT("Arial"), &renderInstance);
	}
	else
	{
		renderInstance->OnLostDevice();
		renderInstance->OnResetDevice();
	}
}



void addonD3Device::processRender()
{
	boost::upgrade_lock<boost::shared_mutex> lockit(d3Mutex);

	if(!renderList.empty())
	{
		for(std::list<renderData>::iterator i = renderList.begin(); i != renderList.end(); i++)
		{
			RECT rText;

			rText.left = (*i).x;
			rText.top = (*i).y;
			rText.right = 1680;
			rText.bottom = ((*i).y + 200);

			boost::upgrade_to_unique_lock<boost::shared_mutex> lockit_(lockit);
			renderInstance->DrawText(NULL, (*i).text.c_str(), -1, &rText, NULL, D3DCOLOR_ARGB((*i).a, (*i).r, (*i).g, (*i).b));
		}
	}
}



void addonD3Device::renderText(std::string text, int x, int y, int r, int g, int b, int a)
{
	gDebug->traceLastFunction("addonD3Device::renderText(text = '%s', x = 0x%x, y = 0x%x, r = 0x%x, g = 0x%x, b = 0x%x, a = 0x%x) at 0x%x", text.c_str(), x, y, r, g, b, a, &addonD3Device::renderText);

	renderData struct_push;

	struct_push.text = text;
	struct_push.x = x;
	struct_push.y = y;
	struct_push.r = r;
	struct_push.g = g;
	struct_push.b = b;
	struct_push.a = a;

	boost::unique_lock<boost::shared_mutex> lockit(d3Mutex);
	renderList.push_back(struct_push);
}



void addonD3Device::stopLastRender()
{
	gDebug->traceLastFunction("addonD3Device::stopLastRender() at 0x%x", &addonD3Device::stopLastRender);

	boost::unique_lock<boost::shared_mutex> lockit(d3Mutex);
	renderList.pop_back();
}



void addonD3Device::clearRender()
{
	gDebug->traceLastFunction("addonD3Device::clearRender() at 0x%x", &addonD3Device::clearRender);

	boost::unique_lock<boost::shared_mutex> lockit(d3Mutex);
	renderList.clear();
}