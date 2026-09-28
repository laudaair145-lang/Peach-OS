#include "efi.h"

EFI_GUID gopGuid = {0x9042a9de, 0x23dc, 0x4a38, {0x96, 0xfb, 0x7a, 0xde, 0xd0, 0x80, 0x51, 0x6a}};

// --- Bare-Metal Compiler Dependencies ---
void *memset(void *s, int c, uint64_t n) {
    unsigned char *p = (unsigned char *)s;
    while (n--) *p++ = (unsigned char)c;
    return s;
}

void *memcpy(void *dest, const void *src, uint64_t n) {
    unsigned char *d = (unsigned char *)dest;
    const unsigned char *s = (const unsigned char *)src;
    while (n--) *d++ = *s++;
    return dest;
}

// --- Bare-Metal x86 I/O Port Instructions ---
static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ( "outb %0, %1" : : "a"(val), "Nd"(port) );
}
static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ( "inb %1, %0" : "=a"(ret) : "Nd"(port) );
    return ret;
}

// --- PS/2 Mouse & Keyboard ---
void ps2_wait(uint8_t type) {
    uint32_t timeout = 100000;
    if (type == 0) {
        while (timeout--) { if ((inb(0x64) & 1) == 1) return; }
    } else {
        while (timeout--) { if ((inb(0x64) & 2) == 0) return; }
    }
}

void ps2_write(uint8_t data) {
    ps2_wait(1); outb(0x64, 0xD4);
    ps2_wait(1); outb(0x60, data);
}

uint8_t ps2_read() {
    ps2_wait(0); return inb(0x60);
}

void init_input() {
    ps2_wait(1); outb(0x64, 0xA8); 
    ps2_wait(1); outb(0x64, 0x20); 
    ps2_wait(0); uint8_t status = inb(0x60) | 3; 
    ps2_wait(1); outb(0x64, 0x60); 
    ps2_wait(1); outb(0x60, status);
    ps2_write(0xF6); ps2_read(); 
    ps2_write(0xF4); ps2_read(); 
}

const char kbd_US[128] = {
    0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0,
    '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' '
};

