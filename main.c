/** \file main.c
 * \brief Datatransfer with GOULD DSO 650
 *
 * \author Copyright (C) 2017 samplemaker
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public License
 *  as published by the Free Software Foundation; either version 2.1
 *  of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free
 *  Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301 USA
 *
 */

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <regex.h>
#include <time.h>
#include "serial-setup.h"


#define UART_BAUDRATE 9600UL

#define MAX_BUF_SIZE (1024*1024)


typedef struct {
  char *runnumber;
  char *tracename;
} trancmd_t;


char printable(const char ch)
{
  if ((32 <= ch) && (ch < 127)) {
    return ch;
  } else {
    return '.';
  }
}


void hexdump(const void *data, const size_t size)
{
  const uint8_t *b = (const uint8_t *)data;
  for (size_t y=0; y<size; y+=16) {
    char buf[80];
    ssize_t idx = 0;
    idx += sprintf(&(buf[idx]), "%04zx ", y);
    for (size_t x=0; x<16; x++) {
      const size_t i = x+y;
      if (i<size) {
        idx += sprintf(&(buf[idx]), " %02x", b[i]);
      } else {
        idx += sprintf(&(buf[idx]), "   ");
      }
    }
    idx += sprintf(&buf[idx], "  ");
    for (size_t x=0; x<16; x++) {
      const size_t i = x+y;
      if (i<size) {
        idx += sprintf(&buf[idx], "%c", printable(b[i]));
      } else {
        idx += sprintf(&buf[idx], " ");
      }
    }
    printf("%s\n", buf);
  }
}


int getkey() {
  int ch;
  struct termios orig_term_attr;
  struct termios new_term_attr;
  /* set the terminal to raw mode */
  tcgetattr(fileno(stdin), &orig_term_attr);
  memcpy(&new_term_attr, &orig_term_attr, sizeof(struct termios));
  new_term_attr.c_lflag &= ~(ECHO|ICANON);
  new_term_attr.c_cc[VTIME] = 0;
  new_term_attr.c_cc[VMIN] = 0;
  tcsetattr(fileno(stdin), TCSANOW, &new_term_attr);
  /* read a ch from the stdin stream without blocking
   * returns EOF (-1) if no ch is available */
  ch = fgetc(stdin);
  /* restore the original terminal attributes */
  tcsetattr(fileno(stdin), TCSANOW, &orig_term_attr);
  return ch;
}


void
reset_timer(struct timespec *timer_start){
  clock_gettime(CLOCK_MONOTONIC, timer_start);
}

bool
timer_poll_timeout(const struct timespec *timer_start, struct timespec *timer_act, const double thresh){
  clock_gettime(CLOCK_MONOTONIC, timer_act);
  const double time_elapsed = (double)timer_act->tv_sec+1.0e-9*(double)(timer_act->tv_nsec)-
                              ((double)timer_start->tv_sec+1.0e-9*(double)(timer_start->tv_nsec));
  return(time_elapsed > thresh);
}

void
get_plotdata (const int fd, char *buf, size_t *count, double timer1_thresh, double timer2_thresh)
{
  int receive_state = 0;
  char ch;
  int key_pressed = 0;
  bool timeout = false;
  bool get_data = false;
  struct timespec timer_start = {0,0}, timer_act = {0,0};

  reset_timer(&timer_start);
  (*count) = 0;
  printf("press now the plot-button or press <ESC> to cancel transmission\n");
  while ((key_pressed != 0x1b) && (!timeout)){
    if (read(fd, &ch, 1)){
         get_data = true;
        //printf("Rx << 0x%x  \r", (int)(ch)&0xff);
         switch (receive_state) {
            case 0:
              printf("Receiving << \\ \r");
            break;
            case 1:
              printf("Receiving << | \r");
            break;
            case 2:
              printf("Receiving << / \r");
            break;
            case 3:
              printf("Receiving << - \r");
            break;
            default:
            break;
        }
        if (receive_state == 3) {
           receive_state = 0;
        }else{
           receive_state ++;
        }
        if (*count == MAX_BUF_SIZE) {
          printf("data buffer over run (too much data received - increase buffer)\n");
          exit(EXIT_FAILURE);
        }
        fflush(stdout);
        buf[*count] = ch;
        (*count)++;
        reset_timer(&timer_start);
    }
    key_pressed = getkey();
    //TX/RX connection only - hence if after timer_thresh no new data is received
    //assume end of transmission
    if (get_data){
       timeout = timer_poll_timeout(&timer_start, &timer_act, timer1_thresh);
    }else{
       timeout = timer_poll_timeout(&timer_start, &timer_act, timer2_thresh);
    }
  }
  printf ("<< %d bytes received \n", (int)(*count));
}


