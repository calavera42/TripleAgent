#pragma once

#include <windows.h>
#include <gdiplus.h>

#include "../../../DataProvider/include/Structs.h"

enum class BalloonType {
	Speak,
	Think
};

struct BalloonShapeInfo {
	BalloonInfo* BalloonInfo;
	BalloonType Type;
	AgRect DesiredClientRect;
};

//É interessante separar a lógica do formato do balão da lógica do conteúdo
class IBalloonShape 
{
public:
	virtual void Paint(Gdiplus::Graphics* g, BalloonShapeInfo bsi) = 0;
};
