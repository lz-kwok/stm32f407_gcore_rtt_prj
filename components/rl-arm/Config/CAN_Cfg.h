/*----------------------------------------------------------------------------
 *      RL-ARM - CAN
 *----------------------------------------------------------------------------
 *      Name:    CAN_Cfg.h
 *      Purpose: Header for CAN Configuration Settings
 *      Rev.:    V4.70
 *----------------------------------------------------------------------------
 *      This code is part of the RealView Run-Time Library.
 *      Copyright (c) 2004-2013 KEIL - An ARM Company. All rights reserved.
 *---------------------------------------------------------------------------*/

#ifndef _CAN_CFG_H
#define _CAN_CFG_H

// *** <<< Use Configuration Wizard in Context Menu >>> ***

// <e0> Use CAN Controller 1
// </e>
// <e1> Use CAN Controller 2
// </e>
#define USE_CAN_CTRL1           1
#define USE_CAN_CTRL2           1

// <o0> Number of transmit objects for controllers <1-1024>
//      <i> Determines the size of software message buffer for transmitting.
//      <i> Default: 20
// <o1> Number of receive objects for controllers <1-1024>
//      <i> Determines the size of software message buffer for receiving.
//      <i> Default: 20
#define CAN_No_SendObjects     20
#define CAN_No_ReceiveObjects  20

// *** <<< End of Configuration section             >>> ***

#endif /* _CAN_CFG_H */


/*----------------------------------------------------------------------------
 * end of file
 *---------------------------------------------------------------------------*/