void
get_trackdata (const int fd, char *buf, size_t *count, double timer_thresh)
{
  int receive_state = 0;
  char ch;
  int key_pressed = 0;
  bool timeout = false;
  struct timespec timer_start = {0,0}, timer_act = {0,0};

  reset_timer(&timer_start);
  (*count) = 0;
  printf("press <ESC> to cancel transmission\n");
  while ((key_pressed != 0x1b) && (!timeout)){
    if (read(fd, &ch, 1)){
        //printf("Rx << 0x%x  \r", (int)(ch)&0xff);
         switch (receive_state) {
            case 0:
              printf("Receiving << \\ \r");
            break;
            case 1:
              printf("Receiving << | \r");
            break;
            case 2:
              printf("Receiving << / \r");
            break;
            case 3:
              printf("Receiving << - \r");
            break;
            default:
            break;
        }
        if (receive_state == 3) {
           receive_state = 0;
        }else{
           receive_state ++;
        }
        if (*count == MAX_BUF_SIZE) {
          printf("data buffer over run (too much data received - increase buffer)\n");
          exit(EXIT_FAILURE);
        }
        fflush(stdout);
        buf[*count] = ch;
        (*count)++;
        reset_timer(&timer_start);
    }
    key_pressed = getkey();
    //TX/RX connection only - hence if after timer_thresh no new data is received
    //assume end of transmission
    timeout = timer_poll_timeout(&timer_start, &timer_act, timer_thresh);
  }
  printf ("<< %d bytes received \n", (int)(*count));
}


void
send_cmd (const int fd, const char *buf, const size_t len)
{
  printf( "TX >> %.*s \n", (int)(len), buf);
  const char ch = 0x0A;
  write (fd, buf, len);
  write (fd, &ch, 1);
}


void save_disc(const char *file, const void *buf, const size_t count)
{
  FILE *fd;

  if (file == NULL){
    fd = fopen ("log.dat", "w");
    printf( "no output file specified - storing under default './log.dat'\n");
  }
  else {
    fd = fopen (file, "w");
  }

  if (fd == NULL){
    exit(EXIT_FAILURE);
  }
  rewind(fd);
  fwrite(buf, 1, count, fd);
  fclose(fd);
  printf("%zd bytes written\n", count);
}


