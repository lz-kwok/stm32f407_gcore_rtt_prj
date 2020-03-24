/*----------------------------------------------------------------------------
 *      RL-ARM - TCPnet
 *----------------------------------------------------------------------------
 *      Name:    HTTP_CGI.C
 *      Purpose: HTTP Server CGI Module
 *      Rev.:    V4.72
 *----------------------------------------------------------------------------
 *      This code is part of the RealView Run-Time Library.
 *      Copyright (c) 2004-2013 KEIL - An ARM Company. All rights reserved.
 *---------------------------------------------------------------------------*/

#include <Net_Config.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "bsp.h"


extern RTC_TimeTypeDef  RTC_TimeStructure;
extern RTC_AlarmTypeDef RTC_AlarmStructure;
extern RTC_DateTypeDef  RTC_DateStructure;

/* ---------------------------------------------------------------------------
 * The HTTP server provides a small scripting language.
 *
 * The script language is simple and works as follows. Each script line starts
 * with a command character, either "i", "t", "c", "#" or ".".
 *   "i" - command tells the script interpreter to "include" a file from the
 *         virtual file system and output it to the web browser.
 *   "t" - command should be followed by a line of text that is to be output
 *         to the browser.
 *   "c" - command is used to call one of the C functions from the this file.
 *         It may be followed by the line of text. This text is passed to
 *         'cgi_func()' as a pointer to environment variable.
 *   "#' - command is a comment line and is ignored (the "#" denotes a comment)
 *   "." - denotes the last script line.
 *
 * --------------------------------------------------------------------------*/

/* at_System.c */
extern  LOCALM localm[];
#define LocM   localm[NETIF_ETH]

/* Net_Config.c */
extern struct tcp_cfg   tcp_config;
extern struct http_cfg  http_config;
#define tcp_NumSocks    tcp_config.NumSocks
#define tcp_socket      tcp_config.Scb
#define http_EnAuth     http_config.EnAuth
#define http_auth_passw http_config.Passw

extern volatile int16_t  iTemp;

/* Local variables. */
static char const state[][9] = {
  "FREE",
  "CLOSED",
  "LISTEN",
  "SYN_REC",
  "SYN_SENT",
  "FINW1",
  "FINW2",
  "CLOSING",
  "LAST_ACK",
  "TWAIT",
  "CONNECT"};

/* My structure of CGI status U32 variable. This variable is private for */
/* each HTTP Session and is not altered by HTTP Server. It is only set to  */
/* zero when the cgi_func() is called for the first time.                  */
typedef struct {
  U16 xcnt;
  U16 unused;
} MY_BUF;
#define MYBUF(p)        ((MY_BUF *)p)

/* Local functions */
static void dot_format (U64 val, char *sp);

/*----------------------------------------------------------------------------
 * HTTP Server Common Gateway Interface Functions
 *---------------------------------------------------------------------------*/

/*--------------------------- cgi_process_var -------------------------------*/

