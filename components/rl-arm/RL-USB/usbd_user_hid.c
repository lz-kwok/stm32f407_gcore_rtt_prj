/*----------------------------------------------------------------------------
 *      RL-ARM - USB
 *----------------------------------------------------------------------------
 *      Name:    usbd_user_hid.c
 *      Purpose: Human Interface Device Class User module
 *      Rev.:    V4.70
 *----------------------------------------------------------------------------
 *      This code is part of the RealView Run-Time Library.
 *      Copyright (c) 2004-2013 KEIL - An ARM Company. All rights reserved.
 *---------------------------------------------------------------------------*/
#include <stdio.h>
#include <RTL.h>
#include <rl_usb.h>
#include <stm32f4xx.h>                  /* STM32F4xx Definitions              */


U8 feat;

void usbd_hid_init (void) {

}

int usbd_hid_get_report (U8 rtype, U8 rid, U8 *buf, U8 req) {

  switch (rtype) {
    case HID_REPORT_INPUT:
      switch (rid) {
         case 0:
          switch (req) {
            case USBD_HID_REQ_EP_CTRL:
            case USBD_HID_REQ_PERIOD_UPDATE:
//              *buf = (U8)(KBD_GetKeys ());
              return (1);
            case USBD_HID_REQ_EP_INT:
              break;
          }
           break;
      }
      break;
    case HID_REPORT_FEATURE:
      buf[0] = feat;
      return (1);
  }
  return (0);
}

void usbd_hid_set_report (U8 rtype, U8 rid, U8 *buf, int len, U8 req) {

  switch (rtype) {
    case HID_REPORT_OUTPUT:
		/* 这里不能再使用串口打印，因为usbd_STM32F4xx_FS文件里面使用了PA10*/
//		printf("buf = %d\r\n", buf[0]);
//      LED_Val (*buf);
      break;
    case HID_REPORT_FEATURE:
      feat = buf[0];
      break;
  }
}