void convert_and_save_disc(const char *file, const void *buf, const size_t count)
{
  FILE *fd1,*fd2;

  if (file == NULL){
    fd1 = fopen ("log.dat", "w");
    fd2 = fopen ("log.csv", "w");
    printf( "no output file specified - storing under default './log.*'\n");
  }
  else {
    fd1 = fopen (file, "w");
    //exchange file extension if there is one
    char fileexp[256];
    memset(fileexp, 0, sizeof(fileexp));
    strcpy(fileexp,file);
    char *pExt = strrchr(fileexp, '.');
    if (pExt != NULL)
       strcpy(pExt, ".csv");
    else
       strcat(fileexp, ".csv");
    printf("Exporting FAMOS track file to CSV: %s\n", fileexp);
    fd2 = fopen (fileexp, "w");
  }

  if ((fd1 == NULL) | (fd2 == NULL)){
    exit(EXIT_FAILURE);
  }
  rewind(fd1);
  fwrite(buf, 1, count, fd1);
  fclose(fd1);
  printf("%zd bytes written\n", count);

  char str[256];
  long double mesialVoltage, offsetVoltage, sampleRate, triggerDelay;

  int r;
  regex_t reg;
  regmatch_t match[5];

  /*
  |CD,1,a,1,b,c,d,e;
    X-axis:
    a=sample rate (default 1. for ext clock)
    b=trigger delay
    c=0 for ext clock,1 for DSO timebase
    d=length of e string (1 or 6)
    e=EXTCLK or s (s=seconds)

    we search for one or more character not containing ',' which are separated by ','
  */
  regcomp(&reg, "\\|CD,1,([^,]+),1,([^,]+),([^,]+),[^,]+,[^,]+;", REG_EXTENDED);
  r=regexec(&reg, buf, 4, match, 0);
  if (r == 0)
  {
    fprintf(fd2,"#X-Axis:\n");
    fprintf(fd2,"#Samplerate:        \t%.*s\n", match[1].rm_eo - match[1].rm_so, (unsigned char *)(buf) + match[1].rm_so);
    strncpy(str, buf + match[1].rm_so, match[1].rm_eo - match[1].rm_so);
    sscanf(str, "%Lf", &sampleRate);
    printf("Samplerate: %Le\n",sampleRate);
    fprintf(fd2,"#Trigger delay:     \t%.*s\n", match[2].rm_eo - match[2].rm_so, (unsigned char *)(buf) + match[2].rm_so);
    strncpy(str, buf + match[2].rm_so, match[2].rm_eo - match[2].rm_so);
    sscanf(str, "%Lf", &triggerDelay);
    printf("Trigger Delay: %Le\n",triggerDelay);
    const char *ch = (buf + match[3].rm_so);
    if (*ch == '0'){
      fprintf(fd2,"#Time base:       \tExt clock\n");
    }else if(*ch == '1'){
      fprintf(fd2,"#Time base:        \tDSO timebase\n");
    } else {
      fprintf(fd2,"#Error: unknown timebase field\n");
    };
  }else{
    printf("Note: No horizontal setup found\n");
  }

  /*
  |CR,1,1,0,1,0.,255.,0.,255.,a,b,c,d,e;
    Y-axis:
    a=mesial voltage
    b=offset in volts
    c=1 if variable volts/div off, else 0
    d=length of e string (1 or 4)
    e=V if variable volts/div off, else V NC

    we search for one or more character not containing ',' which are separated by ','
  */
  regcomp(&reg, "\\|CR,1,1,0,1,[^,]+,[^,]+,[^,]+,[^,]+,([^,]+),([^,]+),([^,]+),[^,]+,[^,]+;",REG_EXTENDED);
  r=regexec(&reg, buf, 4, match, 0);
  if (r == 0)
  {
    fprintf(fd2,"#Y-Axis:\n");
    fprintf(fd2,"#Mesial voltage:    \t%.*s\n", match[1].rm_eo - match[1].rm_so, (unsigned char *)(buf) + match[1].rm_so);
    strncpy(str, buf + match[1].rm_so, match[1].rm_eo - match[1].rm_so);
    sscanf(str, "%Lf", &mesialVoltage);
    printf("Mesial Voltage: %Le\n",mesialVoltage);
    fprintf(fd2,"#Offset in volts:   \t%.*s\n", match[2].rm_eo - match[2].rm_so, (unsigned char *)(buf) + match[2].rm_so);
    strncpy(str, buf + match[2].rm_so, match[2].rm_eo - match[2].rm_so);
    sscanf(str, "%Lf", &offsetVoltage);
    printf("Offset Voltage: %Le\n",offsetVoltage);
    const char *ch = (buf + match[3].rm_so);
    if (*ch == '0'){
      fprintf(fd2,"#Variable volts/div on\n");
    }else if(*ch == '1'){
      fprintf(fd2,"#Variable volts/div off\n");
    } else {
      fprintf(fd2,"#Error: variable volts/div\n");
    };
  } else{
    printf("Note: No vertical setup found\n");
  }

  /*
  |CS,1,a,b;
    a=the number of data bytes
    b=the data itself
  |CA,1,0000000000;
    Footer
  */
  regcomp(&reg, "\\|CS,1,([0-9]+),(.*);\\|CA,1,0000000000;", REG_EXTENDED);
  r=regexec(&reg, buf, 4, match, 0);
  if (r == 0)
  {
    fprintf(fd2,"#Number of Samples: \t%.*s\n", match[1].rm_eo - match[1].rm_so, (unsigned char *)(buf) + match[1].rm_so);
    size_t len = match[2].rm_eo - match[2].rm_so;
    unsigned char *ch;
    for (size_t i = 0; i < len; i++){
      ch = ((unsigned char *)(buf) + match[2].rm_so + i);
      /* could be either
         physVoltage=mesialVoltage*(internalVoltage-128.0l)/128.0l-offsetVoltage;
         or
         physVoltage=mesialVoltage*(internalVoltage-128.0l)/127.0l-offsetVoltage;
       */
      long double physVoltage=mesialVoltage*((long double)(*ch)-128.0l)/128.0l-offsetVoltage;
      long double timeBase = (long double)(i)*sampleRate-triggerDelay;
      fprintf(fd2,"%.7Le \t %.7Le\n",timeBase,physVoltage);
    }
  } else{
    printf("Note: no datapoints found\n");
  }

  regfree(&reg);
  fclose(fd2);
}


void
print_help(void)
{
    printf("\n\r  SYNOPSIS\n\r");
    printf("         dso_serial -d device -o output [-n runnumber] [-p tracename] [-s]\n\r\n\r");
    printf("  DESCRIPTION\n\r");
    printf("         DSO GOULD 650 and DataSys 9xx RS-423 via RS-232 downloader\n\r\n\r");
    printf("  OPTIONS\n\r");
    printf("         -s\n\r");
    printf("                screenshot (start this program and then press plot)\n\r\n\r");
    printf("         -n runnumber numeric data\n\r");
    printf("                number under which the trace is stored\n\r\n\r");
    printf("         -p tracename string data\n\r");
    printf("                tracename to be downloaded\n\r");
    printf("                if a FAMOS file is recognized the data is converted to *.csv as well\n\r\n\r");
    printf("         -o output file\n\r");
    printf("                output file for downloaded trace data\n\r\n\r");
    printf("         -d device\n\r");
    printf("                device file for serial data transfer\n\r\n\r");
    printf("  EXAMPLES\n\r");
    printf("         ./dso_serial -d /dev/ttyUSB0 -o plot.hpgl -s\n\r");
    printf("         ./dso_serial -d /dev/ttyUSB0 -o trace1.dat -n 20 -p TR1_5K0.DAT\n\r\n\r");
    printf("  NOTES\n\r");
    printf("  AUTHOR\n\r");
    printf("         samplemaker\n\r\n\r");
}


