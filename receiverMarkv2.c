#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <time.h> //for testing purposes

// Access from ARM Running Linux 
#define BCM2708_PERI_BASE        0x3F000000
#define GPIO_BASE                (BCM2708_PERI_BASE + 0x200000) /* GPIO controller */
 
#define PAGE_SIZE (4*1024)
#define BLOCK_SIZE (4*1024)
 
int  mem_fd;
void *gpio_map;
 
// I/O access
volatile unsigned *gpio;
 
 
// GPIO setup macros. Always use INP_GPIO(x) before using OUT_GPIO(x) or SET_GPIO_ALT(x,y)
#define INP_GPIO(g) *(gpio+((g)/10)) &= ~(7<<(((g)%10)*3))
#define OUT_GPIO(g) *(gpio+((g)/10)) |=  (1<<(((g)%10)*3))
#define SET_GPIO_ALT(g,a) *(gpio+(((g)/10))) |= (((a)<=3?(a)+4:(a)==4?3:2)<<(((g)%10)*3))
 
#define GPIO_SET *(gpio+7)  // sets   bits which are 1 ignores bits which are 0
#define GPIO_CLR *(gpio+10) // clears bits which are 1 ignores bits which are 0
 
#define GET_GPIO(g) (*(gpio+13)&(1<<g)) // 0 if LOW, (1<<g) if HIGH
 
#define GPIO_PULL *(gpio+37) // Pull up/pull down
#define GPIO_PULLCLK0 *(gpio+38) // Pull up/pull down clock
 
void setup_io();

int clockPin = 27;
int dataPin = 24;
int bits[100000000];

int main(int argc, char **argv)
{
  setup_io();
  INP_GPIO(clockPin);
  INP_GPIO(dataPin);
  int lastSeen = GET_GPIO(clockPin)>>clockPin;
  int counter = 0;
  int done = 0;
  printf("Setup done \n");
  fflush(0);
  clock_t start = 0;
  int first = 1;
  while(1)
  {
    if (GET_GPIO(clockPin)>>clockPin != lastSeen)
    {
      int rec = GET_GPIO(dataPin)>>dataPin;
      lastSeen ^= 1;
      bits[counter] = rec;
      counter++;
      // printf("clock: %d \n", lastSeen);
      // printf("rec: %d \n", rec);
      // printf("counter: %d \n", counter);
      // fflush(0);
      if (rec == 0) 
      {
        done++;
      } else 
      {
        done = 0;
      }
      if (done == 32)
      {
        break;
      }
      if(first == 1) {
        start = clock();
        printf("Started\n");
        first = 0;
      }
    }
  }
  
  printf("shit done \n");
  double time = (double) (clock() - start)/CLOCKS_PER_SEC;
  printf("Sending complete in: %.6fs\n", time);
  printf("Counter-32: %.6fs\n", counter-32);
  printf("bits/second:  %f\n", (counter-32)/time);
  FILE *fp;
  // finished: fp = fopen("output.txt","w");
    // int i = 0;
    // int j = 0;
    // for (i = 0; i < counter/8-4; i++) {
        // int letter = 0;
        // for (j = 0; j < 8; j++) {
            // printf("%d",bitresult[i*8+j]);
            // letter += bits[i*8+j] <<= j;
        // }
        // fprintf(fp, "%c", letter);
        // fflush(0);
    // }
    // fclose(fp); // Close the file

 
  
  
  fp = fopen("output.txt","w");
  int i = 0;
  int j = 0;
  for(i=0; i<counter-32; i=i+8)
  {
    int symbol = 0;
    for (j=0; j<8; j++)
    {
      symbol += bits[i+j] <<= j;
    }
    
    fprintf(fp, "%c", symbol);
    fflush(0);
  } 
  fclose(fp);
  time = (double) (clock() - start)/CLOCKS_PER_SEC;
  printf("With writing to file: %.6fs\n", time);
  return 0;
}


void setup_io()
{
   /* open /dev/mem */
  if ((mem_fd = open("/dev/mem", O_RDWR|O_SYNC) ) < 0) {
    printf("can't open /dev/mem \n");
    exit(-1);
  }
 
   /* mmap GPIO */
  gpio_map = mmap(
    NULL,             //Any adddress in our space will do
    BLOCK_SIZE,       //Map length
    PROT_READ|PROT_WRITE,// Enable reading & writting to mapped memory
    MAP_SHARED,       //Shared with other processes
    mem_fd,           //File to map
    GPIO_BASE         //Offset to GPIO peripheral
  );
 
  close(mem_fd); //No need to keep mem_fd open after mmap

  if (gpio_map == MAP_FAILED) {
    printf("mmap error %d\n", (int)gpio_map);//errno also set!
    exit(-1);
  }

  // Always use volatile pointer!
  gpio = (volatile unsigned *)gpio_map;
 
} // setup_io