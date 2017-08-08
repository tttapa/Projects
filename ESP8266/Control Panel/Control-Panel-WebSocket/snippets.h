char* output_str = strtok((char*) payload, ":");
char* state_str = strtok(NULL, ":");

if (state_str == NULL || output_str == NULL)  // invalid message format
  return;

bool state = state_str[0] - '0';  // convert from '1' (ASCII) to 1 (bool) or '0' to 0
uint8_t output = atoi(output_str);  // convert the output number string to an int