int main(int argc, char *argv[])
{
  /* for data content */
  char buf[MAX_BUF_SIZE];
  //memset (buf, '\0', sizeof (buf));
  /* for compilation of commands assuming they will never be larger */
  char cmd_buf[4096];
  trancmd_t trancmd={NULL,NULL};
  size_t count;

  char *out_file = NULL;
  char *device = NULL;
  int opt;
  const int delay = 300000;

  enum { NONE, SCREENSHOT, GETFILE } mode = NONE;

  while ((opt = getopt(argc, argv, "sn:p:d:o:")) != -1) {
    switch (opt) {
      case 's': mode = SCREENSHOT; break;
      case 'p': trancmd.tracename = strdup(optarg); //duplicates into a null terminated string
                printf ("Download File: \"%s\"\n", trancmd.tracename);
                mode = GETFILE; break;
      case 'n': trancmd.runnumber = strdup(optarg); //duplicates into a null terminated string
                printf ("Run number: \"%s\"\n", trancmd.runnumber); break;
      case 'd': device = strdup(optarg); break; //duplicates into a null terminated string
      case 'o': out_file = strdup(optarg); break; //duplicates into a null terminated string
      default:
          fprintf(stderr, "Usage: %s [sg:n:p:d:o:]\n", argv[0]);
          exit(EXIT_FAILURE);
      }
  }
  // Now optind (declared extern int by <unistd.h>) is the index of the first non-option argument.
  // If it is >= argc, there were no non-option arguments.

  if (device == NULL){
    print_help();
    printf ("No device specified\n");
    exit(EXIT_FAILURE);
  }
  int fd = serial_open (device);
  if (fd < 0) {
    print_help();
    printf ("Error serial_open()\n");
    exit(EXIT_FAILURE);
  }

  /* 8N1 - 8 bits, no parity, 1 stop bit */
  serial_setup(fd, UART_BAUDRATE, 8, PARITY_NONE, 1);

  switch (mode) {
     case SCREENSHOT:
       //listen and wait until the plot button is pressed
       get_plotdata (fd, buf, &count, 2.2, 120.0);
       hexdump(buf, count);
       save_disc(out_file, buf, count);
     break;
     case GETFILE:
       if ((trancmd.tracename != NULL) & (trancmd.runnumber != NULL)){
         //assemble string const char cmd_buf[] = "TRAN:FILE:RNUM 6";
         const char cmd_rnum[] = "TRAN:FILE:RNUM "; //creates a null terminated string
         //strlen gives length excluding null terminating character
         //string in cmd_buf will be non null terminated after copying
         strncpy(cmd_buf, cmd_rnum, strlen (cmd_rnum));
         size_t i = strlen (cmd_rnum);
         //append directly without having any null termination
         strncpy(cmd_buf + i, trancmd.runnumber, strlen (trancmd.runnumber));
         i = i + strlen (trancmd.runnumber);
         //i holds the string length and the string in cmd_buf does not
         //contain any null termination at all
         send_cmd(fd, cmd_buf, i);
         usleep(delay);
         //assemble string const char cmd_buf[] = "TRAN:FILE:TRACNAM \"TR1_5K0.DAT\"";
         const char cmd_trnam[] = "TRAN:FILE:TRACNAM \"";
         strncpy(cmd_buf, cmd_trnam, strlen(cmd_trnam));
         i = strlen(cmd_trnam);
         strncpy(cmd_buf + i, trancmd.tracename, strlen(trancmd.tracename));
         i = i + strlen(trancmd.tracename);
         strncpy(cmd_buf + i, "\"", 1);
         i++;
         send_cmd(fd,cmd_buf,i);
         usleep(delay);
         const char cmd_exec[] = "TRAN:FILE:EXEC?";
         strncpy(cmd_buf, cmd_exec, strlen(cmd_exec));
         send_cmd(fd,cmd_buf,strlen(cmd_exec));
         //usleep(delay);
         get_trackdata (fd, buf, &count, 2.2);
         hexdump(buf, count);
         convert_and_save_disc(out_file, buf, count);
       }
       else{
         printf ("To download a file you have to specify a runnumber a tracename\n");
       };
     break;
     default:
         print_help();
         printf ("Fall through - no mode\n");
         exit(EXIT_FAILURE);
  }
  close (fd);
  exit(EXIT_SUCCESS);
}