const uint8_t font8x8[96][8] = {
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, {0x18,0x3C,0x3C,0x18,0x18,0x00,0x18,0x00}, 
    {0x66,0x66,0x24,0x00,0x00,0x00,0x00,0x00}, {0x6C,0x6C,0xFE,0x6C,0xFE,0x6C,0x6C,0x00}, 
    {0x18,0x3E,0x60,0x3C,0x06,0x7C,0x18,0x00}, {0x00,0xC6,0xCC,0x18,0x30,0x66,0xC6,0x00}, 
    {0x38,0x6C,0x6C,0x38,0x6D,0x66,0x3B,0x00}, {0x18,0x18,0x30,0x00,0x00,0x00,0x00,0x00}, 
    {0x0C,0x18,0x30,0x30,0x30,0x18,0x0C,0x00}, {0x30,0x18,0x0C,0x0C,0x0C,0x18,0x30,0x00}, 
    {0x00,0x18,0x7E,0x3C,0x7E,0x18,0x00,0x00}, {0x00,0x18,0x18,0x7E,0x18,0x18,0x00,0x00}, 
    {0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x30}, {0x00,0x00,0x00,0x7E,0x00,0x00,0x00,0x00}, 
    {0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x00}, {0x06,0x0C,0x18,0x30,0x60,0xC0,0x80,0x00}, 
    {0x3C,0x66,0x6E,0x76,0x66,0x66,0x3C,0x00}, {0x18,0x38,0x18,0x18,0x18,0x18,0x7E,0x00}, 
    {0x3C,0x66,0x06,0x1C,0x30,0x60,0x7E,0x00}, {0x3C,0x66,0x06,0x1C,0x06,0x66,0x3C,0x00}, 
    {0x1C,0x3C,0x6C,0xCC,0xFE,0x0C,0x0C,0x00}, {0x7E,0x60,0x7C,0x06,0x06,0x66,0x3C,0x00}, 
    {0x3C,0x66,0x60,0x7C,0x66,0x66,0x3C,0x00}, {0x7E,0x06,0x0C,0x18,0x30,0x30,0x30,0x00}, 
    {0x3C,0x66,0x66,0x3C,0x66,0x66,0x3C,0x00}, {0x3C,0x66,0x66,0x3E,0x06,0x66,0x3C,0x00}, 
    {0x00,0x18,0x18,0x00,0x00,0x18,0x18,0x00}, {0x00,0x18,0x18,0x00,0x00,0x18,0x18,0x30}, 
    {0x06,0x0C,0x18,0x30,0x18,0x0C,0x06,0x00}, {0x00,0x00,0x7E,0x00,0x7E,0x00,0x00,0x00}, 
    {0x60,0x30,0x18,0x0C,0x18,0x30,0x60,0x00}, {0x3C,0x66,0x0C,0x18,0x18,0x00,0x18,0x00}, 
    {0x3C,0x66,0x6E,0x6E,0x60,0x66,0x3C,0x00}, {0x18,0x3C,0x66,0x66,0x7E,0x66,0x66,0x00}, 
    {0x7C,0x66,0x66,0x7C,0x66,0x66,0x7C,0x00}, {0x3C,0x66,0x60,0x60,0x60,0x66,0x3C,0x00}, 
    {0x78,0x6C,0x66,0x66,0x66,0x6C,0x78,0x00}, {0x7E,0x60,0x60,0x78,0x60,0x60,0x7E,0x00}, 
    {0x7E,0x60,0x60,0x78,0x60,0x60,0x60,0x00}, {0x3C,0x66,0x60,0x6E,0x66,0x66,0x3C,0x00}, 
    {0x66,0x66,0x66,0x7E,0x66,0x66,0x66,0x00}, {0x3C,0x18,0x18,0x18,0x18,0x18,0x3C,0x00}, 
    {0x06,0x06,0x06,0x06,0x06,0x66,0x3C,0x00}, {0x66,0x6C,0x78,0x70,0x78,0x6C,0x66,0x00}, 
    {0x60,0x60,0x60,0x60,0x60,0x60,0x7E,0x00}, {0x63,0x77,0x7F,0x6B,0x63,0x63,0x63,0x00}, 
    {0x66,0x76,0x7E,0x7E,0x6E,0x66,0x66,0x00}, {0x3C,0x66,0x66,0x66,0x66,0x66,0x3C,0x00}, 
    {0x7C,0x66,0x66,0x7C,0x60,0x60,0x60,0x00}, {0x3C,0x66,0x66,0x66,0x6A,0x6C,0x36,0x00}, 
    {0x7C,0x66,0x66,0x7C,0x6C,0x66,0x66,0x00}, {0x3C,0x66,0x60,0x3C,0x06,0x66,0x3C,0x00}, 
    {0x7E,0x18,0x18,0x18,0x18,0x18,0x18,0x00}, {0x66,0x66,0x66,0x66,0x66,0x66,0x3C,0x00}, 
    {0x66,0x66,0x66,0x66,0x66,0x3C,0x18,0x00}, {0x63,0x63,0x63,0x6B,0x7F,0x77,0x63,0x00}, 
    {0x66,0x66,0x3C,0x18,0x3C,0x66,0x66,0x00}, {0x66,0x66,0x66,0x3C,0x18,0x18,0x18,0x00}, 
    {0x7E,0x06,0x0C,0x18,0x30,0x60,0x7E,0x00}, {0x3C,0x30,0x30,0x30,0x30,0x30,0x3C,0x00}, 
    {0x80,0xC0,0x60,0x30,0x18,0x0C,0x06,0x00}, {0x3C,0x0C,0x0C,0x0C,0x0C,0x0C,0x3C,0x00}, 
    {0x18,0x3C,0x66,0x00,0x00,0x00,0x00,0x00}, {0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0x00}, 
    {0x30,0x30,0x18,0x00,0x00,0x00,0x00,0x00}, {0x00,0x00,0x3C,0x06,0x3E,0x66,0x3E,0x00}, 
    {0x60,0x60,0x7C,0x66,0x66,0x66,0x7C,0x00}, {0x00,0x00,0x3C,0x60,0x60,0x66,0x3C,0x00}, 
    {0x06,0x06,0x3E,0x66,0x66,0x66,0x3E,0x00}, {0x00,0x00,0x3C,0x66,0x7E,0x60,0x3C,0x00}, 
    {0x1C,0x30,0x7C,0x30,0x30,0x30,0x30,0x00}, {0x00,0x00,0x3E,0x66,0x66,0x3E,0x06,0x3C}, 
    {0x60,0x60,0x7C,0x66,0x66,0x66,0x66,0x00}, {0x18,0x00,0x38,0x18,0x18,0x18,0x3C,0x00}, 
    {0x0C,0x00,0x1C,0x0C,0x0C,0x0C,0x0C,0x38}, {0x60,0x60,0x66,0x6C,0x78,0x6C,0x66,0x00}, 
    {0x38,0x18,0x18,0x18,0x18,0x18,0x3C,0x00}, {0x00,0x00,0x66,0x7F,0x7F,0x6B,0x63,0x00}, 
    {0x00,0x00,0x7C,0x66,0x66,0x66,0x66,0x00}, {0x00,0x00,0x3C,0x66,0x66,0x66,0x3C,0x00}, 
    {0x00,0x00,0x7C,0x66,0x66,0x7C,0x60,0x60}, {0x00,0x00,0x3E,0x66,0x66,0x3E,0x06,0x06}, 
    {0x00,0x00,0x7C,0x66,0x60,0x60,0x60,0x00}, {0x00,0x00,0x3E,0x60,0x3C,0x06,0x7C,0x00}, 
    {0x30,0x30,0x7C,0x30,0x30,0x34,0x18,0x00}, {0x00,0x00,0x66,0x66,0x66,0x66,0x3E,0x00}, 
    {0x00,0x00,0x66,0x66,0x66,0x3C,0x18,0x00}, {0x00,0x00,0x63,0x6B,0x7F,0x3E,0x36,0x00}, 
    {0x00,0x00,0x66,0x3C,0x18,0x3C,0x66,0x00}, {0x00,0x00,0x66,0x66,0x66,0x3E,0x06,0x3C}, 
    {0x00,0x00,0x7E,0x0C,0x18,0x30,0x7E,0x00}, {0x0E,0x18,0x18,0x70,0x18,0x18,0x0E,0x00}, 
    {0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18}, {0x70,0x18,0x18,0x0E,0x18,0x18,0x70,0x00}, 
    {0x3A,0x6C,0x00,0x00,0x00,0x00,0x00,0x00}, {0x00,0x00,0x18,0x18,0x18,0x18,0x18,0x00} 
};

