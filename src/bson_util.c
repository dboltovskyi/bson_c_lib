#include "bson_util.h"

void write_int32_le(uint8_t *bytes, int32_t value, size_t *position) {
  int i = 0;
  for (i = 0; i < SIZE_INT32; i++) {
    bytes[(*position)++] = (uint8_t)value & 0x000000FF;
    value >>= 8;
  }
}

void write_int64_le(uint8_t *bytes, int64_t value, size_t *position) {
  int i = 0;
  for (i = 0; i < SIZE_INT64; i++) {
    bytes[(*position)++] = (uint8_t)value & 0x000000FFull;
    value >>= 8;
  }
}

void write_double_le(uint8_t *bytes, double value, size_t *position) {
  union doubleUnion_t {
    double value;
    uint64_t intValue;
  };
  union doubleUnion_t unionVal;
  unionVal.value = value;
  int i = 0;
  for (i = 0; i < SIZE_DOUBLE; i++) {
    bytes[(*position)++] = (uint8_t)unionVal.intValue & 0x000000FFull;
    unionVal.intValue >>= 8;
  }
}

size_t read_byte_len(uint8_t *output, const uint8_t **data, size_t *data_size) {
  if (*data_size < 1) {
    return 0;
  }
  *output = **data;
  (*data)++;
  *data_size -= 1;
  return 1;
}

int32_t read_int32_le(uint8_t **bytes) {
  int32_t value = 0;
  int i = 0;
  for (i = SIZE_INT32 - 1; i >= 0; i--) {
    value <<= 8;
    value += (*bytes)[i];
  }
  (*bytes) += SIZE_INT32;
  return value;
}

size_t read_int32_le_len(int32_t *output, const uint8_t **data, size_t *data_size) {
  if (*data_size < SIZE_INT32) {
    return 0;
  }
  *output = 0;
  int i = 0;
  for (i = SIZE_INT32 - 1; i >= 0; i--) {
    *output <<= 8;
    *output += (*data)[i];
  }
  (*data) += SIZE_INT32;
  *data_size -= SIZE_INT32;
  return SIZE_INT32;
}

int64_t read_int64_le(uint8_t **bytes) {
  int64_t value = 0;
  int i = 0;
  for (i = SIZE_INT64 - 1; i >= 0; i--) {
    value <<= 8;
    value += (*bytes)[i];
  }
  (*bytes) += SIZE_INT64;
  return value;
}

size_t read_int64_le_len(int64_t *output, const uint8_t **data, size_t *data_size) {
  if (*data_size < SIZE_INT64) {
    return 0;
  }
  *output = 0;
  int i = 0;
  for (i = SIZE_INT64 - 1; i >= 0; i--) {
    *output <<= 8;
    *output += (*data)[i];
  }
  (*data) += SIZE_INT64;
  *data_size -= SIZE_INT64;
  return SIZE_INT64;
}

double read_double_le(uint8_t **bytes) {
  union doubleUnion_t {
    double value;
    uint64_t intValue;
  };
  union doubleUnion_t unionVal;
  unionVal.intValue = 0;
  int i = 0;
  for (i = SIZE_DOUBLE - 1; i >= 0; i--) {
    unionVal.intValue <<= 8;
    unionVal.intValue += (*bytes)[i];
  }
  (*bytes) += SIZE_DOUBLE;
  return unionVal.value;
}

size_t read_double_le_len(double *output, const uint8_t **data, size_t *data_size) {
  if (*data_size < SIZE_DOUBLE) {
    return 0;
  }
  union doubleUnion_t {
    double value;
    uint64_t intValue;
  };
  union doubleUnion_t unionVal;
  unionVal.intValue = 0;
  int i = 0;
  for (i = SIZE_DOUBLE - 1; i >= 0; i--) {
    unionVal.intValue <<= 8;
    unionVal.intValue += (*data)[i];
  }
  *output = unionVal.value;
  (*data) += SIZE_DOUBLE;
  *data_size -= SIZE_DOUBLE;
  return SIZE_DOUBLE;
}

size_t read_string_len(char **output, const uint8_t **data, size_t *data_size) {
  size_t i = 0;
  for (i = 0; i < *data_size; i++) {
    if ((*data)[i] == 0x00) {
      break;
    }
  }
  if (i == *data_size) {
    // '\0' is not found
    return 0;
  }

  *output = byte_array_to_bson_string((uint8_t *)*data, i);

  // add 1 since we also consumed '\0' at the end
  size_t bytes_read = i + 1;
  (*data) += bytes_read;
  *data_size -= bytes_read;
  return bytes_read;
}

uint8_t *string_to_byte_array(char *stringVal) {
  size_t length = strlen(stringVal);
  uint8_t *bytes = malloc(length + 1);
  int i = 0;
  for (i = 0; i < length; i++) {
    bytes[i] = (uint8_t)stringVal[i];
  }
  bytes[length] = 0x00;
  return bytes;
}

size_t c_string_length(uint8_t *value) {
  size_t length = 0;
  while (value[length] != 0x00) {
    length++;
  }
  return length;
}

char *byte_array_to_string(uint8_t *bytes) {
  size_t length = c_string_length(bytes);
  return byte_array_to_bson_string(bytes, length);
}

char *byte_array_to_bson_string(uint8_t *bytes, size_t length) {
  char *stringVal = malloc(sizeof(char) * (length + 1));
  
  int i = 0;
  for (i = 0; i < length; i++) {
    stringVal[i] = (char)(bytes[i] & 0xFF);
  }
  stringVal[length] = 0x00;
  return stringVal;
}

uint8_t *index_to_key(size_t index) {
  size_t length = digits(index);
  uint8_t *bytes = malloc(length);
  size_t modValue = index;
  int i = 0;
  for (i = (int)length - 1; i >= 0; i--) {
    //Convert digit to UTF-8
    bytes[i] = 0x30 + (modValue % 10);
    modValue /= 10;
  }
  return bytes;
}

size_t object_key_size(char *key) {
  return strlen(key) + 1;
}

size_t array_key_size(size_t index) {
  //One byte for each decimal digit in index plus null character
  return digits(index) + 1;
}

size_t digits(size_t value) {
  size_t modValue = value;
  size_t numDigits = 1;
  while (modValue >= 10) {
    numDigits++;
    modValue /= 10;
  }
  return numDigits;
}
