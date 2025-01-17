//////////////////////////////////////////////////////////////////////
// Demo1 ARM7 Code - Based on an example shipped with NDSLIB.
// Chris Double (chris.double@double.co.nz)
//////////////////////////////////////////////////////////////////////
 
#include <nds.h>

#define TOUCH_CAL_X1 (*(vs16*)0x027FFCD8)
#define TOUCH_CAL_Y1 (*(vs16*)0x027FFCDA)
#define TOUCH_CAL_X2 (*(vs16*)0x027FFCDE)
#define TOUCH_CAL_Y2 (*(vs16*)0x027FFCE0)
#define SCREEN_WIDTH    256
#define SCREEN_HEIGHT   192
s32 TOUCH_WIDTH  = TOUCH_CAL_X2 - TOUCH_CAL_X1;
s32 TOUCH_HEIGHT = TOUCH_CAL_Y2 - TOUCH_CAL_Y1;
s32 TOUCH_OFFSET_X = ( ((SCREEN_WIDTH -60) * TOUCH_CAL_X1) / TOUCH_WIDTH  ) - 28;
s32 TOUCH_OFFSET_Y = ( ((SCREEN_HEIGHT-60) * TOUCH_CAL_Y1) / TOUCH_HEIGHT ) - 28;

void InterruptHandler(void) {
  static int heartbeat = 0;
 
  if (REG_IF & IRQ_VBLANK) {
    uint16 but=0, x=0, y=0, xpx=0, ypx=0, z1=0, z2=0, batt=0, aux=0;
    int t1=0, t2=0;
    uint32 temp=0;
    uint8 ct[sizeof(IPC->curtime)];

    
    // Update the heartbeat
    heartbeat++;
 
    // Read the X/Y buttons and the /PENIRQ line
    but = REG_KEYXY;
    if (!(but & 0x40)) {
      // Read the touch screen
      x = touchRead(TSC_MEASURE_X);
      y = touchRead(TSC_MEASURE_Y);
      xpx = ( ((SCREEN_WIDTH -60) * x) / TOUCH_WIDTH  ) - TOUCH_OFFSET_X;
      ypx = ( ((SCREEN_HEIGHT-60) * y) / TOUCH_HEIGHT ) - TOUCH_OFFSET_Y;
      z1 = touchRead(TSC_MEASURE_Z1);
      z2 = touchRead(TSC_MEASURE_Z2);
    }

    batt = touchRead(TSC_MEASURE_BATTERY);
    aux  = touchRead(TSC_MEASURE_AUX);

    // Read the time
    rtcGetTime((uint8 *)ct);
    BCDToInteger((uint8 *)&(ct[1]), 7);
 
    // Read the temperature
    temp = touchReadTemperature(&t1, &t2);
 
    // Update the IPC struct
    IPC->heartbeat = heartbeat;
    IPC->buttons   = but;
    IPC->touchX    = x;
    IPC->touchY    = y;
    IPC->touchXpx  = xpx;
    IPC->touchYpx  = ypx;
    IPC->touchZ1   = z1;
    IPC->touchZ2   = z2;
    IPC->battery   = batt;
    IPC->aux       = aux;

    for(u32 i=0; i<sizeof(ct); i++) {
      IPC->curtime[i] = ct[i];
    }

    IPC->temperature = temp;
    IPC->tdiode1 = t1;
    IPC->tdiode2 = t2;
  }
 
  // Acknowledge interrupts
  REG_IF = REG_IF;
}
 

//////////////////////////////////////////////////////////////////////
 

int main(int argc, char ** argv) {
  // Reset the clock if needed
  rtcReset();

  //enable sound
  SOUND_CR = SCHANNEL_ENABLE | SOUND_VOL(0x7F);
  IPC->soundData = 0;
 
  // Set up the interrupt handler
  REG_IME = 0;
  IRQ_HANDLER = &InterruptHandler;
  REG_IE = IRQ_VBLANK;
  REG_IF = ~0;
  DISP_SR = DISP_VBLANK_IRQ;
  REG_IME = 1;

  // Keep the ARM7 out of main RAM
  while (1) swiWaitForVBlank();
  return 0;
}

 
//////////////////////////////////////////////////////////////////////

