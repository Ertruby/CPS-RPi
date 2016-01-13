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


//
// Cleanup some GPIO pins
//
void cleanup()
{
  int g;
  for (g=7; g<=11; g++)
  {
    GPIO_CLR = 1<<g;
    INP_GPIO(g);
  }
} 

//
// Receiver, should be run in seperate thread eventually
// Args: list of GPIO pins to use, name of the file to write to
//
void receiver(int pins[], char *filename)
{
  int i, size;
  int receiving = 1;
  int msgsize = 5; //DONT USE MAGIC NUMBERS
  size = sizeof(pins)/sizeof(int);
  for (i=0; i<size; i++)
  {
    GPIO_CLR = 1<<i;
    INP_GPIO(i);
  }
  FILE * file = fopen(filename, "wb");
  int index = 0; //MARRUK
  while (receiving)
  {
    int yup = fputs( "yolo", file); //misschien handig om aan te passen
    index++;
    if(index == msgsize){
      receiving = 0;
    }
    
  }
  fclose(file);
}

int main(int argc, char **argv)
{
  int g,rep;
 
  setup_io();
 
  //Set GPIO pins 7-11 to 0 and Input
  int tra;
  for (tra=7; tra<=11; tra++)
  {
    for (g=7; g<=11; g++)
    {
      GPIO_CLR = 1<<g;
      INP_GPIO(g);
    }
  }

 
  // Set GPIO pins 7-11 to output
  for (g=10; g<11; g++)
  {
    INP_GPIO(g); // must use INP_GPIO before we can use OUT_GPIO
    OUT_GPIO(g);
  }
 
  // Switch GPIO pin 10 on and off with a 1 second delay
 /*  for (rep=0; rep<10; rep++)
  {
    for (g=10; g<11; g++)
    {
      GPIO_SET = 1<<g;
      sleep(1);
    }
    for (g=10; g<11; g++)
    {
      GPIO_CLR = 1<<g;
      sleep(1);
    }
  } */
  
  //Receiver shit
  int pinnetjes[2] = {10,11};
  receiver(pinnetjes, "yolo.txt");
  
  //Cleanup some GPIO pins
  cleanup();
  return 0;
 
}
 
//
// Set up a memory regions to access GPIO
//
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





// Good stuff
void sender() 
{
  // 14 en 15
  OUT_GPIO(14);
  OUT_GPIO(15);
  int msg[23] = {1,0,0,1,1,0,1,0,0,1,0,1,1,0,1,0,0,1,1,0,1,1,1};
  int clock = 1;
  int i = 0;
  for (i; i < sizeof (int_array) / sizeof (int); i++)
  {
	GPIO_SET = clock<<14;
	GPIO_SET = msg[i]<<15;
	if (clock == 0) 
	{
		clock = 1;
	} else 
	{
		clock = 0;
	}
	sleep(0.01)
  }
}

void receiver()
{
 // 23 en 24 
  IN_GPIO(23);
  IN_GPIO(24);
  int lastSeen = 0;
  while(1)
  {
	if (GET_GPIO(23) != lastSeen)
	{
	  int rec = GET_GPIO(24);
	  lastSeen = GET_GPIO(23);
	  printf("Received: %d", rec);
	  fflush(0);
	}
	
  }
} 
