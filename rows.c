// rows, output stdin for number of chars in a row
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>

struct termios oldoptions, newoptions;
struct winsize w;

int getch(int fd);
void showusage();

int main(int argc, char *argv[])
{
  int opt;
  long int i, i1=1, chars, rows;
  unsigned int waitforkey, repeatrows, more;
  char c;
  size_t nread;
  
  // termios backup STDERR_FILENO options, will read keyboard from there
  tcgetattr( STDERR_FILENO, &oldoptions ); // read current options
  newoptions=oldoptions;
  newoptions.c_lflag &= ~( ICANON | ECHO );
  // read terminal windows size
  ioctl( STDOUT_FILENO, TIOCGWINSZ, &w );
  
   i=rows=chars=waitforkey=repeatrows=more=0;
   // parse command line
   while ((opt = getopt(argc, argv, ":kpmb:r:")) != -1) {
    switch (opt) {
     case 'k':
      waitforkey=1;
     break;
     case 'p':
      repeatrows=1;
     break;
     case 'm':
      more=1;
     break;
     case 'b':
      chars=atoi(optarg);
     break;
     case 'r':
      rows=atoi(optarg);
     break;
     case '?':
      showusage();
     break;
    }
   }
   // more like settings
   if (more) {
    waitforkey=1;
    repeatrows=1;
    if (!rows) // assume terminal height
     rows=w.ws_row;
   }
   if (waitforkey && rows)
    repeatrows=1;
  
    // read stdin, loop until EOF
    while ((nread=read(STDIN_FILENO, &c, 1))) {
     if (chars && (c=='\n' || c=='\r' || c=='\t'))
      continue;
     ++i;
     if (i==chars || c=='\n' || c=='\r') {
      if (i1==rows && repeatrows) {
       i1=0;
       if (waitforkey)
        if ((opt=getch(STDERR_FILENO))=='q' || opt==27)
         break;
      }
      if ((waitforkey && !repeatrows))
       if ((opt=getch(STDERR_FILENO))=='q' || opt==27)
        break;
      if (i1==rows && !repeatrows)
       break;
      if (c!='\n' && c!='\r')
       write(STDOUT_FILENO, "\n", 1);
      i=0; ++i1;
     }
     write(STDOUT_FILENO, &c, 1);
    }
    
 return 0;
}

// getch with termios
int getch(int fd)
{
  char t;
  
   tcsetattr( fd, TCSANOW, &newoptions );
   while (read(fd, &t, 1) <= 0);
   tcsetattr( fd, TCSANOW, &oldoptions );

 return t;
}
// show usage
void showusage()
{
  printf("Usage:\n rows [options]\n\nA bytes per row CRT viewer\n\nOptions:\n");
  printf(" -m\t\tmore like output, fill terminal height or with selected\n -p\t\trepeat rows sequence\n -b<number>\tbytes per row, default as read\n -r<number>\trows, default one\n -k<number>\twait for key on each row\n     --help\tdisplay this help\n\nDistributed under the GNU licence\n");
  exit (-1);
}
