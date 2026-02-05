#include "EditCommand.h"
#include "..\DriveMount.h"
#include "..\FileSystem.h"
#include "..\String.h"
#include "..\Drawing.h"
#include "..\InputManager.h"
#include "..\TerminalBuffer.h"
#include <string>
#include <vector>
#include <xtl.h>

#ifndef FILE_ATTRIBUTE_DIRECTORY
#define FILE_ATTRIBUTE_DIRECTORY 0x00000010
#endif
#ifndef ERROR_PATH_NOT_FOUND
#define ERROR_PATH_NOT_FOUND 3
#endif
#ifndef ERROR_FILE_NOT_FOUND
#define ERROR_FILE_NOT_FOUND 2
#endif

static const size_t EDIT_MAX_SIZE = 65536;
static const size_t EDIT_MAX_LINES = 10000;

static bool IsSwitch(const std::string& a)
{
    return (a.length() >= 1 && (a[0] == '/' || a[0] == '-'));
}

static void ResolvePath(const std::string& pathArg, const std::string& currentDir, std::string& outPath)
{
    outPath = currentDir;
    size_t colon = pathArg.find(':');
    if (colon != std::string::npos)
    {
        std::string drivePart = String::ToUpper(pathArg.substr(0, colon));
        std::string pathPart = pathArg.substr(colon + 1);
        while (!pathPart.empty() && (pathPart[0] == '\\' || pathPart[0] == '/'))
        {
            pathPart.erase(0, 1);
        }
        if (!drivePart.empty())
        {
            DriveMount::Mount(drivePart.c_str());
            outPath = drivePart + "\\";
            if (!pathPart.empty() && pathPart != "." && pathPart != "..")
            {
                outPath += pathPart;
            }
        }
    }
    else if (!pathArg.empty() && pathArg != "." && pathArg != "..")
    {
        if (outPath.length() > 0 && outPath[outPath.length() - 1] != '\\')
        {
            outPath += "\\";
        }
        outPath += pathArg;
    }
    while (outPath.length() > 0 && (outPath[outPath.length() - 1] == '\\' || outPath[outPath.length() - 1] == '/'))
    {
        outPath.erase(outPath.length() - 1, 1);
    }
    if (outPath.empty() && currentDir.length() > 0)
    {
        outPath = currentDir;
        while (outPath.length() > 0 && (outPath[outPath.length() - 1] == '\\' || outPath[outPath.length() - 1] == '/'))
        {
            outPath.erase(outPath.length() - 1, 1);
        }
    }
}