enum OS_STATE { 
    STATE_DESKTOP=0, STATE_MENU=1, STATE_FILES=2, 
    STATE_WEB=3, STATE_TERM=4, STATE_SETTINGS=5,
    STATE_STORE=6, STATE_PROBE=7, STATE_SEARCH=8,
    STATE_POWEROFF=9
};

const char *CURSOR_BITMAP[22] = {
    "W            ", "WW           ", "WBW          ", "WBBW         ", "WBBBW        ", 
    "WBBBBW       ", "WBBBBBW      ", "WBBBBBBW     ", "WBBBBBBBW    ", "WBBBBBBBBW   ", 
    "WBBBBBBBBBW  ", "WBBBBBBBBBBW ", "WBBBBBBWWWWw ", "WBBBWBBW     ", "WBBW WBBW    ", 
    "WBW  WBBW    ", "WW   WBBW    ", "W     WBBW   ", "      WBBW   ", "      WBBW   ", 
    "       WW    ", "             "
};

const char *PEACH_ICON[20] = {
    "                    ", "         ss         ", "       llls         ", "      lllll         ",
    "       lll          ", "      pppppp        ", "    pppdpwwppp      ", "   ppppdpwwpppp     ",
    "  pppppdpwwppppp    ", " ppppppdpwwpppppp   ", " ppppppdppppppppp   ", " ppppppdppppppppp   ",
    " ppppppdppppppppp   ", "  pppppdpppppppp    ", "   ppppdppppppp     ", "    pppdpppppp      ",
    "      pppppp        ", "                    ", "                    ", "                    "
};

