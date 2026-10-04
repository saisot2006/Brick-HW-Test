#include <SDL/SDL.h>
#include <alsa/asoundlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>

static int probe_one(snd_pcm_format_t fmt, unsigned int rate) {
    pid_t pid = fork();
    if (pid < 0) return -2;
    if (pid == 0) {
        snd_pcm_t *pcm = NULL;
        snd_pcm_hw_params_t *p = NULL;
        int rc = snd_pcm_open(&pcm, "hw:0,0", SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK);
        if (rc < 0) _exit(1);
        snd_pcm_hw_params_malloc(&p);
        if (!p) { snd_pcm_close(pcm); _exit(1); }
        rc = snd_pcm_hw_params_any(pcm, p);
        if (rc >= 0) rc = snd_pcm_hw_params_test_format(pcm, p, fmt);
        if (rc >= 0) rc = snd_pcm_hw_params_test_rate(pcm, p, rate, 0);
        snd_pcm_hw_params_free(p);
        snd_pcm_close(pcm);
        _exit(rc >= 0 ? 0 : 1);
    }

    const int timeout_ms = 1200;
    int elapsed = 0, status = 0;
    while (elapsed < timeout_ms) {
        pid_t r = waitpid(pid, &status, WNOHANG);
        if (r == pid)
            return WIFEXITED(status) && WEXITSTATUS(status) == 0 ? 1 : 0;
        if (r < 0) return -2;
        usleep(20000);
        elapsed += 20;
    }
    kill(pid, SIGKILL);
    waitpid(pid, &status, 0);
    return -2;
}

static const char *fmt_name(snd_pcm_format_t f) {
    switch (f) {
        case SND_PCM_FORMAT_S16_LE: return "S16_LE";
        case SND_PCM_FORMAT_S24_LE: return "S24_LE";
        case SND_PCM_FORMAT_S24_3LE: return "S24_3LE";
        default: return "?";
    }
}

int main(void) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0) return 1;
    SDL_Surface *screen = SDL_SetVideoMode(640, 480, 16, SDL_SWSURFACE);
    if (!screen) { SDL_Quit(); return 1; }

    snd_pcm_format_t fmts[] = {
        SND_PCM_FORMAT_S16_LE,
        SND_PCM_FORMAT_S24_LE,
        SND_PCM_FORMAT_S24_3LE
    };
    unsigned rates[] = {44100, 48000, 96000, 192000};
    int result[3][4] = {{0}};

    for (int i=0;i<3;i++)
        for (int j=0;j<4;j++)
            result[i][j] = probe_one(fmts[i], rates[j]);

    FILE *log = fopen("Brick_HW_Test_direct.txt", "w");
    if (log) {
        fprintf(log, "Direct ALSA PCM capability probe\n");
        fprintf(log, "No audio data played\n\n");
        for (int i=0;i<3;i++) {
            fprintf(log, "%s:", fmt_name(fmts[i]));
            for (int j=0;j<4;j++)
                fprintf(log, " %u=%s", rates[j],
                        result[i][j] == 1 ? "PASS" :
                        result[i][j] == 0 ? "FAIL" : "TIMEOUT");
            fprintf(log, "\n");
        }
        fclose(log);
    }

    /* Keep UI intentionally simple: results are also logged. */
    SDL_Color white = {255,255,255,0};
    SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));
    SDL_Flip(screen);

    int running = 1;
    while (running) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) running = 0;
            if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_ESCAPE ||
                    e.key.keysym.sym == SDLK_b ||
                    e.key.keysym.sym == SDLK_m) running = 0;
            }
        }
        SDL_Delay(20);
    }
    (void)white;
    SDL_Quit();
    return 0;
}