/** Load file into lines. Returns empty on success, error message otherwise. */
static std::string LoadFile(const std::string& path, std::vector<std::string>& lines)
{
    lines.clear();
    if (path.empty())
    {
        return "The syntax of the command is incorrect.\n";
    }
    std::string apiPath = FileSystem::ToApiPath(path);
    DWORD attrs = GetFileAttributesA(apiPath.c_str());
    if (attrs != 0xFFFFFFFF && (attrs & FILE_ATTRIBUTE_DIRECTORY))
    {
        return "Access is denied.\n";
    }
    HANDLE h = CreateFileA(apiPath.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE)
    {
        if (GetLastError() == ERROR_PATH_NOT_FOUND || GetLastError() == ERROR_FILE_NOT_FOUND)
        {
            HANDLE hCreate = CreateFileA(apiPath.c_str(), GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
            if (hCreate != INVALID_HANDLE_VALUE)
            {
                CloseHandle(hCreate);
            }
            lines.push_back("");
            return "";
        }
        return "Access is denied.\n";
    }
    DWORD sizeHigh = 0;
    DWORD sizeLow = GetFileSize(h, &sizeHigh);
    if (sizeLow == 0xFFFFFFFF && GetLastError() != NO_ERROR)
    {
        CloseHandle(h);
        return "Access is denied.\n";
    }
    if (sizeHigh != 0 || sizeLow > (DWORD)EDIT_MAX_SIZE)
    {
        CloseHandle(h);
        return "File too large.\n";
    }
    std::string raw;
    raw.reserve(sizeLow);
    char buf[4096];
    DWORD read = 0;
    while (ReadFile(h, buf, sizeof(buf), &read, NULL) && read > 0)
    {
        for (DWORD i = 0; i < read; i++)
        {
            char c = buf[i];
            if (c != '\0')
            {
                raw += c;
            }
        }
    }
    CloseHandle(h);

    std::string line;
    for (size_t i = 0; i < raw.length(); i++)
    {
        char c = raw[i];
        if (c == '\n')
        {
            if (lines.size() < EDIT_MAX_LINES)
            {
                lines.push_back(line);
            }
            line.clear();
        }
        else if (c != '\r')
        {
            line += c;
        }
    }
    if (lines.size() < EDIT_MAX_LINES)
    {
        lines.push_back(line);
    }
    if (lines.empty())
    {
        lines.push_back("");
    }
    return "";
}

/** Save lines to file. Returns empty on success, error message otherwise. */
static std::string SaveFile(const std::string& path, const std::vector<std::string>& lines)
{
    if (path.empty())
    {
        return "The syntax of the command is incorrect.\n";
    }
    std::string apiPath = FileSystem::ToApiPath(path);
    HANDLE h = CreateFileA(apiPath.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE)
    {
        return "Access is denied.\n";
    }
    for (size_t i = 0; i < lines.size(); i++)
    {
        const std::string& line = lines[i];
        DWORD written = 0;
        if (!WriteFile(h, line.c_str(), (DWORD)line.length(), &written, NULL) || written != (DWORD)line.length())
        {
            CloseHandle(h);
            return "Access is denied.\n";
        }
        const char crlf[] = "\r\n";
        if (!WriteFile(h, crlf, 2, &written, NULL) || written != 2)
        {
            CloseHandle(h);
            return "Access is denied.\n";
        }
    }
    CloseHandle(h);
    return "";
}

static void ClampCursor(size_t& cursorRow, size_t& cursorCol, const std::vector<std::string>& lines)
{
    if (lines.empty())
    {
        cursorRow = 0;
        cursorCol = 0;
        return;
    }
    if (cursorRow >= lines.size())
    {
        cursorRow = lines.size() - 1;
    }
    const std::string& line = lines[cursorRow];
    if (cursorCol > line.length())
    {
        cursorCol = line.length();
    }
}

static void RunEditorLoop(const std::string& path, std::vector<std::string>& lines)
{
    const int rows = TerminalBuffer::GetRows();
    const int cols = TerminalBuffer::GetCols();
    const int contentRows = rows - 1;
    if (contentRows <= 0 || cols <= 0)
    {
        return;
    }

    size_t cursorRow = 0;
    size_t cursorCol = 0;
    int scrollRow = 0;
    int scrollCol = 0;
    bool dirty = false;
    bool exitRequested = false;
    std::vector<char> screenBuffer((size_t)(rows * cols), ' ');

    ClampCursor(cursorRow, cursorCol, lines);

    while (!exitRequested)
    {
        InputManager::PumpInput();

        KeyboardState keyboardState;
        if (InputManager::TryGetKeyboardState(-1, &keyboardState))
        {
            if (keyboardState.KeyDown)
            {
                char vk = keyboardState.VirtualKey;
                char ascii = keyboardState.Ascii;

                if (vk == VK_F3)
                {
                    exitRequested = true;
                    continue;
                }
                if (vk == VK_F2)
                {
                    std::string err = SaveFile(path, lines);
                    if (!err.empty())
                    {
                        (void)err;
                    }
                    dirty = false;
                    continue;
                }
                if (vk == VK_UP)
                {
                    if (cursorRow > 0)
                    {
                        cursorRow--;
                        cursorCol = (cursorCol > lines[cursorRow].length()) ? lines[cursorRow].length() : cursorCol;
                        if ((int)cursorRow < scrollRow)
                        {
                            scrollRow = (int)cursorRow;
                        }
                    }
                    continue;
                }
                if (vk == VK_DOWN)
                {
                    if (cursorRow + 1 < lines.size())
                    {
                        cursorRow++;
                        cursorCol = (cursorCol > lines[cursorRow].length()) ? lines[cursorRow].length() : cursorCol;
                        if (cursorRow >= (size_t)(scrollRow + contentRows))
                        {
                            scrollRow = (int)cursorRow - contentRows + 1;
                        }
                    }
                    continue;
                }
                if (vk == VK_LEFT)
                {
                    if (cursorCol > 0)
                    {
                        cursorCol--;
                    }
                    else if (cursorRow > 0)
                    {
                        cursorRow--;
                        cursorCol = lines[cursorRow].length();
                    }
                    continue;
                }
                if (vk == VK_RIGHT)
                {
                    const std::string& line = lines[cursorRow];
                    if (cursorCol < line.length())
                    {
                        cursorCol++;
                    }
                    else if (cursorRow + 1 < lines.size())
                    {
                        cursorRow++;
                        cursorCol = 0;
                        if (cursorRow >= (size_t)(scrollRow + contentRows))
                        {
                            scrollRow = (int)cursorRow - contentRows + 1;
                        }
                    }
                    continue;
                }
                if (vk == VK_HOME)
                {
                    cursorCol = 0;
                    continue;
                }
                if (vk == VK_END)
                {
                    cursorCol = lines[cursorRow].length();
                    continue;
                }
                if (vk == VK_BACK)
                {
                    if (cursorCol > 0)
                    {
                        lines[cursorRow].erase(cursorCol - 1, 1);
                        cursorCol--;
                        dirty = true;
                    }
                    else if (cursorRow > 0)
                    {
                        size_t prevLen = lines[cursorRow - 1].length();
                        lines[cursorRow - 1] += lines[cursorRow];
                        lines.erase(lines.begin() + (int)cursorRow);
                        cursorRow--;
                        cursorCol = prevLen;
                        dirty = true;
                    }
                    continue;
                }
                if (vk == VK_DELETE)
                {
                    const std::string& line = lines[cursorRow];
                    if (cursorCol < line.length())
                    {
                        lines[cursorRow].erase(cursorCol, 1);
                        dirty = true;
                    }
                    else if (cursorRow + 1 < lines.size())
                    {
                        lines[cursorRow] += lines[cursorRow + 1];
                        lines.erase(lines.begin() + (int)(cursorRow + 1));
                        dirty = true;
                    }
                    continue;
                }
                if (ascii == '\r' || ascii == '\n')
                {
                    std::string tail = lines[cursorRow].substr(cursorCol);
                    lines[cursorRow].erase(cursorCol);
                    lines.insert(lines.begin() + (int)(cursorRow + 1), tail);
                    cursorRow++;
                    cursorCol = 0;
                    if (cursorRow >= (size_t)(scrollRow + contentRows))
                    {
                        scrollRow = (int)cursorRow - contentRows + 1;
                    }
                    dirty = true;
                    continue;
                }
                if (ascii >= 32 && ascii < 127)
                {
                    lines[cursorRow].insert(cursorCol, 1, ascii);
                    cursorCol++;
                    dirty = true;
                    continue;
                }
            }
        }

        if ((int)cursorRow < scrollRow)
        {
            scrollRow = (int)cursorRow;
        }
        if (cursorRow >= (size_t)(scrollRow + contentRows))
        {
            scrollRow = (int)cursorRow - contentRows + 1;
        }
        if ((int)cursorCol < scrollCol)
        {
            scrollCol = (int)cursorCol;
        }
        if ((int)cursorCol >= scrollCol + cols)
        {
            scrollCol = (int)cursorCol - cols + 1;
        }
        {
            int maxScrollCol = (int)lines[cursorRow].length() - cols;
            if (maxScrollCol < 0)
            {
                maxScrollCol = 0;
            }
            if (scrollCol > maxScrollCol)
            {
                scrollCol = maxScrollCol;
            }
        }
        if (scrollCol < 0)
        {
            scrollCol = 0;
        }

        for (int r = 0; r < rows; r++)
        {
            for (int c = 0; c < cols; c++)
            {
                screenBuffer[(size_t)(r * cols + c)] = ' ';
            }
        }
        for (int r = 0; r < contentRows; r++)
        {
            int lineIndex = scrollRow + r;
            const char* linePtr = " ";
            size_t lineLen = 0;
            if (lineIndex >= 0 && (size_t)lineIndex < lines.size())
            {
                linePtr = lines[(size_t)lineIndex].c_str();
                lineLen = lines[(size_t)lineIndex].length();
            }
            for (int c = 0; c < cols; c++)
            {
                int srcCol = scrollCol + c;
                char ch = (srcCol >= 0 && (size_t)srcCol < lineLen) ? linePtr[srcCol] : ' ';
                if (ch < 32 || ch > 126)
                {
                    ch = ' ';
                }
                screenBuffer[(size_t)(r * cols + c)] = ch;
            }
        }
        std::string status = String::Format("Row %u,%u", (unsigned int)(cursorRow + 1), (unsigned int)(cursorCol + 1));
        if (status.length() > (size_t)cols)
        {
            status = status.substr(0, (size_t)cols);
        }
        for (size_t i = 0; i < status.length() && i < (size_t)cols; i++)
        {
            screenBuffer[(size_t)((contentRows * cols) + i)] = status[i];
        }
        size_t pos = status.length();
        if (pos + 2 <= (size_t)cols)
        {
            screenBuffer[(size_t)(contentRows * cols + pos)] = ' ';
            screenBuffer[(size_t)(contentRows * cols + pos + 1)] = ' ';
            pos += 2;
        }
        const char* f2f3 = "F2=Save F3=Exit";
        for (size_t i = 0; i < 15 && pos + i < (size_t)cols; i++)
        {
            screenBuffer[(size_t)(contentRows * cols + pos + i)] = f2f3[i];
        }

        int cursorX = (int)cursorCol - scrollCol;
        int cursorY = (int)(cursorRow - scrollRow);
        if (cursorY >= 0 && cursorY < contentRows)
        {
            uint32_t color = TerminalBuffer::GetTextColor();
            Drawing::DrawTerminal(&screenBuffer[0], color, cursorX, cursorY, true);
        }
        else
        {
            uint32_t color = TerminalBuffer::GetTextColor();
            Drawing::DrawTerminal(&screenBuffer[0], color, -1, -1, false);
        }

        Sleep(16);
    }

    if (dirty)
    {
        SaveFile(path, lines);
    }
}

bool EditCommand::Matches(const std::string& cmd)
{
    return (cmd == "EDIT");
}

std::string EditCommand::Execute(const std::vector<std::string>& args, CommandContext& ctx)
{
    if (args.size() < 2)
    {
        return "The syntax of the command is incorrect.\n";
    }
    std::string path;
    ResolvePath(args[1], ctx.currentDir, path);
    if (path.empty())
    {
        return "The syntax of the command is incorrect.\n";
    }
    std::vector<std::string> lines;
    std::string err = LoadFile(path, lines);
    if (!err.empty())
    {
        return err;
    }
    RunEditorLoop(path, lines);
    return "\x02";
}
