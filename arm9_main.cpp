/*
   framebuffer_demo1 ARM9 Code 
   Chris Double (chris.double@double.co.nz)
   http://www.double.co.nz/nintendo_ds
*/

#include <nds.h>
#include <stdlib.h>

const uint16 BACKGROUND_COLOR = RGB15(0,0,0);
const uint16 SAND_COLOR = RGB15(29,25,16);
const uint16 WATER_COLOR = RGB15(4,4,31);
const uint16 FIRE_COLOR = RGB15(31,8,8);
const uint16 PLANT_COLOR = RGB15(4,25,4);
const uint16 SPOUT_COLOR = RGB15(8,16,31);
const uint16 WAX_COLOR = RGB15(29,27,25);
const uint16 WAX2_COLOR = RGB15(29,27,26);
const uint16 OIL_COLOR = RGB15(16,8,8);
const uint16 WALL_COLOR = RGB15(16,16,16);

const uint16 drawColors[] = {SAND_COLOR, WATER_COLOR, FIRE_COLOR, PLANT_COLOR, SPOUT_COLOR, WAX_COLOR, OIL_COLOR, WALL_COLOR, BACKGROUND_COLOR};
const int NUM_COLORS = sizeof(drawColors)/sizeof(uint16);
static int currentDrawColor = 0;

static int old_x = 0;
static int old_y = 0;
static int shape_x = 0;
static int shape_y = 0;
static int shape_width = 10;
static int shape_height = 10;

static int counter = 0;

float frand()
{
  return (float)rand() / (float)RAND_MAX;
}

void draw_shape(int x, int y, uint16* buffer, uint16 color)
{
  buffer += y * SCREEN_WIDTH + x;
  for(int i = 0; i < shape_height; ++i) {
    uint16* line = buffer + (SCREEN_WIDTH * i);
    for(int j = 0; j < shape_width; ++j) {
      *line++ = color;
    }
  }
}

/* yeah, these are global.
 * this was ported from Pascal.
 */
static int d0,d1,dd,u,v,p,q,ku,kv,kd,kt;
static float tk;
void parallelline(uint16 *buf, uint16 color, int pt_x, int pt_y, int d1)
{
  p = 0;
  d1 = -d1;

  while(p <= u) {
    buf[pt_y*SCREEN_WIDTH + pt_x] = color;
    if(d1 <= kt) { // square move
      pt_x++;
      d1 += kv;
    }
    else { // diagonal move
      pt_x++;
      pt_y++;
    }
    p++;
  }
}

void draw_thick_line(uint16 *buf, int start_x, int start_y, int end_x, int end_y, uint16 color, int thickness)
{
  u = end_x-start_x, // delta x
    v = end_y-start_y,   // delta y
    ku = u+u,            // change in l for square shift
    kv = v+v,            // change in d for square shift
    kd = kv-ku,          // change in d for diagonal shift
    kt = u-kv,           // diag/square decision threshold
    tk = 2*thickness*sqrtf32(u*u+v*v), // constant thickness line
    d0 = 0, d1 = 0, dd = 0, q = 0;
  int pt_x = start_x, pt_y = start_y;

  while(dd < tk) { // outer loop, stepping perpendicular to line
    parallelline(buf,color,pt_x,pt_y,d1);
    if(d0 < kt) { // square move
      pt_y++;
    }
    else { // diagonal move
      dd += kv;
      d0 -= ku;
      if(d1 < kt) { // normal diagonal
        pt_x--;
        pt_y++;
        d1 += kv;
      }
      else { // double square move, extra parallel line
        pt_x--;
        d1 += kd;
        if(dd > tk)
          break;
        parallelline(buf,color,pt_x,pt_y,d1);
        pt_y++;
      }
    }
    dd += ku;
    d0 += kv;
    q++;
  } 
}