const char *ICON_USER[16] = {
    "                ", "      WWWW      ", "     WWWWWW     ", "     WWWWWW     ",
    "      WWWW      ", "                ", "    WWWWWWWW    ", "   WWWWWWWWWW   ",
    "  WWWWWWWWWWWW  ", " WWWWWWWWWWWWWW ", " WWWWWWWWWWWWWW ", "                ",
    "                ", "                ", "                ", "                "
};
const char *ICON_POWER[16] = {
    "                ", "       WW       ", "       WW       ", "       WW       ",
    "    RR    RR    ", "   R        R   ", "  R          R  ", "  R          R  ",
    "  R          R  ", "   R        R   ", "    RR    RR    ", "      RRRR      ",
    "                ", "                ", "                ", "                "
};
const char *ICON_FILES[16] = {
    "                ", "  YYYYY         ", " YyyyyyY        ", "YyyyyyyyYYYYYY  ",
    "YyyyyyyyyyyyyyY ", "YyyyyyyyyyyyyyY ", "YyyyyyyyyyyyyyY ", "YyyyyyyyyyyyyyY ",
    "YyyyyyyyyyyyyyY ", "YyyyyyyyyyyyyyY ", "YyyyyyyyyyyyyyY ", " YYYYYYYYYYYYY  ",
    "                ", "                ", "                ", "                "
};
const char *ICON_WEB[16] = {
    "      BBBB      ", "    BBWWWWBB    ", "   BWWBBBBWWB   ", "  BWBBWBBWBBWB  ",
    "  BWBBWBBWBBWB  ", " BWWBBWBBWBBWWB ", " BWWBBWBBWBBWWB ", " BBBBBBBBBBBBBB ",
    " BWWBBWBBWBBWWB ", " BWWBBWBBWBBWWB ", "  BWBBWBBWBBWB  ", "  BWBBWBBWBBWB  ",
    "   BWWBBBBWWB   ", "    BBWWWWBB    ", "      BBBB      ", "                "
};
const char *ICON_TERM[16] = {
    "                ", " GGGGGGGGGGGGGG ", " G            G ", " G WW         G ",
    " G   WW       G ", " G WW         G ", " G      WWWW  G ", " G            G ",
    " G            G ", " G            G ", " G            G ", " G            G ",
    " GGGGGGGGGGGGGG ", "                ", "                ", "                "
};
const char *ICON_SETTINGS[16] = {
    "      GGGG      ", "    GGGGGGGG    ", "   GG      GG   ", "  GG GGGGGG GG  ",
    "  GG GG  GG GG  ", " GGG GG  GG GGG ", " GGG GGGGGG GGG ", " GGG        GGG ",
    " GGG GGGGGG GGG ", " GGG GG  GG GGG ", "  GG GG  GG GG  ", "  GG GGGGGG GG  ",
    "   GG      GG   ", "    GGGGGGGG    ", "      GGGG      ", "                "
};
const char *ICON_STORE[16] = {
    "                ", "      WWWW      ", "     W    W     ", "   WWWWWWWWWW   ",
    "   WBBBBBBBBW   ", "   WBBBBBBBBW   ", "   WBBBBBBBBW   ", "   WBBBBBBBBW   ",
    "   WBBBBBBBBW   ", "   WBBBBBBBBW   ", "   WWWWWWWWWW   ", "                ",
    "                ", "                ", "                ", "                "
};
const char *ICON_PROBE[16] = {
    "                ", "   W  W  W  W   ", "  WWWWWWWWWWWW  ", "  WGGGGGGGGGGW  ",
    " WWGGGGGGGGGGWW ", "  WGGGGGGGGGGW  ", " WWGGGGGGGGGGWW ", "  WGGGGGGGGGGW  ",
    " WWGGGGGGGGGGWW ", "  WGGGGGGGGGGW  ", "  WWWWWWWWWWWW  ", "   W  W  W  W   ",
    "                ", "                ", "                ", "                "
};

const char *ICON_FILE_DOC[16] = {
    "                ", "   WWWWWWWWW    ", "   W       WW   ", "   W       W W  ",
    "   W       WWWW ", "   W          W ", "   W  BBBBBB  W ", "   W          W ",
    "   W  BBBBBB  W ", "   W          W ", "   W  BBBBBB  W ", "   W          W ",
    "   W          W ", "   WWWWWWWWWWWW ", "                ", "                "
};

const char *ICON_WIFI[16] = {
    "                ", "    WWWWWWWW    ", "  WW        WW  ", " W   WWWWWW   W ",
    "    W      W    ", "   W   WW   W   ", "      W  W      ", "     W    W     ",
    "                ", "       WW       ", "       WW       ", "                ",
    "                ", "                ", "                ", "                "
};
const char *ICON_BT[16] = {
    "                ", "       W        ", "       WW       ", "       W W      ",
    "    W  W  W     ", "     W W W      ", "      WWW       ", "      W W       ",
    "      WWW       ", "     W W W      ", "    W  W  W     ", "       W W      ",
    "       WW       ", "       W        ", "                ", "                "
};
const char *ICON_VOL[16] = {
    "                ", "                ", "        W       ", "       WW  W    ",
    "      WWW   W   ", "  WWWWWWW   W   ", "  WWWWWWW    W  ", "  WWWWWWW    W  ",
    "  WWWWWWW    W  ", "  WWWWWWW   W   ", "      WWW   W   ", "       WW  W    ",
    "        W       ", "                ", "                ", "                "
};
const char *ICON_BATT[16] = {
    "                ", "                ", "                ", "                ",
    "  WWWWWWWWWWWW  ", "  WGGGGGGGGGGWW ", "  WGGGGGGGGGGW W", "  WGGGGGGGGGGW W",
    "  WGGGGGGGGGGW W", "  WGGGGGGGGGGWW ", "  WWWWWWWWWWWW  ", "                ",
    "                ", "                ", "                ", "                "
};

void draw_rect(uint32_t *fb, uint32_t pps, int start_x, int start_y, int width, int height, uint32_t color) {
    for (int y = start_y; y < start_y + height; y++) {
        for (int x = start_x; x < start_x + width; x++) fb[y * pps + x] = color;
    }
}

