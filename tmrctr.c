// tmrctr.c - Use timer counter core to keep time
//
// This basically sets up the core's two timers in cascade mode.
//
// Counter 0 is setup to count down from N to 0, with N chosen such that the
// period is 1 millisecond.  Coutner 1 then counts milliseconds.

#include "xparameters.h"
#include "xtmrctr.h"

#include "tmrctr.h"

void
init_tmrctr()
{
  int csr = 0;

  // The following sequence in taken from the AXI Timer Product Guide (pg079).

  // Clear the timer enable bits in control registers (TCSR0 and TCSR1).
  // 第一个寄存器前4字节置0
  XTmrCtr_SetControlStatusReg(XPAR_TMRCTR_0_BASEADDR, 0, csr);
  // 第二个寄存器前4字节置0
  XTmrCtr_SetControlStatusReg(XPAR_TMRCTR_0_BASEADDR, 1, csr);

  // Write the lower 32-bit timer/counter load register (TLR0).
  // 第一个个寄存器后4字节置0
  XTmrCtr_SetLoadReg(XPAR_TMRCTR_0_BASEADDR, 0, 0);

  // Write the higher 32-bit timer/counter load register (TLR1).
  // 第二个寄存器后4字节置0
  XTmrCtr_SetLoadReg(XPAR_TMRCTR_0_BASEADDR, 1, 0);

  // Set the CASC bit in Control register TCSR0.
  // 第一个寄存器前4字节置0x0000800
  // NOTE: 第二个字节为00001000
  csr |= XTC_CSR_CASC_MASK;
  XTmrCtr_SetControlStatusReg(XPAR_TMRCTR_0_BASEADDR, 0, csr);

  // Set other mode control bits in control register (TCSR0) as needed.
  // 第一个寄存器前4字节置0x0000010
  // NOTE: 第一个字节高4位为0001
  csr |= XTC_CSR_AUTO_RELOAD_MASK;
  XTmrCtr_SetControlStatusReg(XPAR_TMRCTR_0_BASEADDR, 0, csr);

  // Enable the timer in Control register (TCSR0).
  // 第一个寄存器前4字节置0x0000400
  // NOTE: 第二个字节为00001000 -> 00001100
  csr |= XTC_CSR_ENABLE_ALL_MASK;
  XTmrCtr_SetControlStatusReg(XPAR_TMRCTR_0_BASEADDR, 0, csr);
}

u64
read_tmrctr()
{
  u32 hi0, lo, hi1;

  // The following sequence in taken from the AXI Timer Product Guide (pg079).

  // Read the upper 32-bit timer/counter register (TCR1).
  hi0 = XTmrCtr_GetTimerCounterReg(XPAR_TMRCTR_0_BASEADDR, 1);

  // Read the lower 32-bit timer/counter register (TCR0).
  lo = XTmrCtr_GetTimerCounterReg(XPAR_TMRCTR_0_BASEADDR, 0);

  // Read the upper 32-bit timer/counter register (TCR1) again.
  hi1 = XTmrCtr_GetTimerCounterReg(XPAR_TMRCTR_0_BASEADDR, 1);

  // If the value is different from the 32-bit upper value read previously, go
  // back to previous step (reading TCR0). Otherwise 64-bit timer counter value
  // is correct.
  if(hi0 != hi1) {
    lo = XTmrCtr_GetTimerCounterReg(XPAR_TMRCTR_0_BASEADDR, 0);
  }

  return (((u64)hi1) << 32) | lo;
}

u32
tick_tmrctr()
{
  return (u32)((read_tmrctr() >> 15) & 0xffffffff);
}

u32
ms_tmrctr()
{
  return tick_tmrctr() / 3;
}

// Dump contents of TmrCtr registers
void
dump_tmrctr()
{
  xil_printf("Timer/Counter registers:\n");
  xil_printf("       TMRCTR_1 TMRCTR_0\n");
  xil_printf("  TCSR %08x %08x\n",
      XTmrCtr_GetControlStatusReg(XPAR_TMRCTR_0_BASEADDR, 1),
      XTmrCtr_GetControlStatusReg(XPAR_TMRCTR_0_BASEADDR, 0));
  xil_printf("  TLR  %08x %08x\n",
      XTmrCtr_GetLoadReg(XPAR_TMRCTR_0_BASEADDR, 1),
      XTmrCtr_GetLoadReg(XPAR_TMRCTR_0_BASEADDR, 0));
  xil_printf("  TCR  %08x %08x\n",
      XTmrCtr_GetTimerCounterReg(XPAR_TMRCTR_0_BASEADDR, 1),
      XTmrCtr_GetTimerCounterReg(XPAR_TMRCTR_0_BASEADDR, 0));
}
