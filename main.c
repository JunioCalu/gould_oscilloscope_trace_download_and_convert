/** \file 
 * \brief 
 *
 *  Copyright (C) 2011 samplemaker
 *  Copyright (C) 2011 Hans Ulrich Niedermann <hun@n-dimensional.de>
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
#include "serial-setup.h"


#define UART_BAUDRATE 9600UL

#define DEBUG(...)				\
  do {						\
    fprintf(stderr, __VA_ARGS__);		\
  } while (0)


typedef struct {
  char *RuNnAMe;
  char *RuNnUMber;
  int RuNnUMber_s;
  char *TRACeNAMe;
  int TRACeNAMe_s;
} TRANsfer_t;


char printable(const char ch)
{
  if ((32 <= ch) && (ch < 127)) {
    return ch;
  } else {
    return '.';
  }
}


void hexdump(const char *buf, const size_t size)
{
  const uint8_t *b = (const uint8_t *)buf;
  for (size_t y=0; y<size; y+=16) {
    DEBUG("%04x ", y);
    for (int x=0; x<16; x++) {
      if (y+x<size) {
        DEBUG(" %02x", b[y+x]);
      } else {
        DEBUG("   ");
      }
    }
    DEBUG("  ");
    for (size_t x=0; x<16; x++) {
      if (y+x<size) {
        DEBUG("%c", printable(b[y+x]));
      } else {
        DEBUG(" ");
      }
    }
    DEBUG("\n");
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


int open_char_device(const char *device_name)
{
  int fd = serial_open(device_name);
  if (fd < 0) {
    return -1;
  }
  serial_setup(fd, UART_BAUDRATE, 8, PARITY_NONE, 1);
  return fd;
}


void
get_data (const int fd, char *buf, long *count)
{
  int key_pressed = 0;
  char ch;

  (*count) = 0;
  printf("press <ESC> to stop listening \n");
  while (key_pressed != 0x1b){
    if (read(fd, &ch, 1)){
        printf("Rx << 0x%x  \r", (int)(ch)&0xff);
        fflush(stdout);
        buf[*count] = ch;
        (*count)++;
    }
    key_pressed = getkey();
  }
  printf ("<< %d bytes \n", (int)(*count));
}


void
send_cmd (const int fd, const char *buf, const size_t count)
{
  printf( ">> %.*s \n", count, buf);
  write (fd, buf, count);
  /* transmit a line feed */
  const char ch = 0x0A;
  write (fd, &ch, 1);
}


void
send_string (const int fd, const char *buf, const size_t count)
{ 
  printf( "%.*s", count, buf);
  write (fd, buf, count);
}


void
send_LF (const int fd)
{
  printf( "\n");
  const char ch = 0x0A;
  write (fd, &ch, 1);
}


int save_disc(const char *file, const void *buf, const size_t count)
{
  FILE *fd = fopen (file, "w");
  if (fd == NULL){
    return -1;
  }
  rewind(fd);
  fwrite(buf, 1, count, fd);
  fclose(fd);
  printf("%d bytes written\n", count);
  return 0;
}


void
print_help(void)
{
    printf("\n\r  SYNOPSIS\n\r");
    printf("         dso_serial -d device -o output [-n RuNnUMber] [-p TRACeNAMe]\n\r\n\r");
    printf("  DESCRIPTION\n\r");
    printf("         DSO GOULD 650 and DataSys 9xx RS-423 via RS-232 downloader\n\r\n\r");
    printf("  OPTIONS\n\r");
    printf("         -n RuNnUMber numeric data\n\r");
    printf("                Specify the number under which the trace is stored\n\r\n\r");
    printf("         -p TRACeNAMe string data\n\r");
    printf("                Tracename to be downloaded\n\r\n\r");
    printf("         -o output file\n\r");
    printf("                output for downloaded data\n\r\n\r");
    printf("         -d device\n\r");
    printf("                device file for serial output\n\r\n\r");
    printf("  EXAMPLES\n\r");
    printf("         ./dso_serial -d /dev/ttyUSB -o plot.hpgl\n\r");
    printf("         ./dso_serial -d /dev/ttyUSB -o trace1.dat -n 20 -p TR1_500.DAT\n\r\n\r");
    printf("  NOTES\n\r");
    printf("         if only a device is specified sending is omitted and the program can be used to download screenshots (plots)\n\r\n\r");
    printf("  AUTHOR\n\r");
    printf("         tbd\n\r\n\r");
}


