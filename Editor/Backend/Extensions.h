#pragma once
#include <optional>
#include "EngineContext.h"
#include "../Window/WindowContext.h"

inline std::optional<std::string> OpenFilePath(const char* filter, Aurora::EngineContext* engineContext)
{
	OPENFILENAMEA fileDialog; //Passes data to and from GetOpenFileName & GetSaveFileName. It stores settings used to create the dialog box and the results of the user's selection. 
	CHAR szFile[260] = { 0 }; //Our selected file path's buffer.
	ZeroMemory(&fileDialog, sizeof(OPENFILENAME)); //Initialize openedFile's memory to 0.

	fileDialog.lStructSize = sizeof(OPENFILENAME); //Sets the struct size. We do this for every Win32 struct.
	fileDialog.hwndOwner = engineContext->GetSubsystem<Aurora::WindowContext>()->GetWindowHWND(0); //Gets our currently open window and retrieves it HWND which we set as the struct's owner.
	fileDialog.lpstrFile = szFile; //Buffer for our file.
	fileDialog.nMaxFile = sizeof(szFile); //Size of our file buffer.
	fileDialog.lpstrFilter = filter; //File filter.
	fileDialog.nFilterIndex = 1; //Which filter is set by default. 
	fileDialog.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR; //The last flag is very important. If you don't do this and call OpenFileName, it will change the working directory for your application to the folder you open the window from.  

	if (GetOpenFileNameA(&fileDialog) == true)
	{
		return fileDialog.lpstrFile; //We return the file path of the file we open and create a string out of the char* path.
	}

	return std::nullopt; //Return empty string if no file is selected. It means the dialog has been cancelled.
}