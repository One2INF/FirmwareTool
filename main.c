#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "crc32.h"

/** Image header.  All fields are in little endian byte order. */
typedef struct
{
  char build_time[32];
  uint32_t hdr_size;
  uint32_t img_size;
  uint32_t img_flags;
  uint32_t crc32;
  uint32_t reserve[4];
}IMAGE_HEADER_ST __attribute__((aligned(8)));


static long get_file_size(FILE *fp);
static void print_image_header(IMAGE_HEADER_ST *p);
static uint32_t calc_file_crc32(FILE *fp);


const uint8_t magic_num[4] = {0xAA, 0xBB, 0xCC, 0xDD};


int main(int argc, char *argv[])
{
  char *InputFileName;
  char *OutputFileName;

  printf("\r\n----------Firmware tool----------\r\n");

  if(argc == 3)
  {
    InputFileName = argv[1];
    OutputFileName = argv[2];
  }
  else
  {
    printf("Usage: %s input_file_name output_file_name\r\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  FILE *fp;
  if((fp = fopen(InputFileName, "rb")) == NULL)
  {
    fprintf(stderr, "Can't open %s: %s\r\n", InputFileName, strerror(errno));
    exit(EXIT_FAILURE);
  }

  IMAGE_HEADER_ST image_header;
  snprintf(image_header.build_time, sizeof(image_header.build_time), "%s %s", __DATE__, __TIME__);
  image_header.crc32 = calc_file_crc32(fp);
  image_header.hdr_size = sizeof(IMAGE_HEADER_ST);
  image_header.img_size = get_file_size(fp);
  image_header.img_flags = 0xFFFFFFFF;
  print_image_header(&image_header); 

  rewind(fp);

  FILE *fp_signed;
  rewind(fp);
  if((fp_signed = fopen(OutputFileName, "w+b")) == NULL)
  {
    fprintf(stderr, "Can't open %s: %s\r\n", OutputFileName, strerror(errno));
    exit(EXIT_FAILURE);
  }

  fwrite(&image_header, sizeof(image_header), 1, fp_signed);

  uint8_t buff[1024];
  size_t read_size;
  while((read_size = fread(buff, 1, sizeof(buff), fp)))
  {
    fwrite(buff, 1, read_size, fp_signed);
  }

  fclose(fp);
  fclose(fp_signed);

  return 0;
}

static long get_file_size(FILE *fp)
{
  long current_offset = ftell(fp);

  fseek(fp, 0L, SEEK_END);
	long size = ftell(fp);

  fseek(fp, current_offset, SEEK_SET);
  return size;
}

static void print_image_header(IMAGE_HEADER_ST *p)
{
  printf("\r\nbuild time\t: %s\r\n",p->build_time);
  printf("header size\t: %u\r\n", p->hdr_size);
  printf("image size\t: %u\r\n", p->img_size);
  printf("image flags\t: %08X\r\n", p->img_flags);
  printf("crc32\t\t: %08X\r\n", p->crc32);
}

static uint32_t calc_file_crc32(FILE *fp)
{
  uint8_t buff[1024];
  size_t read_size;
  uint32_t crc32 = 0xFFFFFFFF;
  while((read_size = fread(buff, 1, sizeof(buff), fp)))
    crc32 = stm32crc32_Byte(crc32, buff, read_size);

  return crc32;
}