#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include "aes128.h"

typedef uint8_t byte;
typedef uint32_t word;

const int N_k = 4; //length of key, in 32-bit words
const int N_b = 4; //number of columns in the State, in 32-bit words
const int N_r = 10; //number of rounds
const int N_s = 256; //size of s-box (by definition), in 8-bit bytes
const char *const SBOX_NAME = "aes_sbox.txt";
const char *const INV_SBOX_NAME = "aes_inv_sbox.txt";
byte *sbox;
byte *inv_sbox;

int read_input(const char *inFile, int file_length, byte *input_string) {
  FILE *in = fopen(inFile, "r");

  if (in == NULL) {
    printf("ERROR: Cannot %s\n", inFile);
    return(1);
  }
  
  for (int i = 0; i < file_length; i++) {
    fscanf(in, "%xhh", &input_string[i]);
  }

  fclose(in);
  return 0;
}

/*Print the contents of string as bytes represented by two hexadecimal characters.*/
int print_bytes_as_hex(int string_length, byte *byte_string ) {
  const int WORDS_PER_LINE = 4; //number of 32-bit words to print before adding a newline

  if (byte_string == NULL) {
    printf("ERROR: Attempting to print string with value NULL\n");
    return 1;
  }
  for (int i = 0; i < string_length; i++) {
    if (i % (WORDS_PER_LINE * sizeof(word)/sizeof(byte)) == 0) {
      printf("\n");
    }
    printf("%02x ", byte_string[i]);
  }
  printf("\n");

  return 0;
}

/*Counts the number of bytes of message text in the input. Excludes whitespace.*/
int find_input_length(const char *file_name) {
  int file_length = 0;
  FILE *in = fopen(file_name, "r");

  if (in == NULL) {
    printf("ERROR: Cannot open %s\n", file_name);
    return(1);
  }

  while (fscanf(in, "%x") != EOF) {
    file_length++;
  }

  fclose(in);

  return file_length;
}

int words_to_bytes(int n) {
  return n * sizeof(word) / sizeof(byte);
}

int bytes_to_words(int n) {
  return n / sizeof(word) / sizeof(byte);
}

/*Substitutes each byte in the word with the corresponding byte in the S-box*/
word sub_word(word w) {
  printf("sub_word\n");
  byte first_nibble;
  byte second_nibble;
  byte *curr_byte;

  for (int i = 0; i < words_to_bytes(1); i++) {
    curr_byte = ((byte *)&w + i);
    first_nibble = (*curr_byte >> 4);
    second_nibble = (*curr_byte & 0x0F);

    *curr_byte = sbox[16 * first_nibble + second_nibble];
  }
  return w;
}

word rot_word(word w) {
  printf("rot_word\n");
  printf("%x\n", w);

  w = (w >> 8) | ((w & 0x000000FF) << 24);

  printf("%x\n", w);
  return w;
}

word r_con(int n) {
  printf("r_con\n");
  byte temp = 0x02;
  for (int i = 0; i < n; i++) {
    temp *= temp;
  }

  word w = (temp << 24) | 0xFF000000;
  printf("n = %d, w = %x\n", n, temp);
  return w;
}

int cipher(byte *in, byte *out) {
  print_bytes_as_hex(16, in);
  return 0;
}

int key_expansion(byte *key, word *key_schedule) {
  word *temp;
  byte *sched_ptr;

  int i;

  for (i = 0; i < N_k; i++) {
    sched_ptr = (byte *)(&key_schedule + i);
    for (int j = 0; j < 4; j++) {
      sched_ptr[j] = key[4 * i + j];
    }
  }

  i = N_k;

  while (i < N_b * (N_r + 1)) {
    temp = &key_schedule[i - 1];
    printf("%x\n", temp);
    if (i % N_k == 0) {
      printf("%d\n", i);
      *temp = r_con(i / N_k) & sub_word(rot_word(*temp));
      printf("%x\n", temp);
    }
    printf("test\n");
    key_schedule[i] = key_schedule[i - N_k] & *temp;
    i++;
  }
  return 0;
}

int main(int argc, char const *argv[]) {
  const char *output_filename = "output.txt";

  if (argc < 3) {
    printf("ERROR: Must provide filenames of both the message and the key\n");
    return 1;
  }

  if (argc == 4) {
    //Output filename provided
    output_filename = argv[3];
  }

  int input_length = find_input_length(argv[1]); //length of message, in bytes

  byte *input_string = calloc(sizeof(byte), input_length);
  byte *output_string = calloc(sizeof(byte), input_length);
  byte *key = calloc(sizeof(word), N_k);
  word *key_schedule = calloc(sizeof(word), N_b * (N_r + 1));
  sbox = calloc(sizeof(byte), N_s);
  inv_sbox = calloc(sizeof(byte), N_s);

  read_input(argv[1], input_length, input_string);
  read_input(argv[2], words_to_bytes(N_k), key);
  read_input(SBOX_NAME, N_s, sbox);
  read_input(INV_SBOX_NAME, N_s, inv_sbox);

  key_expansion(key, key_schedule);

  print_bytes_as_hex(input_length, input_string);
  print_bytes_as_hex(N_k * 4, key);
  print_bytes_as_hex(N_s, sbox);
  print_bytes_as_hex(N_s, inv_sbox);

  return 0;
}
