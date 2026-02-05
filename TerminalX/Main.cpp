#include <xtl.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <math.h>
#include <stdlib.h>
#include <ctype.h>

#include "Drawing.h"
#include "Debug.h"
#include "InputManager.h"
#include "External.h"
#include "Resources.h"
#include "TerminalBuffer.h"
#include "CommandProcessor.h"
#include "DriveMount.h"
#include "ssfn.h"

#include <xgraphics.h>

#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ|D3DFVF_DIFFUSE|D3DFVF_TEX1)

typedef struct {
    DWORD dwWidth;
    DWORD dwHeight;
    BOOL  fProgressive;
    BOOL  fWideScreen;
	DWORD dwFreq;
} DISPLAY_MODE;

DISPLAY_MODE displayModes[] =
{
	// HDTV Progressive Modes
    {  1280,    720,    TRUE,   TRUE,  60 },         // 1280x720 progressive 16x9
	// EDTV Progressive Modes
    {   720,    480,    TRUE,   TRUE,  60 },         // 720x480 progressive 16x9
    {   640,    480,    TRUE,   TRUE,  60 },         // 640x480 progressive 16x9
    {   720,    480,    TRUE,   FALSE, 60 },         // 720x480 progressive 4x3
    {   640,    480,    TRUE,   FALSE, 60 },         // 640x480 progressive 4x3
	// SDTV PAL-50 Interlaced Modes
    {   720,    480,    FALSE,  TRUE,  50 },         // 720x480 interlaced 16x9 50Hz
    {   640,    480,    FALSE,  TRUE,  50 },         // 640x480 interlaced 16x9 50Hz
    {   720,    480,    FALSE,  FALSE, 50 },         // 720x480 interlaced 4x3  50Hz
    {   640,    480,    FALSE,  FALSE, 50 },         // 640x480 interlaced 4x3  50Hz
	// SDTV NTSC / PAL-60 Interlaced Modes
    {   720,    480,    FALSE,  TRUE,  60 },         // 720x480 interlaced 16x9
    {   640,    480,    FALSE,  TRUE,  60 },         // 640x480 interlaced 16x9
    {   720,    480,    FALSE,  FALSE, 60 },         // 720x480 interlaced 4x3
    {   640,    480,    FALSE,  FALSE, 60 },         // 640x480 interlaced 4x3
};

#define NUM_MODES (sizeof(displayModes) / sizeof(displayModes[0]))

static void WaitButton(ControllerButton controllerButton)
{
	while (true)
	{
		InputManager::ProcessController();
        if (InputManager::ControllerPressed(controllerButton, -1))
		{
			return;
		}
		Sleep(100);
	}
}

#define VK_BACK  0x08
#define VK_PRIOR 0x21  /* Page Up */
#define VK_NEXT  0x22  /* Page Down */
#define VK_LEFT  0x25  /* Left arrow */
#define VK_UP    0x26  /* Up arrow */
#define VK_RIGHT 0x27  /* Right arrow */
#define VK_DOWN  0x28  /* Down arrow */
#define CURSOR_BLINK_MS 530
#define COMMAND_HISTORY_MAX 50

static std::vector<std::string> s_commandHistory;
static size_t s_historyIndex = 0;  /* when == size, we're at "new" line */

static void InitTerminalBuffer()
{
    TerminalBuffer::Clear();
    TerminalBuffer::SetCursor(0, TerminalBuffer::GetRows() - 4);
    TerminalBuffer::Write("Welcome to Terminal-X....\n");
    TerminalBuffer::Write("Type HELP for commands.\n");
    TerminalBuffer::Write("");
    TerminalBuffer::SetPrompt(CommandProcessor::GetCurrentDirForPrompt() + "> ");
    TerminalBuffer::SetCursor(0, TerminalBuffer::GetRows() - 1);
}

