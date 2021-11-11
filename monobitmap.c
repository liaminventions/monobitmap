
/*
Creates a monochrome frame buffer in video RAM.
We map the pattern tables to CHR RAM, using the UxROM (2) mapper.
By cleverly setting up palettes, and using a split-screen
CHR bank switch, we split the screen into four different regions
that display their own pixels.
#include "scene1"

*/

#include "neslib.h"
#include "nes.h"
#include <stdlib.h>

#include "scene16.bin.h"

#define NES_MAPPER 2		// UxROM mapper
#define NES_PRG_BANKS 2
#define NES_CHR_BANKS 0		// CHR RAM


// setup Famitone library

//#link "famitone2.s"
void __fastcall__ famitone_update(void);

bool ppu_is_on = false;
byte i;
byte x2 = 100;
byte y2 = 200;
byte x3 = 140;
byte y3 = 130;
byte read;
byte read1;
unsigned char buffer[0x0100];
int si = 0x0000;
byte temp;

// simple 6502 delay loop (5 cycles per loop)
#define DELAYLOOP(n) \
  __asm__("ldy #%b", n); \
  __asm__("@1: dey"); \
  __asm__("bne @1");

// call every frame to split screen
void monobitmap_split() {
  // split screen at line 128
  split(0,0);
  DELAYLOOP(15); // delay until end of line
  PPU.control = PPU.control ^ 0x10;
}

// set a pixel at (x,y) color 1=set, 0=clear
void monobitmap_set_pixel(byte x, byte y, byte color) {
  byte b;
  // compute pattern table address
  word a = (x/8)*16 | ((y&63)/8)*(16*32) | (y&7);
  if (y & 64) a |= 8;
  if (y & 128) a |= 0x1000;
  // if PPU is active, wait for next frame
  if (ppu_is_on) {
    ppu_wait_nmi();
  }
  // read old byte
  vram_adr(a);
  vram_read(&b, 1);
  if (color) {
    b |= 128 >> (x&7); // set pixel
  } else {
    b &= ~(128 >> (x&7)); // clear pixel
  }
  // write new byte
  vram_adr(a);
  vram_put(b);
  // if PPU is active, reset PPU addr and split screen
  if (ppu_is_on) {
    vram_adr(0);
    monobitmap_split();
  }
}

// draw a line from (x0,y0) to (x1,y1)
void monobitmap_draw_line(int x0, int y0, int x1, int y1, byte color) {
  int dx = abs(x1-x0);
  int sx = x0<x1 ? 1 : -1;
  int dy = abs(y1-y0);
  int sy = y0<y1 ? 1 : -1;
  int err = (dx>dy ? dx : -dy)>>1;
  int e2;
  for(;;) {
    monobitmap_set_pixel(x0, y0, color);
    if (x0==x1 && y0==y1) break;
    e2 = err;
    if (e2 > -dx) { err -= dy; x0 += sx; }
    if (e2 < dy) { err += dx; y0 += sy; }
  }
}

// write values 0..255 to nametable
void monobitmap_put_256inc() {
  word i;
  for (i=0; i<256; i++)
    vram_put(i);
}

// sets up attribute table
void monobitmap_put_attrib() {
  vram_fill(0x00, 0x10); // first palette
  vram_fill(0x55, 0x10); // second palette
}

// clears pattern table
void monobitmap_clear() {
  vram_adr(0x0);
  vram_fill(0x0, 0x2000);
}

// sets up PPU for monochrome bitmap
void monobitmap_setup() {
  monobitmap_clear();
  // setup nametable A and B
  vram_adr(NAMETABLE_A);
  monobitmap_put_256inc();
  monobitmap_put_256inc();
  monobitmap_put_256inc();
  monobitmap_put_256inc();
  vram_adr(NAMETABLE_A + 0x3c0);
  monobitmap_put_attrib();
  monobitmap_put_attrib();
  bank_bg(0);
  // setup sprite 0
  bank_spr(1);
  oam_clear();
  oam_size(0);
  oam_spr(247, 125, 255, 0, 0);
  // draw a pixel for it to collide with
  monobitmap_set_pixel(247, 126, 1);
  // make sprite 255 = white line
  vram_adr(0x1ff0);
  vram_fill(0xff, 0x1);
}

/*{pal:"nes",layout:"nes"}*/
const byte MONOBMP_PALETTE[16] = {
  0x0F,
  0x30, 0x0F, 0x30,  0x00,
  0x0F, 0x30, 0x30,  0x00,
  0x30, 0x0F, 0x30,  0x00,
  0x0F, 0x30, 0x30
};

void readstniccc() {

  read = scene16_bin[si]; // read a byte
  read1=read;
  temp=read1;
  if(temp & 1){
    monobitmap_clear();
  }
  temp=read1;
  if(temp & 2){
    si++;
    buffer[0] = scene16_bin[si]; // read a byte
    si++;
    buffer[1] = scene16_bin[si]; // read a byte
    si++;
    temp = buffer[0];
    while (temp != 0) {
      if (temp & 1) {
        si=si+2;
      }
      temp = temp >> 1;
    }
    temp = buffer[1];
    while (temp != 0) {
      if (temp & 1) {
        si=si+2;
      }
      temp = temp >> 1;
    }
    
  }
  temp=read1;
  if(temp & 4){
    read = scene16_bin[si]; // read a byte
    temp = si;
    si++;
    for(i==0; i==read; i++){
      x2 = scene16_bin[si+i];
      i++;
      y2 = scene16_bin[si+i];
      monobitmap_set_pixel(x2, y2, 1);
    }
    si=si+i;
    
    buffer[3] = false;
    while(buffer[3] == false){
      read = scene16_bin[si];
      if(read > 0xfd){
        buffer[3] = true;
        return;
      }
      if(read = 0xfd){
        while(true){
        }
      }
      buffer[4]=read & 0x0f;
      for(i=0; i=buffer[4]; i++){
        read = scene16_bin[i];
        x2= scene16_bin[temp+i+1];
        y2= scene16_bin[temp+i+2];
        i++;
        read = scene16_bin[i];
        x3= scene16_bin[temp+i+1];
        y3= scene16_bin[temp+i+2];
        monobitmap_draw_line(x2,y2,x3,y3,1);       
      }
      si=si+i;
    }
  }
  temp=read1;
  if(!temp & 4){ 
    buffer[3]=false;
    while(buffer[3] == false){
      read = scene16_bin[si];
      buffer[4]=read & 0x0f;
      for(i==0; i=buffer[4]; i++){
        x2= scene16_bin[si+i];
        i++;
        y2= scene16_bin[si+i];
        i++;
        x3= scene16_bin[si+i];
        i++;
        y3 = scene16_bin[si+i];
        i++;
        monobitmap_draw_line(x2,y2,x3,y3,1);
      }
      si==si+i;
      read = scene16_bin[si];
      if(read >= 0xfd){
        buffer[3] = true;
      }
    }
  }
}

void main(void)
{
  // setup and draw some lines
  monobitmap_setup();
  pal_bg(MONOBMP_PALETTE);
  ppu_on_all();
  ppu_is_on = true;
  readstniccc();
while(true){
  ppu_wait_nmi();
  monobitmap_split();
  ppu_off();
  readstniccc();
  ppu_on_all();
}
}