void cgi_process_var (U8 *qs) {
  /* This function is called by HTTP server to process the Querry_String   */
  /* for the CGI Form GET method. It is called on SUBMIT from the browser. */
  /*.The Querry_String.is SPACE terminated.                                */
  U8 *var;
  int s[4];

  var = (U8 *)alloc_mem (40);
  do {
    /* Loop through all the parameters. */
    qs = http_get_var (qs, var, 40);
    /* Check the returned string, 'qs' now points to the next. */
    if (var[0] != 0) {
      /* Returned string is non 0-length. */
      if (str_scomp (var, "ip=") == __TRUE) {
        /* My IP address parameter. */
        sscanf ((const char *)&var[3], "%d.%d.%d.%d",&s[0],&s[1],&s[2],&s[3]);
        LocM.IpAdr[0]   = s[0];
        LocM.IpAdr[1]   = s[1];
        LocM.IpAdr[2]   = s[2];
        LocM.IpAdr[3]   = s[3];
      }
      else if (str_scomp (var, "msk=") == __TRUE) {
        /* Net mask parameter. */
        sscanf ((const char *)&var[4], "%d.%d.%d.%d",&s[0],&s[1],&s[2],&s[3]);
        LocM.NetMask[0] = s[0];
        LocM.NetMask[1] = s[1];
        LocM.NetMask[2] = s[2];
        LocM.NetMask[3] = s[3];
      }
      else if (str_scomp (var, "gw=") == __TRUE) {
        /* Default gateway parameter. */
        sscanf ((const char *)&var[3], "%d.%d.%d.%d",&s[0],&s[1],&s[2],&s[3]);
        LocM.DefGW[0]   = s[0];
        LocM.DefGW[1]   = s[1];
        LocM.DefGW[2]   = s[2];
        LocM.DefGW[3]   = s[3];
      }
      else if (str_scomp (var, "pdns=") == __TRUE) {
        /* Default gateway parameter. */
        sscanf ((const char *)&var[5], "%d.%d.%d.%d",&s[0],&s[1],&s[2],&s[3]);
        LocM.PriDNS[0]  = s[0];
        LocM.PriDNS[1]  = s[1];
        LocM.PriDNS[2]  = s[2];
        LocM.PriDNS[3]  = s[3];
      }
      else if (str_scomp (var, "sdns=") == __TRUE) {
        /* Default gateway parameter. */
        sscanf ((const char *)&var[5], "%d.%d.%d.%d",&s[0],&s[1],&s[2],&s[3]);
        LocM.SecDNS[0]  = s[0];
        LocM.SecDNS[1]  = s[1];
        LocM.SecDNS[2]  = s[2];
        LocM.SecDNS[3]  = s[3];
      }
    }
  }while (qs);
  free_mem ((OS_FRAME *)var);
}


/*--------------------------- cgi_process_data ------------------------------*/

void cgi_process_data (U8 code, U8 *dat, U16 len) {
  /* This function is called by HTTP server to process the returned Data    */
  /* for the CGI Form POST method. It is called on SUBMIT from the browser. */
  /* Parameters:                                                            */
  /*   code  - callback context code                                        */
  /*           0 = www-url-encoded form data                                */
  /*           1 = filename for file upload (0-terminated string)           */
  /*           2 = file upload raw data                                     */
  /*           3 = end of file upload (file close requested)                */
  /*           4 = any xml encoded POST data (single or last stream)        */
  /*           5 = the same as 4, but with more xml data to follow          */
  /*               Use http_get_content_type() to check the content type    */  
  /*   dat   - pointer to POST received data                                */
  /*   len   - received data length                                         */
  static FILE *f = NULL;
  U8 *p;
  U32 n;

  U8 passw[12],retyped[12];
  U8 *var,stpassw;

  switch (code) {
    case 0:
      /* Url encoded form data received. */
      break;

    case 1:
      /* Filename for file upload received as encoded by the browser. */
      /* It might contain an absolute path to a file from the sending */
      /* host. Open a file for writing. */
      var = dat;
      /* Remove path info from filename, keep only the name. */
      for (p = dat; *p; p++) {
        if (*p == '\\') var = p + 1;
      }
      if (*var == 0) {
        /* Send was clicked but a filename was not selected. */
        return;
      }
      /* Filename is OK, initialize the card. */
      if (finit(NULL) != 0) {
        f = NULL;
        return;
      }
      /* Files will be stored to the root directory of SD card. */
      f = fopen ((const char *)var,"w");
      return;

    case 2:
      /* File content data received. Write data to a file. */
      /* This function will be called several times with   */
      /* code 2 when a big file is being uploaded.         */

      if (f != NULL) {
        /* Write in 512 byte blocks. This is the optimal way for */
        /* the FAT FS with caching enabled. For cache buffer     */
        /* size of 4KB the file write speed is 1.2 MByte/sec.    */
        while ((n = len) > 0) {
          if (n > 512) n = 512;
          fwrite (dat,1,n,f);
          dat += n;
          len -= n;
        }
      }
      return;

    case 3:
      /* File upload finished. Close a file. */
      if (f != NULL) {
        fclose (f);
      }
      return;

	case 4:
      /* XML encoded content type, last packet. */
	  //pType = http_get_content_type ();
      /* check the content type for CGX file request. */
      /* pType is a pointer to a 0-terminated string  */
      /* For example: text/xml; charset=utf-8         */
      return;

    case 5:
      /* XML encoded as under 4, but with more to follow. */
      return;

    default:
      /* Ignore all other codes. */
      return;
  }

  if (len == 0) {
    /* No data or all items (radio, checkbox) are off. */
    return;
  }
  
  stpassw = 0;
  var = (U8 *)alloc_mem (40);
  do {
    /* Parse all returned parameters. */
    dat = http_get_var (dat, var, 40);
    if (var[0] != 0) {
      /* Parameter found, returned string is non 0-length. */
      if (str_scomp (var, "pw=") == __TRUE) {
        /* Change password. */
        str_copy (passw, var+3);
        stpassw |= 1;
      }
      else if (str_scomp (var, "pw2=") == __TRUE) {
        /* Retyped password. */
        str_copy (retyped, var+4);
        stpassw |= 2;
      }
    }
  }while (dat);
  free_mem ((OS_FRAME *)var);

  if (stpassw == 0x03) {
    len = strlen ((const char *)passw);
    if (mem_comp (passw, retyped, len) == __TRUE) {
      /* OK, both entered passwords the same, change it. */
      str_copy (http_auth_passw, passw);
    }
  }
}