static void SubmitCommand()
{
    std::string line = TerminalBuffer::GetInputLine();
    /* Allow empty line when waiting for DATE/TIME (Press ENTER to keep the same date/time) */
    if (line.empty() && CommandProcessor::GetPendingInputType() == CommandProcessor::PendingNone)
    {
        TerminalBuffer::UpdateInputRow();
        return;
    }
    /* Add non-empty command to history (skip if same as last) */
    if (!line.empty())
    {
        if (s_commandHistory.empty() || s_commandHistory.back() != line)
        {
            s_commandHistory.push_back(line);
            if (s_commandHistory.size() > (size_t)COMMAND_HISTORY_MAX)
            {
                s_commandHistory.erase(s_commandHistory.begin());
            }
        }
        s_historyIndex = s_commandHistory.size();
    }
    /* Scroll so current line (prompt+command) becomes history */
    TerminalBuffer::ScrollUp();
    TerminalBuffer::SetCursor(0, TerminalBuffer::GetRows() - 1);
    TerminalBuffer::ClearInputLine();
    TerminalBuffer::UpdateInputRow();

    /* If we're waiting for DATE/TIME input, submit the line as the new date/time (empty = keep same) */
    if (CommandProcessor::GetPendingInputType() != CommandProcessor::PendingNone)
    {
        std::string result = CommandProcessor::SubmitPendingInput(line);
        if (!result.empty())
        {
            TerminalBuffer::SetCursor(0, TerminalBuffer::GetRows() - 1);
            TerminalBuffer::WriteRaw(result);
        }
        TerminalBuffer::SetPrompt(CommandProcessor::GetCurrentDirForPrompt() + "> ");
        TerminalBuffer::SetCursor(0, TerminalBuffer::GetRows() - 1);
        TerminalBuffer::UpdateInputRow();
        return;
    }

    std::vector<std::string> args = CommandProcessor::ParseLine(line);
    std::string result = CommandProcessor::Execute(args);
    if (result.length() >= 1 && result[0] == '\x01')
    {
        TerminalBuffer::Clear();
        TerminalBuffer::SetCursor(0, TerminalBuffer::GetRows() - 1);
    }
    else if (result.length() >= 1 && result[0] == '\x02')
    {
        return;
    }
    else if (result.length() >= 1 && result[0] == '\x03')
    {
        /* DATE/TIME prompt: display message after first newline (current date/time + "Enter the new ...") */
        size_t pos = result.find('\n');
        std::string message = (pos != std::string::npos) ? result.substr(pos + 1) : result.substr(1);
        TerminalBuffer::SetCursor(0, TerminalBuffer::GetRows() - 1);
        TerminalBuffer::WriteRaw(message);
        /* Keep "Enter the new date/time" visible: set prompt so UpdateInputRow doesn't overwrite with HDD0-E:\> */
        if (CommandProcessor::GetPendingInputType() == CommandProcessor::PendingDate)
            TerminalBuffer::SetPrompt("Enter the new date: (yy-mm-dd) ");
        else if (CommandProcessor::GetPendingInputType() == CommandProcessor::PendingTime)
            TerminalBuffer::SetPrompt("Enter the new time: ");
    }
    else if (!result.empty())
    {
        TerminalBuffer::SetCursor(0, TerminalBuffer::GetRows() - 1);
        TerminalBuffer::WriteRaw(result);
    }

    if (result.empty() || result.length() < 1 || result[0] != '\x03')
        TerminalBuffer::SetPrompt(CommandProcessor::GetCurrentDirForPrompt() + "> ");
    TerminalBuffer::SetCursor(0, TerminalBuffer::GetRows() - 1);
    TerminalBuffer::UpdateInputRow();
}

bool SupportsMode(DISPLAY_MODE mode, DWORD dwVideoStandard, DWORD dwVideoFlags)
{
    if (mode.dwFreq == 60 && !(dwVideoFlags & XC_VIDEO_FLAGS_PAL_60Hz) && (dwVideoStandard == XC_VIDEO_STANDARD_PAL_I))
    {
        return false;
    }
    if (mode.dwFreq == 50 && (dwVideoStandard != XC_VIDEO_STANDARD_PAL_I))
    {
        return false;
    }
    if (mode.dwHeight == 480 && mode.fWideScreen && !(dwVideoFlags & XC_VIDEO_FLAGS_WIDESCREEN ))
    {
        return false;
    }
    if (mode.dwHeight == 480 && mode.fProgressive && !(dwVideoFlags & XC_VIDEO_FLAGS_HDTV_480p))
    {
        return false;
    }
    if (mode.dwHeight == 720 && !(dwVideoFlags & XC_VIDEO_FLAGS_HDTV_720p))
    {
        return false;
    }
    if (mode.dwHeight == 1080 && !(dwVideoFlags & XC_VIDEO_FLAGS_HDTV_1080i))
    {
        return false;
    }
    return true;
}

