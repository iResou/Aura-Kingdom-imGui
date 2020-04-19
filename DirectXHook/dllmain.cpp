		// OneShot D3D9 hook

#include <Windows.h>
#include "detours.h"
#include "Patternscaning.h"
#include <iostream>
#include "stdafx.h"



#include <d3d9.h>
#include <d3dx9.h>
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")

#include "../ImGui/imgui.h"
#include "../ImGui/imgui_impl_dx9.h"
#include "../ImGui/imgui_impl_win32.h"
#include <Mem.h>


const char* windowName = "AuraKingdom.to Online- VS2013                   ";


extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

typedef HRESULT(__stdcall * f_EndScene)(IDirect3DDevice9 * pDevice); // Our function prototype 
f_EndScene oEndScene; // Original Endscene

typedef LRESULT(CALLBACK* WNDPROC)(HWND, UINT, WPARAM, LPARAM);

WNDPROC oWndProc;


HRESULT __stdcall Hooked_EndScene(IDirect3DDevice9 * pDevice) // Our hooked endscene
{
	/*D3DRECT BarRect = { 100, 100, 200, 200 };
	pDevice->Clear(1, &BarRect, D3DCLEAR_TARGET, D3DCOLOR_ARGB(255, 0, 255, 0), 0.0f, 0);*/
	bool MovSpeed = false;
	uintptr_t moduleBase = (uintptr_t)GetModuleHandle(L"game.bin");

	//calling it with NULL also gives you the address of the .exe module
	moduleBase = (uintptr_t)GetModuleHandle(NULL);
	

	
	static bool init = true;
	if (init)
	{
		init = false;
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		
		ImGui_ImplWin32_Init(FindWindowA(NULL, windowName));
		ImGui_ImplDX9_Init(pDevice);
	}


	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();


	
	
	
	{
		static bool checkBox = false;
		static float sliderFloat = 0.f;
		static int sliderInt = 0;
		ImGui::Checkbox("Disable at city teleport", &checkBox);
		
		ImGui::SliderFloat("Mov Speed Value", &sliderFloat, 10.f, 30.f);
		ImGui::SliderInt("Slider Int", & sliderInt, 0, 80);

		
			if (checkBox == true)
			{
				*(float*)mem::FindDMAAddy(moduleBase + 0x0118D3AC, { 0xC, 0x64, 0x10,0xC,0x14 }) = sliderFloat; //Note: Find more methods to patch bytes
				
			}
		
		//*(float*)mem::FindDMAAddy(moduleBase + 0x0118D3B4, { 0x90, 0x30, 0x4,0xC,0x330,0x1C,0x14 }) = sliderFloat;
	}
		
	
	

	ImGui::EndFrame();
	ImGui::Render();

	ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
	
	return oEndScene(pDevice); // Call original ensdcene so the game can draw
}

LRESULT __stdcall WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

	if (true && ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
		return true;

	return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
}


DWORD WINAPI MainThread(LPVOID param) // Our main thread
{
	HWND  window = FindWindowA(NULL, windowName);

	oWndProc = (WNDPROC)SetWindowLongPtr(window, GWL_WNDPROC, (LONG_PTR)WndProc);

	IDirect3D9 * pD3D = Direct3DCreate9(D3D_SDK_VERSION);

	if (!pD3D)
		return false;

	D3DPRESENT_PARAMETERS d3dpp{ 0 };
	d3dpp.hDeviceWindow = window, d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD , d3dpp.Windowed = TRUE;
	
	IDirect3DDevice9 *Device = nullptr;
	if (FAILED(pD3D->CreateDevice(0, D3DDEVTYPE_HAL, d3dpp.hDeviceWindow, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &Device)))
	{
		pD3D->Release();
		return false;
	}
		

	void ** pVTable = *reinterpret_cast<void***>(Device); 

	if (Device)
		Device->Release() , Device = nullptr;
		
	oEndScene = (f_EndScene)DetourFunction((PBYTE)pVTable[42], (PBYTE)Hooked_EndScene); 
	
	return false; 
}


BOOL APIENTRY DllMain(HMODULE hModule,DWORD  ul_reason_for_call,LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH: // Gets runned when injected
		AllocConsole(); // Enables the console
		freopen("CONIN$", "r", stdin); // Makes it possible to output to output to console with cout.
		freopen("CONOUT$", "w", stdout);
		CreateThread(0, 0, MainThread, hModule, 0, 0); // Creates our thread 
		break;

	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

