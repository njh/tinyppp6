#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

int open_fixture(const char *fixture_name)
{
   char filename[FILENAME_MAX];
   int fd;
   
   snprintf(filename, FILENAME_MAX, "./fixtures/%s.bin", fixture_name);
   
   fd = open(filename, O_RDONLY);
   if (fd < 0) {
     perror("Failed to open fixture");
     exit(-1);
   }

   return fd;
}

int read_fixture(const char *fixture_name, uint8_t buffer[], int buffer_len) {
   int fd = open_fixture(fixture_name);
   int result = 0;
   
   result = read(fd, buffer, buffer_len);
   if (result <= 0) {
     perror("Failed to read fixture");
     exit(-1);
   }
   
   close(fd);

   return result;
}