void draw_string(uint32_t *fb, uint32_t pps, int start_x, int start_y, const char *str, uint32_t fg, int scale) {
    int cur_x = start_x;
    while (*str) {
        if (*str >= 32 && *str <= 127) {
            const uint8_t *glyph = font8x8[(*str) - 32];
            for (int y = 0; y < 8 * scale; y++) {
                for (int x = 0; x < 8 * scale; x++) {
                    if (glyph[y / scale] & (1 << (7 - (x / scale)))) {
                        fb[(start_y + y) * pps + (cur_x + x)] = fg;
                    }
                }
            }
        }
        cur_x += 8 * scale;
        str++;
    }
}

void draw_icon(uint32_t *fb, uint32_t pps, int start_x, int start_y, const char **icon, int size, int scale) {
    for (int y = 0; y < size * scale; y++) {
        for (int x = 0; x < size * scale; x++) {
            char pixel = icon[y / scale][x / scale];
            uint32_t color = 0;
            if (pixel == 'Y') color = 0x00FFD700; else if (pixel == 'y') color = 0x00DAA520;
            else if (pixel == 'B') color = 0x004285F4; else if (pixel == 'W' || pixel == 'w') color = 0x00FFFFFF;
            else if (pixel == 'G') color = 0x009AA0A6; else if (pixel == 'R') color = 0x00EA4335;
            else if (pixel == 'p') color = 0x00FFB347; else if (pixel == 'd') color = 0x00FF7F50; 
            else if (pixel == 'l') color = 0x00228B22; else if (pixel == 's') color = 0x008B4513; 
            
            if (pixel != ' ') fb[(start_y + y) * pps + (start_x + x)] = color;
        }
    }
}

void draw_cursor(uint32_t *fb, uint32_t pps, int start_x, int start_y, uint32_t screen_w, uint32_t screen_h) {
    for (int y = 0; y < 22; y++) {
        for (int x = 0; x < 13; x++) {
            if (start_x + x >= screen_w || start_y + y >= screen_h) continue; 
            char pixel = CURSOR_BITMAP[y][x];
            if (pixel == 'W' || pixel == 'w') fb[(start_y + y) * pps + (start_x + x)] = 0x00FFFFFF;
            else if (pixel == 'B') fb[(start_y + y) * pps + (start_x + x)] = 0x00000000;
        }
    }
}

const char *app_names[6] = {"Files", "Web", "Terminal", "Settings", "Store", "System Probe"};
const char **app_icons[6] = {ICON_FILES, ICON_WEB, ICON_TERM, ICON_SETTINGS, ICON_STORE, ICON_PROBE};
int app_states[6] = {STATE_FILES, STATE_WEB, STATE_TERM, STATE_SETTINGS, STATE_STORE, STATE_PROBE};