void updatePixel(uint16 *pixels, int x)
{
  int n;
  if(pixels[x] == SAND_COLOR || pixels[x] == WATER_COLOR) {
    // sand and water move down and possibly sideways
    if(frand() < 0.95
       && (pixels[n = x+SCREEN_WIDTH] == BACKGROUND_COLOR
           || pixels[n = x+SCREEN_WIDTH-1] == BACKGROUND_COLOR
           || pixels[n = x+SCREEN_WIDTH+1] == BACKGROUND_COLOR
           || (rand() < 0.5 && pixels[n = x-1] == BACKGROUND_COLOR)
           || (rand() < 0.5 && pixels[n = x+1] == BACKGROUND_COLOR)
           )) {
      uint16 tmp = pixels[n];
      pixels[n] = pixels[x];
      pixels[x] = tmp;
    }
  } else if(pixels[x] == OIL_COLOR) {
    // oil explodes near fire
    if(frand() < 0.25
       && (pixels[x - SCREEN_WIDTH] == FIRE_COLOR || pixels[x + 1] == FIRE_COLOR
           || pixels[x - 1] == FIRE_COLOR || pixels[x + SCREEN_WIDTH] == FIRE_COLOR)) {
      pixels[x] = pixels[x + 1] = pixels[x - 1] =
        pixels[x + SCREEN_WIDTH] = pixels[x - SCREEN_WIDTH] = FIRE_COLOR;
    }
    // oil moves down and possibly sideways
    if(frand() < 0.95
       && (pixels[n = x+SCREEN_WIDTH] == BACKGROUND_COLOR
           || pixels[n = x+SCREEN_WIDTH-1] == BACKGROUND_COLOR
           || pixels[n = x+SCREEN_WIDTH+1] == BACKGROUND_COLOR
           || (frand() < 0.5 && pixels[n = x-1] == BACKGROUND_COLOR)
           || (frand() < 0.5 && pixels[n = x+1] == BACKGROUND_COLOR)
           )) {
      uint16 tmp = pixels[n];
      pixels[n] = pixels[x];
      pixels[x] = tmp;
    }
  }
  else if(pixels[x] == FIRE_COLOR) {
    int rmove = (int)((float)rand()/(float)RAND_MAX - RAND_MAX/2);
    // fire moves up randomly
    if(frand() < 0.5 && pixels[x - SCREEN_WIDTH + rmove] == BACKGROUND_COLOR) {
      pixels[x - SCREEN_WIDTH + rmove] = FIRE_COLOR;
    }
    // fire can go out if there's nothing flammable surrounding it
    if(frand() < 0.4 && pixels[x - SCREEN_WIDTH] != PLANT_COLOR && pixels[x - 1] != PLANT_COLOR
       && pixels[x + SCREEN_WIDTH] != PLANT_COLOR && pixels[x + 1] != PLANT_COLOR
       && pixels[x - SCREEN_WIDTH] != WAX_COLOR && pixels[x - 1] != WAX_COLOR
       && pixels[x + SCREEN_WIDTH] != WAX_COLOR && pixels[x + 1] != WAX_COLOR) {
      pixels[x] = BACKGROUND_COLOR;
    }
    // water puts out fire
    if(frand() < 0.9
       && (pixels[n = x + 1] == WATER_COLOR
           || pixels[n = x - 1] == WATER_COLOR
           ||  pixels[n = x + SCREEN_WIDTH] == WATER_COLOR)) {
      pixels[x] = pixels[n] = BACKGROUND_COLOR;
    }
  }
  else if(pixels[x] == PLANT_COLOR) {
    // plant burns very easily
    if(frand() < 0.2
       && (pixels[x - SCREEN_WIDTH] == FIRE_COLOR || pixels[x + 1] == FIRE_COLOR
           ||  pixels[x + SCREEN_WIDTH] == FIRE_COLOR || pixels[x - 1] == FIRE_COLOR))
      pixels[x] = FIRE_COLOR;
    // slow growth
    if(frand() >= 0.1)
      return;
    // plant grows in water
    if(pixels[x - SCREEN_WIDTH] == WATER_COLOR)
      pixels[x - SCREEN_WIDTH] = PLANT_COLOR;
    if(pixels[x + SCREEN_WIDTH] == WATER_COLOR)
      pixels[x + SCREEN_WIDTH] = PLANT_COLOR;
    if(pixels[x - 1] == WATER_COLOR)
      pixels[x - 1] = PLANT_COLOR;
    if(pixels[x + 1] == WATER_COLOR)
      pixels[x + 1] = PLANT_COLOR;
  }
  else if(pixels[x] == SPOUT_COLOR) {
    // spout produces water
    if(frand() < 0.05) {
      if(pixels[x - SCREEN_WIDTH] == BACKGROUND_COLOR)
        pixels[x - SCREEN_WIDTH] = WATER_COLOR;
      if(pixels[x + SCREEN_WIDTH] == BACKGROUND_COLOR)
        pixels[x + SCREEN_WIDTH] = WATER_COLOR;
      if(pixels[x - 1] == BACKGROUND_COLOR)
        pixels[x - 1] = WATER_COLOR;
      if(pixels[x + 1] == BACKGROUND_COLOR)
        pixels[x + 1] = WATER_COLOR;
    }
    // slow decay from sand
    if(frand() >= 0.1)
      return;
    // sand decays spout
    if(pixels[x - SCREEN_WIDTH] == SAND_COLOR || pixels[x + 1] == SAND_COLOR
       ||  pixels[x + SCREEN_WIDTH] == SAND_COLOR || pixels[x - 1] == SAND_COLOR)
      pixels[x] = SAND_COLOR;
  }
  else if(pixels[x] == WAX_COLOR) {
    // wax burns slowly
    if(frand() >= 0.01
       || pixels[x - SCREEN_WIDTH] != FIRE_COLOR
       && pixels[x + 1] != FIRE_COLOR
       && pixels[x - 1] != FIRE_COLOR && pixels[x + SCREEN_WIDTH] != FIRE_COLOR)
      return;
    // but it does burn
    pixels[x] = FIRE_COLOR;
    // wax melts
    if(pixels[n = x + SCREEN_WIDTH] == BACKGROUND_COLOR
       || pixels[n = x + SCREEN_WIDTH - 1] == BACKGROUND_COLOR
       || pixels[n = x + SCREEN_WIDTH + 1] == BACKGROUND_COLOR
       || pixels[n = x + 1] == BACKGROUND_COLOR
       || pixels[n = x - 1] == BACKGROUND_COLOR)
      pixels[n] = WAX2_COLOR;
  }
  else if(pixels[x] == WAX2_COLOR) {
    if(frand() >= 0.8)
      return;
    // dripping wax falls
    if(pixels[n = x + SCREEN_WIDTH] == BACKGROUND_COLOR
       || pixels[n = x + SCREEN_WIDTH - 1] == BACKGROUND_COLOR
       || pixels[n = x + SCREEN_WIDTH + 1] == BACKGROUND_COLOR) {
      pixels[n] = WAX2_COLOR;
      pixels[x] = BACKGROUND_COLOR;
    }
    // dripping wax that hits a surface turns back to wax
    else {
      pixels[x] = WAX_COLOR;
    }
  }
}

