#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

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

int clockPin = 17;
int dataPin = 23;

int main(int argc, char **argv)
{
  setup_io();
  INP_GPIO(clockPin);
  int clock = GET_GPIO(clockPin)>>clockPin;
  clock ^= 1;
  INP_GPIO(dataPin);
  OUT_GPIO(clockPin);
  OUT_GPIO(dataPin);
  
  long filelen;
  FILE *fp;
  fp = fopen("input.txt","r");
  if( fp == NULL )
  {
    perror("Error while opening the file.\n");
    exit(EXIT_FAILURE);
  }
  fseek(fp, 0, SEEK_END);          // Jump to the end of the file
  filelen = ftell(fp);             // Get the current byte offset in the file
  rewind(fp);                      // Jump back to the beginning of the file
  char buffer[filelen];
  fread(buffer, filelen, 1, fp);   // Read in the entire file
  fclose(fp);                      // Close the file
  
  int i = 0;
  for(i=0; i<filelen; i++) 
  {
    char c = buffer[i];
    int j = 0;
    for(j=0; j<8; j++)
    {
      int bit = (c >> j & 1); 
      GPIO_CLR = 1<<dataPin;
      GPIO_SET = bit<<dataPin;
      usleep(2);
      
      GPIO_CLR = 1<<clockPin;
      GPIO_SET = clock<<clockPin;
      
      // printf("clock: %d \n", clock);
      // printf("msg: %d \n", msg[i]);
      // fflush(0);

      clock ^= 1;
      usleep(2);
    }
  }
  for(i=0; i<32; i++)
  {
    GPIO_CLR = 1<<dataPin;
    GPIO_SET = 0<<dataPin;
    usleep(2);
    
    GPIO_CLR = 1<<clockPin;
    GPIO_SET = clock<<clockPin;
    
    // printf("clock: %d \n", clock);
    // printf("msg: %d \n", msg[i]);
    // fflush(0);

    clock ^= 1;
    usleep(2);
  }
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