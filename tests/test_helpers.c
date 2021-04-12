#include <stdlib.h>


FILE* open_fixture(const char *fixture_name)
{
   char filename[FILENAME_MAX];
   FILE* stream = NULL;
   
   snprintf(filename, FILENAME_MAX, "fixtures/%s.bin", fixture_name);
   
   stream = fopen(filename, "rb");
   if (!stream) {
     perror("Failed to open fixture");
     exit(-1);
   }

   return stream;
}

int read_fixture(const char *fixture_name, uint8_t buffer[], int buffer_len) {
   FILE* stream = open_fixture(fixture_name);
   int result = 0;
   
   result = fread(buffer, 1, buffer_len, stream);
   if (result <= 0) {
     perror("Failed to read fixture");
     exit(-1);
   }
   
   fclose(stream);

   return result;
}