bool CreateDevice()
{
	uint32_t videoFlags = XGetVideoFlags();
	uint32_t videoStandard = XGetVideoStandard();
	uint32_t currentMode;
    for (currentMode = 0; currentMode < NUM_MODES-1; currentMode++)
    {
        if (SupportsMode(displayModes[currentMode], videoStandard, videoFlags))
        {
            break;
        }
    } 

	LPDIRECT3D8 d3d = Direct3DCreate8(D3D_SDK_VERSION);
    if(d3d == NULL)
	{
		Debug::Print("Failed to create d3d\n");
        return false;
	}

	Drawing::SetBufferWidth(displayModes[currentMode].dwWidth);
	Drawing::SetBufferHeight(displayModes[currentMode].dwHeight);

	D3DPRESENT_PARAMETERS params; 
    ZeroMemory(&params, sizeof(params));
	params.BackBufferWidth = displayModes[currentMode].dwWidth;
    params.BackBufferHeight = displayModes[currentMode].dwHeight;
	params.Flags = displayModes[currentMode].fProgressive ? D3DPRESENTFLAG_PROGRESSIVE : D3DPRESENTFLAG_INTERLACED;
    params.Flags |= displayModes[currentMode].fWideScreen ? D3DPRESENTFLAG_WIDESCREEN : 0;
    params.FullScreen_RefreshRateInHz = displayModes[currentMode].dwFreq;
	params.BackBufferFormat = D3DFMT_X8R8G8B8;
    params.BackBufferCount = 1;
    params.EnableAutoDepthStencil = FALSE;
	params.SwapEffect = D3DSWAPEFFECT_DISCARD;
    params.FullScreen_PresentationInterval = D3DPRESENT_INTERVAL_ONE;

	LPDIRECT3DDEVICE8 d3dDevice;
    if (FAILED(d3d->CreateDevice(0, D3DDEVTYPE_HAL, NULL, D3DCREATE_HARDWARE_VERTEXPROCESSING, &params, &d3dDevice)))
	{
		Debug::Print("Failed to create device\n");
        return false;
	}
	Drawing::SetD3dDevice(d3dDevice);
    
    Drawing::GetD3dDevice()->SetRenderState(D3DRS_ZFUNC, D3DCMP_NEVER);
	Drawing::GetD3dDevice()->SetRenderState(D3DRS_LIGHTING, FALSE);
	Drawing::GetD3dDevice()->SetVertexShader(D3DFVF_CUSTOMVERTEX);
	Drawing::GetD3dDevice()->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	Drawing::GetD3dDevice()->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	Drawing::GetD3dDevice()->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	Drawing::GetD3dDevice()->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	Drawing::GetD3dDevice()->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	Drawing::GetD3dDevice()->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	Drawing::GetD3dDevice()->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	Drawing::GetD3dDevice()->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
	Drawing::GetD3dDevice()->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);

	Drawing::GetD3dDevice()->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
	Drawing::GetD3dDevice()->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
	Drawing::GetD3dDevice()->SetTextureStageState(0, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);

	D3DXMATRIX matIdentity;
	D3DXMatrixIdentity(&matIdentity);
	D3DXMATRIX matProjection;
	D3DXMatrixOrthoOffCenterLH(&matProjection, 0, (float)Drawing::GetBufferWidth(), 0, (float)Drawing::GetBufferHeight(), 1.0f, 800.0f);
	Drawing::GetD3dDevice()->SetTransform(D3DTS_VIEW, &matIdentity);
	Drawing::GetD3dDevice()->SetTransform(D3DTS_WORLD, &matIdentity);
	Drawing::GetD3dDevice()->SetTransform(D3DTS_PROJECTION, &matProjection);
	Drawing::GetD3dDevice()->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
	Drawing::GetD3dDevice()->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);

	Drawing::GetD3dDevice()->BeginScene();
	Drawing::GetD3dDevice()->Clear(0L, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER|D3DCLEAR_STENCIL, 0xff000000, 1.0f, 0L);
	Drawing::GetD3dDevice()->EndScene();
	Drawing::GetD3dDevice()->Present(NULL, NULL, NULL, NULL);

	return true;
}