/*--------------------------- cgi_func --------------------------------------*/
U16 cgi_func (U8 *env, U8 *buf, U16 buflen, U32 *pcgi) {
  /* This function is called by HTTP server script interpreter to make a    */
  /* formated output for 'stdout'. It returns the number of bytes written   */
  /* to the output buffer. Hi-bit of return value (len is or-ed with 0x8000)*/
  /* is a repeat flag for the system script interpreter. If this bit is set */
  /* to 1, the system will call the 'cgi_func()' again for the same script  */
  /* line with parameter 'pcgi' pointing to a 4-byte buffer. This buffer    */
  /* can be used for storing different status variables for this function.  */
  /* It is set to 0 by HTTP Server on first call and is not altered by      */
  /* HTTP server for repeated calls. This function should NEVER write more  */
  /* than 'buflen' bytes to the buffer.                                     */
  /* Parameters:                                                            */
  /*   env    - environment variable string                                 */
  /*   buf    - HTTP transmit buffer                                        */
  /*   buflen - length of this buffer (500-1400 bytes - depends on MSS)     */
  /*   pcgi   - pointer to session local buffer used for repeated loops     */
  /*            This is a U32 variable - size is 4 bytes. Value is:         */
  /*            - on 1st call = 0                                           */
  /*            - 2nd call    = as set by this function on first call       */
  TCP_INFO *tsoc;
  U32 len = 0;
  
  const char mask[] = "*.*";
  static FINFO info;
  char temp[16];

  switch (env[0]) {
    /* Analyze the environment string. It is the script 'c' line starting */
    /* at position 2. What you write to the script file is returned here. */
    case 'a' :
      /* Network parameters - file 'network.cgi' */
      switch (env[2]) {
        case 'i':
          /* Write the local IP address. The format string is included */
          /* in environment string of the script line.                 */
          len = sprintf((char *)buf,(const char *)&env[4],LocM.IpAdr[0],
                        LocM.IpAdr[1],LocM.IpAdr[2],LocM.IpAdr[3]);
          break;
        case 'm':
          /* Write local Net mask. */
          len = sprintf((char *)buf,(const char *)&env[4],LocM.NetMask[0],
                        LocM.NetMask[1],LocM.NetMask[2],LocM.NetMask[3]);
          break;
        case 'g':
          /* Write default gateway address. */
          len = sprintf((char *)buf,(const char *)&env[4],LocM.DefGW[0],
                        LocM.DefGW[1],LocM.DefGW[2],LocM.DefGW[3]);
          break;
        case 'p':
          /* Write primary DNS server address. */
          len = sprintf((char *)buf,(const char *)&env[4],LocM.PriDNS[0],
                        LocM.PriDNS[1],LocM.PriDNS[2],LocM.PriDNS[3]);
          break;
        case 's':
          /* Write secondary DNS server address. */
          len = sprintf((char *)buf,(const char *)&env[4],LocM.SecDNS[0],
                        LocM.SecDNS[1],LocM.SecDNS[2],LocM.SecDNS[3]);
          break;
      }
      break;

    case 'c':
      /* TCP status - file 'tcp.cgi' */
      while ((len + 150) < buflen) {
        tsoc = &tcp_socket[MYBUF(pcgi)->xcnt];
        MYBUF(pcgi)->xcnt++;
        /* 'sprintf' format string is defined here. */
        len += sprintf((char *)(buf+len),"<tr align=\"center\">");
        if (tsoc->State <= TCP_STATE_CLOSED) {
          len += sprintf ((char *)(buf+len),
                          "<td>%d</td><td>%s</td><td>-</td><td>-</td>"
                          "<td>-</td><td>-</td></tr>\r\n",
                          MYBUF(pcgi)->xcnt,state[tsoc->State]);
        }
        else if (tsoc->State == TCP_STATE_LISTEN) {
          len += sprintf ((char *)(buf+len),
                          "<td>%d</td><td>%s</td><td>-</td><td>-</td>"
                          "<td>%d</td><td>-</td></tr>\r\n",
                          MYBUF(pcgi)->xcnt,state[tsoc->State],tsoc->LocPort);
        }
        else {
          len += sprintf ((char *)(buf+len),
                          "<td>%d</td><td>%s</td><td>%d.%d.%d.%d</td>"
                          "<td>%d</td><td>%d</td><td>%d</td></tr>\r\n",
                          MYBUF(pcgi)->xcnt,state[tsoc->State],
                          tsoc->RemIpAdr[0],tsoc->RemIpAdr[1],
                          tsoc->RemIpAdr[2],tsoc->RemIpAdr[3],
                          tsoc->RemPort,tsoc->LocPort,tsoc->AliveTimer);
        }
        /* Repeat for all TCP Sockets. */
        if (MYBUF(pcgi)->xcnt == tcp_NumSocks) {
          break;
        }
      }
      if (MYBUF(pcgi)->xcnt < tcp_NumSocks) {
        /* Hi bit is a repeat flag. */
        len |= 0x8000;
      }
      break;

    case 'd':
      /* System password - file 'system.cgi' */
      switch (env[2]) {
        case '1':
          len = sprintf((char *)buf,(const char *)&env[4],
                        http_EnAuth ? "Enabled" : "Disabled");
          break;
		  
        case '2':
          len = sprintf((char *)buf,(const char *)&env[4],http_auth_passw);
          break;
      }
      break;

    case 'g':
      /* DS18B20 Input - xml file 'ds180b20.cgx' */
      switch (env[2]) {
        case '1':
			  RTC_GetTime(RTC_Format_BIN, &RTC_TimeStructure);
			  len = sprintf((char *)buf,(const char *)&env[4],  RTC_TimeStructure.RTC_Hours, 
															   RTC_TimeStructure.RTC_Minutes,
															   RTC_TimeStructure.RTC_Seconds);
          break;
			  
        case '2':
          len = sprintf((char *)buf,(const char *)&env[4],(float)iTemp/16);
          break;
		
        case '3':
          len = sprintf((char *)buf,(const char *)&env[4], iTemp/16);
          break;
      }
      break;
	  
	 case 'h':
      /* Directory of SD Card - file 'dir.cgi' */
      if (MYBUF(pcgi)->xcnt == 0) {
        /* First Call, set initial value for 'ffind' function. */
        info.fileID = 0;
        if (finit(NULL) != 0) {
          /* No card or failed to initialize. */
          break;
        }
      }
      /* Repeat for all files, ignore Folders. */
      MYBUF(pcgi)->xcnt++;
      if (ffind (mask,&info) == 0) {
        dot_format (info.size, temp);
        len += sprintf ((char *)buf,"<tr align=center><td>%d.</td>"
                                    "<td align=left><a href=\"/%s\">%s</a></td>"
                                    "<td align=right>%s</td>"
                                    "<td>%02d.%02d.%04d - %02d:%02d</td></tr>\r\n",
                           MYBUF(pcgi)->xcnt,info.name,info.name,temp,
                           info.time.day, info.time.mon, info.time.year,
                           info.time.hr, info.time.min);
        /* Hi bit is a repeat flag. */
        len |= 0x8000;
      }
      break;
	  
	 case 'p':
      /* DS18B20 Input - xml file 'ds180b20.cgx' */
	  RTC_GetTime(RTC_Format_BIN, &RTC_TimeStructure);
      len = sprintf((char *)buf,(const char *)&env[1],  RTC_TimeStructure.RTC_Hours, 
													   RTC_TimeStructure.RTC_Minutes,
													   RTC_TimeStructure.RTC_Seconds);
      break;
	  
	 case 'q':
	 switch (env[1]) {
        case '1':
			BEEP_Start(5, 1, 1);
			break;
        case '2':
			 BEEP_Start(5, 1, 2);
          break;
		}

	  len = 0;
      break;

    case 'x':
      /* DS18B20 Input - xml file 'ds180b20.cgx' */
      len = sprintf((char *)buf,(const char *)&env[1], (float)iTemp/16);
      break;

    case 'y':
      /* Button state - xml file 'button.cgx' */
      len = sprintf((char *)buf,"<checkbox><id>button%c</id><on>%s</on></checkbox>",
                    env[1],(1 & (1 << (env[1]-'0'))) ? "true" : "false");
  }
  return ((U16)len);
}

