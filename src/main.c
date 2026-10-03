
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define W 1024
#define H 768
#define MAXL 220
#define MAXLINES 34

static SDL_Window *win;
static SDL_Renderer *ren;
static TTF_Font *font;
static char lines[MAXLINES][MAXL];
static int nlines=0, running=1;

static void addline(const char *s){
    if(nlines>=MAXLINES) return;
    snprintf(lines[nlines],MAXL,"%s",s?s:"");
    nlines++;
}
static void add_cmd(const char *cmd, int maxout){
    FILE *p=popen(cmd,"r");
    if(!p){ addline("  command unavailable"); return; }
    char b[512]; int count=0;
    while(fgets(b,sizeof(b),p) && count<maxout){
        b[strcspn(b,"\r\n")]=0;
        if(b[0]) addline(b);
        count++;
    }
    pclose(p);
}
static void text(const char *s,int x,int y,int size,SDL_Color c){
    TTF_Font *f=font;
    /* bundled font is opened at one size; use same font for predictable behavior */
    (void)size;
    SDL_Surface *sf=TTF_RenderUTF8_Blended(f,s,c);
    if(!sf) return;
    SDL_Texture *tx=SDL_CreateTextureFromSurface(ren,sf);
    SDL_Rect d={x,y,sf->w,sf->h};
    SDL_RenderCopy(ren,tx,NULL,&d);
    SDL_DestroyTexture(tx); SDL_FreeSurface(sf);
}
static void collect(void){
    char b[256];
    addline("BRICK PRO HARDWARE TEST");
    addline("");
    FILE *f=fopen("/proc/device-tree/model","r");
    if(f){ size_t k=fread(b,1,sizeof(b)-1,f); fclose(f); b[k]=0; for(size_t i=0;i<k;i++) if(b[i]==0)b[i]=' '; snprintf(b,sizeof(b),"Model: %s",b); addline(b); }
    snprintf(b,sizeof(b),"Kernel: %s",strstr("", "") ? "" : "see log");
    addline(b);
    add_cmd("uname -m",1);
    add_cmd("grep -E 'MemTotal|MemAvailable' /proc/meminfo | head -2",2);
    addline("");
    addline("AUDIO / ALSA");
    add_cmd("cat /proc/asound/cards 2>/dev/null",4);
    add_cmd("cat /proc/asound/pcm 2>/dev/null",8);
    addline("");
    addline("HW CAPABILITY (from ALSA/aplay)");
    add_cmd("aplay -l 2>/dev/null | head -12",12);
    add_cmd("aplay -D default --dump-hw-params /dev/zero 2>/dev/null | grep -E 'FORMAT|RATE|CHANNELS' | head -12",12);
    addline("");
    addline("Full raw report: .userdata/tg5040/logs/Brick_HW_Test_raw.txt");
    addline("B = exit");
}
int main(void){
    if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_JOYSTICK)!=0) return 1;
    if(TTF_Init()!=0){SDL_Quit();return 1;}
    win=SDL_CreateWindow("Brick HW Test",SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,W,H,SDL_WINDOW_SHOWN);
    if(!win){TTF_Quit();SDL_Quit();return 1;}
    ren=SDL_CreateRenderer(win,-1,SDL_RENDERER_ACCELERATED);
    if(!ren) ren=SDL_CreateRenderer(win,-1,SDL_RENDERER_SOFTWARE);
    font=TTF_OpenFont("./resource/font.ttf",24);
    if(!font){ if(ren)SDL_DestroyRenderer(ren); SDL_DestroyWindow(win); TTF_Quit(); SDL_Quit(); return 1; }
    if(SDL_NumJoysticks()>0) SDL_JoystickOpen(0);
    collect();
    SDL_Color fg={236,234,229,255}, dim={150,150,150,255}, accent={232,227,64,255};
    SDL_Event e;
    while(running){
        while(SDL_PollEvent(&e)){
            if(e.type==SDL_QUIT) running=0;
            else if(e.type==SDL_KEYDOWN && !e.key.repeat && (e.key.keysym.scancode==SDL_SCANCODE_ESCAPE || e.key.keysym.scancode==SDL_SCANCODE_LALT)) running=0;
            else if(e.type==SDL_JOYBUTTONDOWN && e.jbutton.button==0) running=0; /* B, same mapping as working Brick HW Test */
        }
        SDL_SetRenderDrawColor(ren,10,10,10,255); SDL_RenderClear(ren);
        text("BRICK HW TEST",24,18,24,accent);
        int y=64;
        for(int i=0;i<nlines;i++){
            SDL_Color c=(i==0)?accent:fg;
            if(strstr(lines[i],"Full raw") || strstr(lines[i],"B =")) c=dim;
            text(lines[i],24,y,24,c); y+=21;
            if(y>H-28) break;
        }
        SDL_RenderPresent(ren);
        SDL_Delay(30);
    }
    TTF_CloseFont(font); SDL_DestroyRenderer(ren); SDL_DestroyWindow(win);
    TTF_Quit(); SDL_Quit(); return 0;
}
