#include "include/AGXWin.h"

#include "src/Agent/AgentWindow.h"
#include "src/Balloon/BalloonWindow.h"

#include <windows.h>

AGENT_WIN IAgentWindow* CreateAgentWindow()
{
    return new AgentWindow();
}

AGENT_WIN IBalloonWindow* CreateBalloonWindow()
{
    return new BalloonWindow();
}

AGENT_WIN void DestroyAgentWindow(IAgentWindow* win)
{
    delete win;
}

AGENT_WIN void DestroyBalloonWindow(IBalloonWindow* win)
{
    delete win;
}

AGENT_WIN void ProcessMessages()
{
    DWORD result = MsgWaitForMultipleObjects(0, NULL, FALSE, 10, QS_ALLINPUT);

    if (result == WAIT_OBJECT_0)
    {
        MSG message;
        while (PeekMessage(&message, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&message);
            DispatchMessage(&message);
        }
    }
}