/*----------------------------------------------------------------------------
 *        Print size in dotted fomat
 *---------------------------------------------------------------------------*/
static void dot_format (U64 val, char *sp) {

  if (val >= (U64)1e9) {
    sp += sprintf (sp,"%d.",(U32)(val/(U64)1e9));
    val %= (U64)1e9;
    sp += sprintf (sp,"%03d.",(U32)(val/(U64)1e6));
    val %= (U64)1e6;
    sprintf (sp,"%03d.%03d",(U32)(val/1000),(U32)(val%1000));
    return;
  }
  if (val >= (U64)1e6) {
    sp += sprintf (sp,"%d.",(U32)(val/(U64)1e6));
    val %= (U64)1e6;
    sprintf (sp,"%03d.%03d",(U32)(val/1000),(U32)(val%1000));
    return;
  }
  if (val >= 1000) {
    sprintf (sp,"%d.%03d",(U32)(val/1000),(U32)(val%1000));
    return;
  }
  sprintf (sp,"%d",(U32)(val));
}

/*--------------------------- cgx_content_type ------------------------------*/
#if 0
U8 *cgx_content_type (void) {
  /* User configurable Content-type for CGX script files. If this function */
  /* is missing, or returns a NULL pointer, the default 'text/xml' content */
  /* type from the library is used for xml-script file types.              */
  return ("text/xml; charset=utf-8");
}
#endif

/*--------------------------- http_encoding ---------------------------------*/

#if 0
U8 *http_encoding (void) {
  /* User configurable character encoding for text types. If this function */
  /* is missing, or returns a NULL pointer, web browser uses a default     */
  /* character encoding, which is set in the browser configuration.        */
  return ("utf-8");
}
#endif

/*--------------------------- http_accept_host ------------------------------*/
#if 0
BOOL http_accept_host (U8 *rem_ip, U16 rem_port) {
  /* This function checks if a connection from remote host is accepted or  */
  /* not. If this function is missing, all remote hosts are accepted.      */
  //printf("ip = %d,%d,%d,%d\r\n", rem_ip[0], rem_ip[1], rem_ip[2], rem_ip[0]);
	
   if (rem_ip[0] == 192  &&
       rem_ip[1] == 168  &&
       rem_ip[2] == 1    &&
       rem_ip[3] == 1) {
      /* Accept a connection. */  
      return (__TRUE);
   }
   /* Deny a connection. */
   return (__FALSE);
}
#endif

/*----------------------------------------------------------------------------
 * end of file
 *---------------------------------------------------------------------------*/