int
main (int argc, char *argv[])
{
  long count;
  char buf[1000*1024];
  memset (buf, '\0', sizeof (buf));

  TRANsfer_t TRANsfer;
  TRANsfer.TRACeNAMe_s = 0;
  TRANsfer.RuNnUMber_s = 0;

  char *out_file = NULL;
  uint8_t dev_file_pos = 0;
  bool exit_on_error = false;
  uint16_t a = 1;
  while (a < argc){
       if ((!strcmp(argv[a], "-d")) &&  (a + 1 < argc)){
         dev_file_pos = a + 1;
         a += 2;
       }
       else if ((!strcmp(argv[a], "-p")) && (a + 1 < argc)){
              TRANsfer.TRACeNAMe = argv[a + 1];
              TRANsfer.TRACeNAMe_s = strlen(TRANsfer.TRACeNAMe);
              a += 2;
            }
            else if ((!strcmp(argv[a], "-n")) && (a + 1 < argc)){
                   TRANsfer.RuNnUMber = argv[a + 1];
                   TRANsfer.RuNnUMber_s = strlen(TRANsfer.RuNnUMber);
                   a += 2;
                 }
                 else if ((!strcmp(argv[a], "-o")) && (a + 1 < argc)){
                        out_file = argv[a + 1];
                        a += 2;
                      }
	              else{
                        exit_on_error = true;
	                a = argc;
                      }
  }

  if ((dev_file_pos == 0) || (exit_on_error == true)){
    print_help();
    exit(0);
  }

  int fd = serial_open (argv[dev_file_pos]);
  if (fd < 0) {
    printf ("Error serial_open()\n");
    exit(1);
  }
  /* 8N1 - 8 bits, no parity, 1 stop bit */
  serial_setup(fd, UART_BAUDRATE, 8, PARITY_NONE, 1);

  const int delay = 100000;

  if (TRANsfer.RuNnUMber_s > 0){
    printf( "TX >> ");
    send_string(fd, "TRAN:FILE:RNUM ", 15);
    send_string(fd, TRANsfer.RuNnUMber, TRANsfer.RuNnUMber_s);
    send_LF(fd);
    usleep(delay);
  }
  if (TRANsfer.TRACeNAMe_s > 0){
    printf( "TX >> ");
    send_string(fd, "TRAN:FILE:TRACNAM \"", 19);
    send_string(fd, TRANsfer.TRACeNAMe, TRANsfer.TRACeNAMe_s);
    send_string(fd, "\"", 1);
    send_LF(fd);
    usleep(delay);
  }
  if ((TRANsfer.TRACeNAMe_s > 0) | (TRANsfer.RuNnUMber_s > 0)){
    printf( "TX >> ");
    send_string(fd, "TRAN:FILE:EXEC?", 15);
    send_LF(fd);
    usleep(delay);
  }

  get_data (fd, buf, &count);
  hexdump(buf, count);

  if (out_file != NULL){
    save_disc(out_file, buf, count);
  }
  else
  {
    save_disc("dump.asci", buf, count);
  }

  close (fd);

  exit(0);
}

  //const char cmd_buf[] = "CHAN2:COUP?";
  //const char cmd_buf[] = "CHANnel2:COUPling AC";
  //const char cmd_buf[] = "CHAN2:PRO?";
  //const char cmd_buf[] = "CHAN2:PRO 10";
  //const char cmd_buf[] = "ALL?";
  //const char cmd_buf[] = "HELP?";
  //const char cmd_buf[] = "BEEP 1";
  //const char cmd_buf[] = "SYS:HEAD 0";
  //const char cmd_buf[] = "TRAN:FILE:RNUM 6";
  //const char cmd_buf[] = "TRAN:FILE:TRACNAM \"TR1_500.DAT\"";
  //const char cmd_buf[] = "TRAN:FILE:EXEC?";
  //const char cmd_buf[] = "TRAN:FILE:RNAM \"UNNAMED\"";
  //const char cmd_buf[] = "TRAN:FILE:RNAM?";
  //const char cmd_buf[] = "TRAN:FILE:RNUM?";
  //const char cmd_buf[] = "TRAN:FILE:TYPE?";
  //const char cmd_buf[] = "TRAN:FILE:USER?";
  //const char cmd_buf[] = "TRAN:FILE:USER \"DEFAULT\"";

/*
  const char cmd_buf1[] = "TRAN:FILE:RNUM 20";
  send_cmd(fd, cmd_buf1, sizeof (cmd_buf1));
  usleep(100000);
  const char cmd_buf2[] = "TRAN:FILE:TRACNAM \"TR2_500.DAT\"";
  send_cmd(fd, cmd_buf2, sizeof (cmd_buf2));
  usleep(100000);
  const char cmd_buf3[] = "TRAN:FILE:EXEC?";
  usleep(100000);
  send_cmd(fd, cmd_buf3, sizeof (cmd_buf3));
*/