void __cdecl main()
{
	Debug::Print("Welcome to Xbox HD Updater\n");

	bool deviceCreated = CreateDevice();

	Drawing::GenerateBitmapFont();

    InputManager::Init();

    DriveMount::Mount("HDD0-E");

	InitTerminalBuffer();

    bool exitRequested = false;
    while (!exitRequested)
    {
        InputManager::PumpInput();

        KeyboardState keyboardState;
        if (InputManager::TryGetKeyboardState(-1, &keyboardState))
        {
            if (keyboardState.KeyDown)
            {
                if (keyboardState.Ascii == '\r' || keyboardState.Ascii == '\n')
                {
                    std::string line = TerminalBuffer::GetInputLine();
                    SubmitCommand();
                    std::vector<std::string> args = CommandProcessor::ParseLine(line);
                    if (!args.empty())
                    {
                        std::string cmd = args[0];
                        for (size_t i = 0; i < cmd.length(); i++)
                            cmd[i] = (char)toupper((unsigned char)cmd[i]);
                        if (cmd == "EXIT")
                            exitRequested = true;
                    }
                }
                else if ((unsigned char)keyboardState.VirtualKey == VK_PRIOR)
                {
                    TerminalBuffer::ScrollPageUp();
                }
                else if ((unsigned char)keyboardState.VirtualKey == VK_NEXT)
                {
                    TerminalBuffer::ScrollPageDown();
                }
                else if ((unsigned char)keyboardState.VirtualKey == VK_UP)
                {
                    TerminalBuffer::ScrollToBottom();
                    if (!s_commandHistory.empty())
                    {
                        if (s_historyIndex > 0)
                        {
                            s_historyIndex--;
                            TerminalBuffer::SetInputLine(s_commandHistory[s_historyIndex]);
                        }
                        else if (s_historyIndex == s_commandHistory.size())
                        {
                            s_historyIndex = s_commandHistory.size() - 1;
                            TerminalBuffer::SetInputLine(s_commandHistory[s_historyIndex]);
                        }
                    }
                }
                else if ((unsigned char)keyboardState.VirtualKey == VK_LEFT)
                {
                    TerminalBuffer::ScrollToBottom();
                    TerminalBuffer::MoveInputCursorLeft();
                }
                else if ((unsigned char)keyboardState.VirtualKey == VK_RIGHT)
                {
                    TerminalBuffer::ScrollToBottom();
                    TerminalBuffer::MoveInputCursorRight();
                }
                else if ((unsigned char)keyboardState.VirtualKey == VK_DOWN)
                {
                    TerminalBuffer::ScrollToBottom();
                    if (!s_commandHistory.empty())
                    {
                        if (s_historyIndex < s_commandHistory.size() - 1)
                        {
                            s_historyIndex++;
                            TerminalBuffer::SetInputLine(s_commandHistory[s_historyIndex]);
                        }
                        else
                        {
                            s_historyIndex = s_commandHistory.size();
                            TerminalBuffer::ClearInputLine();
                        }
                    }
                }
                else if ((unsigned char)keyboardState.VirtualKey == VK_BACK || keyboardState.Ascii == '\x08')
                {
                    TerminalBuffer::ScrollToBottom();
                    TerminalBuffer::BackspaceInput();
                }
                else if (keyboardState.Ascii >= 32 && keyboardState.Ascii <= 126)
                {
                    TerminalBuffer::ScrollToBottom();
                    TerminalBuffer::AppendInputChar((char)keyboardState.Ascii);
                }
            }
        }

        if (InputManager::ControllerPressed(ControllerA, -1))
        {
            break;
        }

        TerminalBuffer::UpdateInputRow();
        DWORD tick = GetTickCount();
        bool cursorOn = (tick % (CURSOR_BLINK_MS * 2)) < CURSOR_BLINK_MS;
        int curX = TerminalBuffer::GetInputCursorX();
        int curY = TerminalBuffer::GetInputCursorY();
        Drawing::DrawTerminal(TerminalBuffer::GetBuffer(), TerminalBuffer::GetTextColor(), curX, curY, cursorOn);
        Sleep(0);
    }

    HalReturnToFirmware(2);
}
