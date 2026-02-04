#include "TerminalBuffer.h"
#include "Drawing.h"

namespace
{
    char* s_buffer = NULL;
    int s_cursor_x = 0;
    int s_cursor_y = 0;
    std::string s_prompt = "C:\\> ";
    std::string s_inputLine;
    unsigned char s_colorAttr = 0x0A;
    unsigned char s_colorAttrDefault = 0x0A;

    static const unsigned int s_colorTable[16] =
    {
        0xFF000000, 0xFF0000AA, 0xFF00AA00, 0xFF00AAAA,
        0xFFAA0000, 0xFFAA00AA, 0xFFAAAA00, 0xFFAAAAAA,
        0xFF555555, 0xFF5555FF, 0xFF55FF55, 0xFF55FFFF,
        0xFFFF5555, 0xFFFF55FF, 0xFFFFFF55, 0xFFFFFFFF
    };
}

void TerminalBuffer::SetColorAttribute(unsigned char attr)
{
    s_colorAttr = attr;
}

void TerminalBuffer::ResetColorAttribute()
{
    s_colorAttr = s_colorAttrDefault;
}

unsigned char TerminalBuffer::GetColorAttribute()
{
    return s_colorAttr;
}

unsigned int TerminalBuffer::GetTextColor()
{
    return s_colorTable[s_colorAttr & 0x0F];
}

unsigned int TerminalBuffer::GetBackgroundColor()
{
    return s_colorTable[(s_colorAttr >> 4) & 0x0F];
}

void TerminalBuffer::Init()
{
    if (s_buffer != NULL)
    {
        return;
    }
    s_buffer = (char*)malloc(GetRows() * GetCols());
}

void TerminalBuffer::Clear()
{
    Init();

    for (int row = 0; row < GetRows(); row++)
    {
        for (int col = 0; col < GetCols(); col++)
        {
            s_buffer[(row * GetCols()) + col] = ' ';
        }
    }
    s_cursor_x = 0;
    s_cursor_y = 0;
}

void TerminalBuffer::SetCursor(int x, int y)
{
    s_cursor_x = (x >= 0 && x < GetCols()) ? x : 0;
    s_cursor_y = (y >= 0 && y < GetRows()) ? y : 0;
}

void TerminalBuffer::Write(std::string message, ...)
{
    Init();

    char buffer[1024];
    va_list arglist;
    va_start(arglist, message);
    _vsnprintf(buffer, 1024, message.c_str(), arglist);
    va_end(arglist);
    buffer[1024 - 1] = '\0';

    for (const char* p = buffer; *p; p++)
    {
        if (*p == '\n')
        {
            s_cursor_x = 0;
            s_cursor_y++;
            if (s_cursor_y >= GetRows())
            {
                ScrollUp();
                s_cursor_y = GetRows() - 1;
            }
            continue;
        }

        if (s_cursor_x >= GetCols())
        {
            s_cursor_x = 0;
            s_cursor_y++;
            if (s_cursor_y >= GetRows())
            {
                ScrollUp();
                s_cursor_y = GetRows() - 1;
            }
        }

        if (s_cursor_y >= 0 && s_cursor_y < GetRows() && s_cursor_x >= 0 && s_cursor_x < GetCols())
        {
            s_buffer[(s_cursor_y * GetCols()) + s_cursor_x] = *p;
        }
        s_cursor_x++;
    }
}

void TerminalBuffer::ScrollUp()
{
    Init();

    for (int row = 0; row < GetRows() - 1; row++)
    {
        for (int col = 0; col < GetCols(); col++)
        {
            s_buffer[(row * GetCols()) + col] = s_buffer[((row + 1) * GetCols()) + col];
        }
    }
    for (int col = 0; col < GetCols(); col++)
    {
        s_buffer[((GetRows() - 1) * GetCols()) + col] = ' ';
    }
}

const char* TerminalBuffer::GetBuffer()
{
    return s_buffer;
}

int TerminalBuffer::GetCols()
{
    return Drawing::GetBufferWidth() / TERMINAL_FONT_SIZE_WIDTH;
}

int TerminalBuffer::GetRows()
{
    return Drawing::GetBufferHeight() / TERMINAL_FONT_SIZE_HEIGHT;
}

int TerminalBuffer::GetCursorX()
{
    return s_cursor_x;
}

int TerminalBuffer::GetCursorY()
{
    return s_cursor_y;
}

void TerminalBuffer::SetPrompt(const std::string& prompt)
{
    s_prompt = prompt;
}

const std::string& TerminalBuffer::GetPrompt()
{
    return s_prompt;
}

void TerminalBuffer::AppendInputChar(char c)
{
    if (c < 32 || c > 126)
        return;
    int cols = GetCols();
    int promptLen = (int)s_prompt.length();
    if ((int)s_inputLine.length() + promptLen >= cols)
        return;
    s_inputLine += c;
}

void TerminalBuffer::BackspaceInput()
{
    if (!s_inputLine.empty())
        s_inputLine.erase(s_inputLine.length() - 1, 1);
}

void TerminalBuffer::ClearInputLine()
{
    s_inputLine.clear();
}

const std::string& TerminalBuffer::GetInputLine()
{
    return s_inputLine;
}

void TerminalBuffer::UpdateInputRow()
{
    Init();
    int rows = GetRows();
    int cols = GetCols();
    if (rows == 0)
        return;
    int inputRow = rows - 1;
    std::string line = s_prompt + s_inputLine;
    for (int col = 0; col < cols; col++)
    {
        char ch = (col < (int)line.length()) ? line[col] : ' ';
        s_buffer[(inputRow * cols) + col] = ch;
    }
}

int TerminalBuffer::GetInputCursorX()
{
    return (int)(s_prompt.length() + s_inputLine.length());
}

int TerminalBuffer::GetInputCursorY()
{
    int rows = GetRows();
    return (rows > 0) ? (rows - 1) : 0;
}
