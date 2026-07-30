//
// File: rt_zcfcn.h
//
// Code generated for Simulink model 'FlightCore'.
//
// Model version                  : 1.26
// Simulink Coder version         : 25.2 (R2025b) 28-Jul-2025
// C/C++ source code generated on : Wed Jul 29 15:50:08 2026
//
#ifndef rt_zcfcn_h_
#define rt_zcfcn_h_
#include "zero_crossing_types.h"
#include "rtwtypes.h"
#ifndef slZcHadEvent
#define slZcHadEvent(ev, zcsDir)       (((ev) & (zcsDir)) != 0x00 )
#endif

#ifndef slZcUnAliasEvents
#define slZcUnAliasEvents(evL, evR)    ((((slZcHadEvent((evL), (SL_ZCS_EVENT_N2Z)) && slZcHadEvent((evR), (SL_ZCS_EVENT_Z2P))) || (slZcHadEvent((evL), (SL_ZCS_EVENT_P2Z)) && slZcHadEvent((evR), (SL_ZCS_EVENT_Z2N)))) ? (SL_ZCS_EVENT_NUL) : (evR)))
#endif

#ifdef __cplusplus

extern "C"
{

#endif

  extern ZCEventType rt_ZCFcn(ZCDirection zcDir, ZCSigState *prevZc, real_T
    currValue);

#ifdef __cplusplus

}                                      // extern "C"

#endif
#endif                                 // rt_zcfcn_h_

//
// File trailer for generated code.
//
// [EOF]
//