// --- Main OS Entry ---
EFI_STATUS efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    EFI_GRAPHICS_OUTPUT_PROTOCOL *gop;
    SystemTable->BootServices->LocateProtocol(&gopGuid, 0, (void **)&gop);

    init_input(); 

    uint32_t width = gop->Mode->Info->HorizontalResolution;
    uint32_t height = gop->Mode->Info->VerticalResolution;
    uint32_t pps = gop->Mode->Info->PixelsPerScanLine;
    uint32_t *screen = (uint32_t *)gop->Mode->FrameBufferBase;

    uint32_t *backbuffer;
    SystemTable->BootServices->AllocatePool(EFI_LOADER_DATA, height * pps * 4, (void **)&backbuffer);

    int cursor_x = width / 2, cursor_y = height / 2;
    int old_cursor_x = cursor_x, old_cursor_y = cursor_y;
    
    enum OS_STATE current_state = STATE_DESKTOP;
    bool ui_needs_redraw = true; 
    bool mouse_moved = true;     
    bool last_left = false;
    uint8_t mouse_cycle = 0, mouse_byte[3];

    char search_buffer[40];
    memset(search_buffer, 0, sizeof(search_buffer));
    int search_len = 0;

    int menu_w = 340, menu_h = 350; 
    int menu_x = 10, menu_y = height - 40 - menu_h - 10;
    
    int card_w = 80, card_h = 80;
    int pad_x = 25, pad_y = 20;
    int grid_start_x = menu_x + pad_x;
    int grid_start_y = menu_y + 80;
    
    int hovered_app = -1;
    int hover_tt_x = 0;
    int hover_tt_y = 0;

    // Files App Layout Variables
    int win_w = 640, win_h = 360;
    int win_x = (width - win_w) / 2;
    int win_y = (height - win_h) / 2;

    while (current_state != STATE_POWEROFF) {
        
        if (inb(0x64) & 1) { 
            uint8_t status = inb(0x64);
            uint8_t data = inb(0x60); 
            
            if (status & 0x20) { 
                if (mouse_cycle == 0 && (data & 0x08)) { 
                    mouse_byte[0] = data; mouse_cycle++;
                } else if (mouse_cycle == 1) {
                    mouse_byte[1] = data; mouse_cycle++;
                } else if (mouse_cycle == 2) {
                    mouse_byte[2] = data; mouse_cycle = 0; 
                    
                    int32_t rel_x = mouse_byte[1] - ((mouse_byte[0] << 4) & 0x100);
                    int32_t rel_y = mouse_byte[2] - ((mouse_byte[0] << 3) & 0x100);
                    
                    if (rel_x != 0 || rel_y != 0) {
                        cursor_x += rel_x; cursor_y -= rel_y; 
                        if (cursor_x < 0) cursor_x = 0;
                        if (cursor_x > (int)width - 1) cursor_x = width - 1;
                        if (cursor_y < 0) cursor_y = 0;
                        if (cursor_y > (int)height - 1) cursor_y = height - 1;
                        mouse_moved = true;
                    }

                    // --- Application Hover Logic ---
                    int new_hovered = -1;
                    if (current_state == STATE_MENU || current_state == STATE_SEARCH) {
                        if (cursor_x >= menu_x && cursor_x <= menu_x + menu_w && cursor_y >= menu_y && cursor_y <= menu_y + menu_h) {
                            int col = 0, row = 0;
                            for (int i = 0; i < 6; i++) {
                                bool match = true;
                                for (int j = 0; j < search_len; j++) {
                                    char c1 = app_names[i][j]; char c2 = search_buffer[j];
                                    if (c1 >= 'A' && c1 <= 'Z') c1 += 32;
                                    if (c2 >= 'A' && c2 <= 'Z') c2 += 32;
                                    if (c1 != c2 || c1 == 0) { match = false; break; }
                                }
                                if (match) {
                                    int cx = grid_start_x + col * (card_w + pad_x);
                                    int cy = grid_start_y + row * (card_h + pad_y);
                                    
                                    if (cursor_x >= cx && cursor_x <= cx + card_w && cursor_y >= cy && cursor_y <= cy + card_h) {
                                        new_hovered = i;
                                        hover_tt_x = cx + card_w / 2;
                                        hover_tt_y = cy;
                                        break;
                                    }
                                    col++;
                                    if (col > 2) { col = 0; row++; }
                                }
                            }
                        }
                    }
                    if (new_hovered != hovered_app) {
                        hovered_app = new_hovered;
                        ui_needs_redraw = true;
                    }

                    // --- Click Processing ---
                    bool current_left = mouse_byte[0] & 0x01;
                    bool clicked = (last_left && !current_left);
                    last_left = current_left;

                    if (clicked) {
                        if (current_state == STATE_FILES) {
                            if (cursor_x >= win_x + win_w - 30 && cursor_x <= win_x + win_w - 10 && 
                                cursor_y >= win_y + 5 && cursor_y <= win_y + 25) {
                                current_state = STATE_DESKTOP;
                                ui_needs_redraw = true;
                                continue;
                            }
                        }

                        if (cursor_x >= 10 && cursor_x <= 35 && cursor_y >= (int)height - 35) {
                            if (current_state == STATE_MENU || current_state == STATE_SEARCH) {
                                current_state = STATE_DESKTOP;
                                hovered_app = -1;
                            } else {
                                current_state = STATE_MENU;
                                search_len = 0;
                                memset(search_buffer, 0, sizeof(search_buffer));
                            }
                            ui_needs_redraw = true;
                        } 
                        else if (current_state == STATE_MENU || current_state == STATE_SEARCH) {
                            if (cursor_x >= menu_x && cursor_x <= menu_x + menu_w && cursor_y >= menu_y && cursor_y <= menu_y + menu_h) {
                                
                                if (cursor_y >= menu_y + 20 && cursor_y <= menu_y + 60 && cursor_x >= menu_x + 20 && cursor_x <= menu_x + menu_w - 20) {
                                    current_state = STATE_SEARCH; ui_needs_redraw = true;
                                }
                                else if (cursor_y >= menu_y + menu_h - 60 && cursor_x >= menu_x + menu_w - 60) {
                                    current_state = STATE_POWEROFF; ui_needs_redraw = true;
                                } 
                                else if (hovered_app != -1) {
                                    current_state = app_states[hovered_app];
                                    hovered_app = -1;
                                    ui_needs_redraw = true;
                                }
                            } else {
                                current_state = STATE_DESKTOP;
                                hovered_app = -1;
                                ui_needs_redraw = true;
                            }
                        }
                    }
                }
            } 
            else if (data < 128) { 
                char c = kbd_US[data];
                if (c != 0 && (current_state == STATE_MENU || current_state == STATE_SEARCH)) {
                    current_state = STATE_SEARCH;
                    if (c == '\b') {
                        if (search_len > 0) search_buffer[--search_len] = '\0';
                    } else if (search_len < 30) {
                        search_buffer[search_len++] = c;
                        search_buffer[search_len] = '\0';
                    }
                    ui_needs_redraw = true;
                }
            }
        }

        if (ui_needs_redraw) {
            draw_rect(backbuffer, pps, 0, 0, width, height, 0x0087CEEB); 
            draw_rect(backbuffer, pps, 0, height/2, width, height/2, 0x00696969); 

            if (current_state == STATE_FILES) {
                // Main Window Shell
                draw_rect(backbuffer, pps, win_x, win_y, win_w, win_h, 0x00FFFFFF);
                draw_rect(backbuffer, pps, win_x, win_y, win_w, 1, 0x00D0D0D0);
                draw_rect(backbuffer, pps, win_x, win_y + win_h - 1, win_w, 1, 0x00D0D0D0);
                draw_rect(backbuffer, pps, win_x, win_y, 1, win_h, 0x00D0D0D0);
                draw_rect(backbuffer, pps, win_x + win_w - 1, win_y, 1, win_h, 0x00D0D0D0);

                // Titlebar
                draw_rect(backbuffer, pps, win_x + 1, win_y + 1, win_w - 2, 30, 0x00F0F0F0);
                draw_string(backbuffer, pps, win_x + 12, win_y + 11, "Files", 0x00333333, 1);

                // Close Button
                draw_rect(backbuffer, pps, win_x + win_w - 30, win_y + 5, 20, 20, 0x00EA4335); 
                draw_string(backbuffer, pps, win_x + win_w - 24, win_y + 11, "X", 0x00FFFFFF, 1);

                // Files and Folders Only (Grid Layout)
                const char* item_names[] = {"System", "Users", "Documents", "Downloads", "Music", "Pictures", "readme.txt", "notes.txt"};
                int item_types[] = {1, 1, 1, 1, 1, 1, 0, 0}; // 1 = folder, 0 = file

                int start_grid_x = win_x + 40;
                int start_grid_y = win_y + 60;
                int col_width = 135;
                int row_height = 80;

                for (int i = 0; i < 8; i++) {
                    int col = i % 4;
                    int row = i / 4;
                    int cx = start_grid_x + (col * col_width);
                    int cy = start_grid_y + (row * row_height);

                    if (item_types[i] == 1) {
                        draw_icon(backbuffer, pps, cx + 12, cy, ICON_FILES, 16, 2);
                    } else {
                        draw_icon(backbuffer, pps, cx + 12, cy, ICON_FILE_DOC, 16, 2);
                    }
                    draw_string(backbuffer, pps, cx, cy + 40, item_names[i], 0x00222222, 1);
                }

            } else if (current_state == STATE_WEB) {
                draw_rect(backbuffer, pps, 50, 50, width-100, height-150, 0x00FFFFFF); 
                draw_rect(backbuffer, pps, 50, 50, width-100, 40, 0x004285F4); 
            } else if (current_state == STATE_TERM) {
                draw_rect(backbuffer, pps, width/4, height/4, width/2, height/2, 0x00111111); 
                draw_rect(backbuffer, pps, width/4, height/4, width/2, 40, 0x00333333); 
            } else if (current_state == STATE_SETTINGS) {
                draw_rect(backbuffer, pps, width/4, height/4, width/2, height/2, 0x00202124); 
                draw_rect(backbuffer, pps, width/4, height/4, width/2, 40, 0x00111111); 
            } else if (current_state == STATE_STORE) {
                draw_rect(backbuffer, pps, width/4, height/4, width/2, height/2, 0x0000BFFF); 
                draw_rect(backbuffer, pps, width/4, height/4, width/2, 40, 0x00008B8B); 
            } else if (current_state == STATE_PROBE) {
                draw_rect(backbuffer, pps, width/4, height/4, width/2, height/2, 0x0032CD32); 
                draw_rect(backbuffer, pps, width/4, height/4, width/2, 40, 0x00228B22); 
            } 

            if (current_state == STATE_MENU || current_state == STATE_SEARCH) {
                draw_rect(backbuffer, pps, menu_x, menu_y, menu_w, menu_h, 0x00202124); 
                
                char display_search[45];
                int slen = 0;
                while (search_buffer[slen] && slen < 40) {
                    display_search[slen] = search_buffer[slen];
                    slen++;
                }
                display_search[slen] = 127; 
                display_search[slen + 1] = '\0';

                draw_rect(backbuffer, pps, menu_x + 20, menu_y + 20, menu_w - 40, 40, 0x00303134);
                if (search_len == 0) draw_string(backbuffer, pps, menu_x + 35, menu_y + 36, "Search...\x7f", 0x009AA0A6, 2);
                else draw_string(backbuffer, pps, menu_x + 35, menu_y + 36, display_search, 0x00FFFFFF, 2);

                int col = 0, row = 0;
                for (int i = 0; i < 6; i++) {
                    bool match = true;
                    for (int j = 0; j < search_len; j++) {
                        char c1 = app_names[i][j]; char c2 = search_buffer[j];
                        if (c1 >= 'A' && c1 <= 'Z') c1 += 32;
                        if (c2 >= 'A' && c2 <= 'Z') c2 += 32;
                        if (c1 != c2 || c1 == 0) { match = false; break; }
                    }
                    if (match) {
                        int cx = grid_start_x + col * (card_w + pad_x);
                        int cy = grid_start_y + row * (card_h + pad_y);
                        
                        draw_rect(backbuffer, pps, cx, cy, card_w, card_h, 0x00303134); 
                        draw_icon(backbuffer, pps, cx + 24, cy + 24, app_icons[i], 16, 2);
                        
                        col++;
                        if (col > 2) { col = 0; row++; }
                    }
                }

                if (hovered_app != -1) {
                    int len = 0; while (app_names[hovered_app][len]) len++;
                    int tt_w = len * 8 + 10;
                    int tt_h = 24;
                    int tt_x = hover_tt_x - tt_w / 2;
                    int tt_y = hover_tt_y - tt_h - 10;

                    draw_rect(backbuffer, pps, tt_x, tt_y, tt_w, tt_h, 0x00E8EAED);
                    draw_rect(backbuffer, pps, tt_x, tt_y, tt_w, 1, 0x00BDC1C6); 
                    draw_rect(backbuffer, pps, tt_x, tt_y + tt_h - 1, tt_w, 1, 0x00BDC1C6);
                    draw_rect(backbuffer, pps, tt_x, tt_y, 1, tt_h, 0x00BDC1C6); 
                    draw_rect(backbuffer, pps, tt_x + tt_w - 1, tt_y, 1, tt_h, 0x00BDC1C6);
                    
                    draw_string(backbuffer, pps, tt_x + 5, tt_y + 8, app_names[hovered_app], 0x00202124, 1);
                }

                draw_rect(backbuffer, pps, menu_x, menu_y + menu_h - 60, menu_w, 60, 0x00171717); 
                draw_icon(backbuffer, pps, menu_x + 20, menu_y + menu_h - 40, ICON_USER, 16, 1);
                draw_icon(backbuffer, pps, menu_x + menu_w - 40, menu_y + menu_h - 40, ICON_POWER, 16, 1);
            }

            draw_rect(backbuffer, pps, 0, height - 40, width, 40, 0x00222222);
            draw_icon(backbuffer, pps, 10, height - 30, (const char**)PEACH_ICON, 20, 1);

            int tray_y = height - 28; 
            draw_icon(backbuffer, pps, width - 126, tray_y, ICON_BT, 16, 1);
            draw_icon(backbuffer, pps, width - 96, tray_y, ICON_WIFI, 16, 1);
            draw_icon(backbuffer, pps, width - 66, tray_y, ICON_VOL, 16, 1);
            draw_icon(backbuffer, pps, width - 36, tray_y, ICON_BATT, 16, 1);

            for (uint32_t i = 0; i < height * pps; i++) screen[i] = backbuffer[i];

            ui_needs_redraw = false;
            mouse_moved = true; 
        }

        if (mouse_moved) {
            for (int y = 0; y < 22; y++) {
                for (int x = 0; x < 13; x++) {
                    if (old_cursor_x + x < width && old_cursor_y + y < height) {
                        uint32_t target_idx = (old_cursor_y + y) * pps + (old_cursor_x + x);
                        screen[target_idx] = backbuffer[target_idx]; 
                    }
                }
            }
            draw_cursor(screen, pps, cursor_x, cursor_y, width, height);
            old_cursor_x = cursor_x;
            old_cursor_y = cursor_y;
            mouse_moved = false;
        }
    }

    draw_rect(screen, pps, 0, 0, width, height, 0x00000000); 
    while (1) __asm__ volatile("hlt");

    return EFI_SUCCESS;
}