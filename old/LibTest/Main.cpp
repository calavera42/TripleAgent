#include "../DataProvider/include/IAgentFile.h"
#include <iostream>

int main() 
{
	IAgentFile* af = CreateAgentFile();
	af->Load("d:/desktop/peedy.acs");

	for (const auto& str : af->GetAnimationNames())
	{
		AnimationInfo ai = af->GetAnimationInfo(str);

		std::printf("%ls %d\n", str.c_str(), ai.Transition);

		for (const auto& frame : ai.Frames)
			std::printf("\t%s %s %d\n", frame.Overlays.size() > 0 ? "FALA" : "NORM", frame.FrameDuration == 0 ? "NULO" : "NORM", frame.ExitFrameIndex);
	}

	while (true);

	return 0;
}