void updatePixels(uint16 *pixels)
{
  for(int i=SCREEN_HEIGHT-2; i>0; i--) {
    if(counter % 2 == 0) {
      for(int j=1; j<SCREEN_WIDTH; j++) {
        int x = i*SCREEN_WIDTH + j;
        updatePixel(pixels,x);
      }
    }
    else {
      for(int j=SCREEN_WIDTH-2; j>0; j--) {
        int x = i*SCREEN_WIDTH + j;
        updatePixel(pixels,x);
      }
    }
  }
}

void vline(uint16 *buf, int x, uint16 color)
{
  for(int i=0; i<SCREEN_HEIGHT; i++)
    buf[i*SCREEN_WIDTH+x] = color;
}

void hline(uint16 *buf, int y, uint16 color)
{
  for(int i=0; i<SCREEN_WIDTH; i++)
    buf[y*SCREEN_WIDTH+i] = color;
}

void on_irq() 
{	
  if(REG_IF & IRQ_VBLANK) {
    //draw_shape(old_x, old_y, VRAM_A, RGB15(0, 0, 0));
    //TODO: fixme, draw line from old_{x,y} to shape_{x,y}
    if(shape_x > 0 && shape_y > 0) {
      int u = shape_x - old_x, v = shape_y - old_y;
//       if(u > v && v > 0) {
//         draw_thick_line(VRAM_A, old_x, old_y, shape_x, shape_y, drawColors[currentDrawColor], 1);
//       }
//       else {
        draw_shape(shape_x, shape_y, VRAM_A, drawColors[currentDrawColor]);
        //      }
    }
    // blank out top/bottom lines
    hline(VRAM_A,0,BACKGROUND_COLOR);
    hline(VRAM_A,SCREEN_HEIGHT-1,BACKGROUND_COLOR);
    // draw side walls
    vline(VRAM_A,0,WALL_COLOR);
    vline(VRAM_A,SCREEN_WIDTH-1,WALL_COLOR);
    updatePixels(VRAM_A);

    // Tell the DS we handled the VBLANK interrupt
    VBLANK_INTR_WAIT_FLAGS |= IRQ_VBLANK;
    REG_IF |= IRQ_VBLANK;
  }
  else {
    // Ignore all other interrupts
    REG_IF = REG_IF;
  }
}

void InitInterruptHandler()
{
  REG_IME = 0;
  IRQ_HANDLER = on_irq;
  REG_IE = IRQ_VBLANK;
  REG_IF = ~0;
  DISP_SR = DISP_VBLANK_IRQ;
  REG_IME = 1;
}

int constrainWithWrap(int x, int mn, int mx) {
  if(x < mn)
    return mx;
  if(x > mx)
    return mn;

  return x;
}

int main(void)
{
  powerON(POWER_ALL);
  videoSetMode(MODE_FB0);
  vramSetBankA(VRAM_A_LCD);

  // Initialize and enable interrupts
  InitInterruptHandler();

  //  lcdSwap();
  while(1) {
    touchPosition touchXY = touchReadXY();
    uint16 keysPressed = ~(REG_KEYINPUT);
    if(keysPressed & KEY_UP) {
      currentDrawColor--;
    }
    if(keysPressed & KEY_DOWN) {
      currentDrawColor++;
    }
    currentDrawColor = constrainWithWrap(currentDrawColor,0,NUM_COLORS-1);
    
    old_x = shape_x;
    old_y = shape_y;
    shape_x = touchXY.px;
    shape_y = touchXY.py;
    swiWaitForVBlank();
  }

  return 0;
}
