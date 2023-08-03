#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "crc32.h"

/** Image header.  All fields are in little endian byte order. */
typedef struct
{
  uint8_t magic_num[8];
  char device_type[24];
  uint32_t hdr_size;
  uint32_t img_size;
  uint32_t img_flags;
  uint32_t crc32;
  char reserve[464];
}IMAGE_HEADER_ST __attribute__((aligned(8)));


static long get_file_size(FILE *fp);
static void print_image_header(IMAGE_HEADER_ST *p);
static uint32_t calc_file_crc32(FILE *fp);



int main(int argc, char *argv[])
{
  char* DeviceType;
  char* InputFileName;
  char* OutputFileName;

  printf("\r\n----------Firmware tool----------\r\n");

  if(argc == 4)
  {
    DeviceType = argv[1];
    InputFileName = argv[2];
    OutputFileName = argv[3];
  }
  else
  {
    printf("Usage: %s DeviceType input_file_name output_file_name\r\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  FILE *fp;
  if((fp = fopen(InputFileName, "rb")) == NULL)
  {
    fprintf(stderr, "Can't open %s: %s\r\n", InputFileName, strerror(errno));
    exit(EXIT_FAILURE);
  }

  uint8_t data[16] = {0x12, 0x13, 0x14, 0x15};
  uint32_t crc32 = stm32crc32_Byte(0xFFFFFFFF, data, 4);
  printf("crc: %08X\r\n", crc32);

  IMAGE_HEADER_ST image_header;
  memset(&image_header, 0, sizeof(image_header));
  for(uint8_t i = 0; i < 8; ++i)
    image_header.magic_num[i] = 0x11 * (i + 1);
  strncpy(image_header.device_type, DeviceType, sizeof(image_header.device_type));
  image_header.crc32 = calc_file_crc32(fp);
  image_header.hdr_size = sizeof(IMAGE_HEADER_ST);
  image_header.img_size = get_file_size(fp);
  image_header.img_flags = 0x01;
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
  printf("\r\nmagic num\t:");
  for(uint8_t i = 0; i < 8; ++i)
    printf("%02X ", p->magic_num[i]);
  printf("\r\ndevice type\t: %s\r\n",p->device_type);
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