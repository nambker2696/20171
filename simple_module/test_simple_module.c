#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#define BUFFER_LENGTH 256               ///< The buffer length (crude but fine)
static char receive[BUFFER_LENGTH];     ///< The receive buffer from the LKM

void menu(){
   printf("\t\t1. Write message to SIMPLE_DEVICE \n");
   printf("\t\t2. Read message from SIMPLE_DEVICE \n");
   printf("\t\t3. Exit\n");
   printf("Do you want to choose? :");
}

int main(){
   int ret, fd;
   int choose = 0;
   char stringToSend[BUFFER_LENGTH];
   char xauGui[BUFFER_LENGTH], input_choose[2];
   printf("Starting device test code SIMPLE_DEVICE\n");
   fd = open("/dev/simple_module", O_RDWR);             // Open the device with read/write access
   if (fd < 0){
      perror("Failed to open the device SIMPLE_DEVICE");
      return errno;
   }

   do{
      menu();
      scanf("%[^\n]%*c", &input_choose);
      choose = atoi(input_choose);
      switch(choose){
         case 1:
            printf("Type in a short string to send to the kernel module:\n");
            scanf("%[^\n]%*c", &xauGui);
            printf("Writing message to the device [%s].\n", xauGui);
            ret = write(fd, xauGui, strlen(xauGui));
            if (ret < 0){
            perror("Failed to write the message to the device.");
               return errno;
            }
            break;
         case 2:
            printf("Press ENTER to read back from the device...\n");
            getchar();
            printf("Reading from the device...\n");
            ret = read(fd, receive, BUFFER_LENGTH);
            if (ret < 0){
               perror("Failed to read the message from the device.");
               return errno;
            }
            printf("The received message is: [%s]\n", receive);
            break;
         case 3:
            printf("End program test SIMPLE_DEVICE\n");
            close(fd);
            break;
      }
      

   }
   while(choose != 3);
   return 0;
}
