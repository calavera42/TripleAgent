#include "GlobalErrorHandler.h"

// somente sdl3
void ErrorHandler::CheckSDLError(bool ret, const char* str)
{
	if (ret == true)
		return;

	std::string errorMessage = SDL_GetError();
	
	printf("Erro do SDL.\n\tErro:\t%s\n\tOrigem:\t%s", errorMessage.c_str(), str);
}