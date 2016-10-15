#include"chip8.h"

G(nop ){}
F(f00 ,memset(M->SCR,0,32*8); M->DF++)
F(f0e ,M->PC=M->STK[M->SP--%16])
G(f0  ){(inst[]){f00,f0e}[oc>>1&1](M,oc);}
F(f1  ,M->PC=nnn)
F(f2  ,M->STK[++M->SP%16]=M->PC; M->PC=nnn)
F(f3  ,if(M->V[x]==kk) M->PC+=2)
F(f4  ,if(M->V[x]!=kk) M->PC+=2)
F(f5  ,if(M->V[x]==M->V[y]) M->PC+=2)
F(f6  ,M->V[x]=kk)
F(f7  ,M->V[x]+=kk)
F(f80 ,M->V[x]=M->V[y])
F(f81 ,M->V[x]|=M->V[y])
F(f82 ,M->V[x]&=M->V[y])
F(f83 ,M->V[x]^=M->V[y])
F(f84 ,M->V[x]+=M->V[y]; M->V[0xf]=M->V[x]<M->V[y])
F(f85 ,M->V[0xf]=M->V[x]>M->V[y]; M->V[x]-=M->V[y])
F(f86 ,M->V[0xf]=M->V[y]&1; M->V[x]=M->V[y]>>1)
F(f87 ,M->V[0xf]=M->V[y]>M->V[x]; M->V[x]=M->V[y]-M->V[x])
F(f8e ,M->V[0xf]=M->V[y]>>7&1; M->V[x]=M->V[y]<<1)
G(f8  ){(inst[]){f80,f81,f82,f83,f84,f85,f86,f87,nop,nop,nop,nop,nop,nop,f8e,nop}[oc&0xf](M,oc);}
F(f9  ,if(M->V[x]!=M->V[y]) M->PC+=2)
F(fa  ,M->I=nnn)
F(fb  ,M->PC=nnn+M->V[0x0])
F(fc  ,M->V[x]=(rand()%256)&kk)
F(fd  ,M->V[0xf]=0; for(byte i=0; i<n; ++i){
    byte iy=(M->V[y]+i)%32;
    byte ix= M->V[x]>>3;

    byte l=M->SCR[iy][ ix     ];
    byte r=M->SCR[iy][(ix+1)%8];
    
    M->SCR[iy][ ix     ]^=M->MEM[M->I+i]>>(  M->V[x]%8);
    M->SCR[iy][(ix+1)%8]^=M->MEM[M->I+i]<<(8-M->V[x]%8);

    M->V[0xf]|=(l^M->SCR[iy][ ix     ])&l;
    M->V[0xf]|=(r^M->SCR[iy][(ix+1)%8])&r;
  }
  M->V[0xf]=M->V[0xf]!=0;
  M->DF++)
F(fee ,if(  M->KB>>M->V[x]&1)  M->PC+=2)
F(fe1 ,if(!(M->KB>>M->V[x]&1)) M->PC+=2)
G(fe  ){(inst[]){fee,fe1}[oc&1](M,oc);}
F(ff7 ,M->V[x]=M->DT)
F(ffa ,M->W=x)
F(ff15,M->DT=M->V[x])
F(ff8 ,M->ST=M->V[x])
F(ffe ,M->I+=M->V[x])
F(ff9 ,M->I=M->MEM[M->V[x]*5])
F(ff3 ,M->MEM[M->I]=M->V[x]/100; M->MEM[M->I+1]=(M->V[x]%100)/10; M->MEM[M->I+2]=M->V[x]%10)
F(ff55,for(byte i=0; i<=x; ++i) M->MEM[M->I++&0xfff]=M->V[i])
F(ff65,for(byte i=0; i<=x; ++i) M->V[i]=M->MEM[M->I++&0xfff])
G(ff5 ){(inst[]){nop,ff15,nop,nop,nop,ff55,ff65,nop,nop,nop,nop,nop,nop,nop,nop,nop}[oc>>4&0xf](M,oc);}
G(ff  ){(inst[]){nop,nop,nop,ff3,nop,ff5,nop,ff7,ff8,ff9,ffa,nop,nop,nop,ffe,nop}[oc&0xf](M,oc);}

void init(struct CHIP8*M,char*fp){
  FILE*in=fopen(fp,"rb");
  fread(M->MEM+0x200,1,4096-0x200,in);
  fclose(in);
  memcpy(M->MEM,charset,5*16);
  memset(M->SCR,0,32*8);
  M->PC=0x200;
  M->I=M->SP=M->KB=M->W=0;
  srand(time(NULL));
}

SDL_Window*w;
SDL_Renderer*r;
SDL_Texture*t;

void init_sdl(){
  SDL_Init(SDL_INIT_VIDEO);
  w=SDL_CreateWindow("",SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED,640,320,SDL_WINDOW_SHOWN);
  r=SDL_CreateRenderer(w,-1,0);
  t=SDL_CreateTexture(r,SDL_PIXELFORMAT_ARGB8888,SDL_TEXTUREACCESS_STREAMING,64,32);
}

void close_sdl(){
  SDL_DestroyTexture(t);
  SDL_DestroyRenderer(r);
  SDL_DestroyWindow(w);
  SDL_Quit();
}

int main(int argc,char*argv[]){
  Uint32 pixmap[32*64],ti,tr=0;
  struct CHIP8 M;
  SDL_Event e;
  init(&M,argv[1]);
  init_sdl();
  for(int R=1; R; ){
    while(SDL_PollEvent(&e))
      switch(e.type){
      case SDL_QUIT   : R=0; break;
      case SDL_KEYDOWN:
        M.KB|=1<<MAP[e.key.keysym.sym%256];
        if(M.W){M.V[M.W]=MAP[e.key.keysym.sym%256]; M.W=0;}
        break;
      case SDL_KEYUP  : M.KB&=~(1<<MAP[e.key.keysym.sym%256]); break;
      }
    //exec
    M.DF=0;
    ti=SDL_GetTicks();
    for(word i=0; i<5000&&M.W<1&&M.DF<1; ++i){
      word oc=M.MEM[M.PC]<<8|M.MEM[M.PC+1];
      M.PC+=2;
      (inst[]){f0,f1,f2,f3,f4,f5,f6,f7,f8,f9,fa,fb,fc,fd,fe,ff}[oc>>12&0xf](&M,oc);
    }
    //draw
    for(word i=0; i<32*64; ++i) pixmap[i]=0xffffff*(M.SCR[i/64][i%64/8]>>(7-i%8)&1);
    SDL_UpdateTexture(t,NULL,pixmap,64*4);
    SDL_RenderCopy(r,t,NULL,NULL);
    SDL_RenderPresent(r);
    tr+=20-SDL_GetTicks()+ti;
    //delay,sound
    M.DT=(--M.DT<0)?0:M.DT;
    M.ST=(--M.ST<0)?0:M.ST;
    if(tr>0){SDL_Delay(tr); tr=0;}
  }
  close_sdl();
  return 0;
}
