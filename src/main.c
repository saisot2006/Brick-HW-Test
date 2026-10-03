#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define W 1024
#define H 768
#define MAXL 180
#define MAXLINES 32

static SDL_Window *win;
static SDL_Renderer *ren;
static TTF_Font *font;
static char lines[MAXLINES][MAXL];
static int nlines = 0;
static int running = 1;

static void addline(const char *s) {
    if (nlines >= MAXLINES) return;
    snprintf(lines[nlines], MAXL, "%s", s ? s : "");
    nlines++;
}

static void add_file(const char *title, const char *path, int maxlines) {
    FILE *f = fopen(path, "r");
    char b[256];
    int n = 0;
    if (!f) return;
    if (title) addline(title);
    while (n < maxlines && fgets(b, sizeof(b), f)) {
        b[strcspn(b, "\r\n")] = 0;
        if (b[0]) addline(b);
        n++;
    }
    fclose(f);
}

static void collect_safe(void) {
    char b[256];
    FILE *f;

    addline("BRICK PRO HARDWARE TEST");
    addline("SAFE AUDIO MODE - no aplay tests");
    addline("");

    f = fopen("/proc/device-tree/model", "r");
    if (f) {
        size_t k = fread(b, 1, sizeof(b)-1, f);
        fclose(f);
        b[k] = 0;
        for (size_t i = 0; i < k; ++i) if (b[i] == 0) b[i] = ' ';
        snprintf(b, sizeof(b), "Model: %s", b);
        addline(b);
    }

    add_file(NULL, "/proc/version", 1);

    addline("");
    addline("AUDIO / ALSA (read-only)");
    add_file(NULL, "/proc/asound/cards", 4);
    add_file(NULL, "/proc/asound/pcm", 8);
    addline("");
    addline("DAC / CODEC");
    add_file(NULL, "/proc/asound/card0/codec#0", 10);
    addline("");
    addline("Raw report: .userdata/tg5040/logs/Brick_HW_Test_raw.txt");
    addline("B / MENU = EXIT");
}

static void draw_text(const char *s, int x, int y, SDL_Color c) {
    SDL_Surface *sf = TTF_RenderUTF8_Blended(font, s, c);
    if (!sf) return;
    SDL_Texture *tx = SDL_CreateTextureFromSurface(ren, sf);
    if (tx) {
        SDL_Rect d = {x, y, sf->w, sf->h};
        SDL_RenderCopy(ren, tx, NULL, &d);
        SDL_DestroyTexture(tx);
    }
    SDL_FreeSurface(sf);
}

int main(void) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) != 0) return 1;
    if (TTF_Init() != 0) { SDL_Quit(); return 1; }

    win = SDL_CreateWindow("Brick HW Test", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                          W, H, SDL_WINDOW_SHOWN);
    if (!win) { TTF_Quit(); SDL_Quit(); return 1; }

    ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
    if (!ren) ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_SOFTWARE);
    if (!ren) { SDL_DestroyWindow(win); TTF_Quit(); SDL_Quit(); return 1; }

    font = TTF_OpenFont("./resource/font.ttf", 24);
    if (!font) {
        SDL_DestroyRenderer(ren); SDL_DestroyWindow(win);
        TTF_Quit(); SDL_Quit(); return 1;
    }

    if (SDL_NumJoysticks() > 0) SDL_JoystickOpen(0);

    /* Build the report only from local, bounded file reads. No aplay/popen audio test. */
    collect_safe();

    SDL_Color fg = {236,234,229,255};
    SDL_Color dim = {150,150,150,255};
    SDL_Color accent = {232,227,64,255};
    SDL_Event e;

    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) running = 0;
            else if (e.type == SDL_KEYDOWN && !e.key.repeat &&
                     (e.key.keysym.scancode == SDL_SCANCODE_ESCAPE ||
                      e.key.keysym.scancode == SDL_SCANCODE_LALT ||
                      e.key.keysym.scancode == SDL_SCANCODE_RETURN)) running = 0;
            else if (e.type == SDL_JOYBUTTONDOWN) {
                /* Known Brick/NextUI mapping: button 0 is B. Also accept common Menu buttons. */
                if (e.jbutton.button == 0 || e.jbutton.button == 8 || e.jbutton.button == 9)
                    running = 0;
            }
        }

        SDL_SetRenderDrawColor(ren, 10, 10, 10, 255);
        SDL_RenderClear(ren);
        draw_text("BRICK HW TEST", 24, 18, accent);
        int y = 64;
        for (int i = 0; i < nlines; ++i) {
            SDL_Color c = (i == 0) ? accent : fg;
            if (strstr(lines[i], "Raw report") || strstr(lines[i], "B / MENU")) c = dim;
            draw_text(lines[i], 24, y, c);
            y += 22;
            if (y > H - 28) break;
        }
        SDL_RenderPresent(ren);
        SDL_Delay(20);
    }

    TTF_CloseFont(font);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    TTF_Quit();
    SDL_Quit();
    return 0;
}
