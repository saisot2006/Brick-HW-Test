
#include <alsa/asoundlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

static int probe_one(snd_pcm_format_t fmt, unsigned int rate) {
    pid_t pid = fork();
    if (pid < 0) return -2;

    if (pid == 0) {
        snd_pcm_t *pcm = NULL;
        snd_pcm_hw_params_t *p = NULL;
        int rc = snd_pcm_open(&pcm, "hw:0,0", SND_PCM_STREAM_PLAYBACK,
                              SND_PCM_NONBLOCK);
        if (rc < 0) _exit(1);

        rc = snd_pcm_hw_params_malloc(&p);
        if (rc < 0 || !p) {
            snd_pcm_close(pcm);
            _exit(1);
        }

        rc = snd_pcm_hw_params_any(pcm, p);
        if (rc >= 0)
            rc = snd_pcm_hw_params_test_format(pcm, p, fmt);
        if (rc >= 0)
            rc = snd_pcm_hw_params_test_rate(pcm, p, rate, 0);

        snd_pcm_hw_params_free(p);
        snd_pcm_close(pcm);
        _exit(rc >= 0 ? 0 : 1);
    }

    /* Never let a broken driver take down the main process. */
    for (int i = 0; i < 60; ++i) {
        int status = 0;
        pid_t r = waitpid(pid, &status, WNOHANG);
        if (r == pid)
            return (WIFEXITED(status) && WEXITSTATUS(status) == 0) ? 1 : 0;
        if (r < 0)
            return -2;
        usleep(20000);
    }

    kill(pid, SIGKILL);
    waitpid(pid, NULL, 0);
    return -2;
}

static const char *fmt_name(snd_pcm_format_t f) {
    switch (f) {
        case SND_PCM_FORMAT_S16_LE:   return "S16_LE";
        case SND_PCM_FORMAT_S24_LE:   return "S24_LE";
        case SND_PCM_FORMAT_S24_3LE:  return "S24_3LE";
        default:                      return "UNKNOWN";
    }
}

static const char *result_name(int r) {
    return r == 1 ? "PASS" : (r == 0 ? "FAIL" : "TIMEOUT");
}

int main(void) {
    const snd_pcm_format_t formats[] = {
        SND_PCM_FORMAT_S16_LE,
        SND_PCM_FORMAT_S24_LE,
        SND_PCM_FORMAT_S24_3LE
    };
    const unsigned rates[] = {44100, 48000, 96000, 192000};

    FILE *out = fopen("Brick_HW_Test_direct.txt", "w");
    if (!out)
        return 1;

    fprintf(out, "Brick Pro Direct ALSA PCM Capability Test\n");
    fprintf(out, "No audio data is played.\n");
    fprintf(out, "Device: hw:0,0\n\n");

    printf("Brick Pro Direct ALSA Test\n");
    printf("No audio data is played.\n\n");

    for (unsigned i = 0; i < sizeof(formats)/sizeof(formats[0]); ++i) {
        printf("%s:", fmt_name(formats[i]));
        fprintf(out, "%s:", fmt_name(formats[i]));

        for (unsigned j = 0; j < sizeof(rates)/sizeof(rates[0]); ++j) {
            int r = probe_one(formats[i], rates[j]);
            printf(" %u=%s", rates[j], result_name(r));
            fprintf(out, " %u=%s", rates[j], result_name(r));
        }
        printf("\n");
        fprintf(out, "\n");
    }

    fflush(out);
    fclose(out);
    return 0;
}
