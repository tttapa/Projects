/* 
 *  This sketch reads data from the Serial input stream of the format "<COMMAND|Value1,Value2,Value3,...,ValueN>"
 *  and parses it into the COMMAND and an array of N values.
 *  Whitespace is ignored.
 */

#define START_MARKER '<'
#define END_MARKER '>'
#define COMMAND_SEP '|'
#define VALUE_SEP ','

void setup()
{
    Serial.begin(115200);
}

const size_t buffLen = 6;     // length of the expected message chunks (number of characters between two commas) (16-bit int has 5 digits + sign)
char buffer[buffLen + 1];     // add one for terminating null character
const size_t cmdBuffLen = 10; // length of the expected command string
char cmdBuffer[cmdBuffLen + 1];

uint8_t bufferIndex = 0;

const size_t arrayOfIntsLen = 16; // number of ints to receive
int arrayOfInts[arrayOfIntsLen];
uint8_t arrayOfIntsIndex = 0;

bool receiving = false;       // set to true when start marker is received, set to false when end marker is received
bool commandReceived = false; // set to true when command separator is received (or if command buffer is full)

void loop()
{
    if (Serial.available() > 0)
    {                                    // If there's at least one byte to read
        char serialByte = Serial.read(); // Read it

        if (isWhiteSpace(serialByte))
            return; // Ignore whitespace

        if (serialByte == START_MARKER)
        { // Start marker received: reset indices and flags
            receiving = true;
            commandReceived = false;
            bufferIndex = 0;
            arrayOfIntsIndex = 0;
            return;
        }
        if (receiving)
        { // If the start marker has been received
            if (!commandReceived)
            { // If the command hasn't been received yet
                if (serialByte == COMMAND_SEP || serialByte == END_MARKER || bufferIndex == cmdBuffLen)
                { // If the command separator is received or if the command buffer is full
                    if (serialByte != END_MARKER && serialByte != COMMAND_SEP && bufferIndex == cmdBuffLen)
                    {
                        Serial.println("Error: command buffer is full, command is truncated");
                    }
                    cmdBuffer[bufferIndex] = '\0'; // Terminate the string in the buffer
                    if (serialByte == END_MARKER)
                    { // If the end marker is received
                        if (strcmp(cmdBuffer, "RAW") == 0)
                        { // Check if the received string is "RAW"
                            Serial.println("RAW:");
                        }
                        else
                        {
                            Serial.print("Unknown command (");
                            Serial.print(cmdBuffer);
                            Serial.println("):");
                        }
                        Serial.println("Message finished: (No data)");
                        Serial.println();
                        receiving = false; // Stop receivinng
                    }
                    else if (serialByte == COMMAND_SEP)
                    {
                        if (strcmp(cmdBuffer, "RAW") == 0)
                        { // Check if the received string is "RAW"
                            Serial.println("RAW:");
                        }
                        else
                        {
                            Serial.print("Unknown command (");
                            Serial.print(cmdBuffer);
                            Serial.println("):");
                        }
                        bufferIndex = 0; // Reset the index of the buffer to overwrite it with the numbers we're about to receive
                        commandReceived = true;
                    }
                }
                else
                {                                          // If the received byte is not the command separator and the command buffer is not full
                    cmdBuffer[bufferIndex++] = serialByte; // Write the new data into the buffer
                }
            }
            else if (serialByte == VALUE_SEP || serialByte == END_MARKER || bufferIndex == buffLen)
            { // If the value separator or the end marker is received, or if the buffer is full
                if (bufferIndex == buffLen && !(serialByte == VALUE_SEP || serialByte == END_MARKER))
                {
                    Serial.println("Error: buffer is full, data is truncated");
                }
                if (bufferIndex == 0)
                { // If the buffer is still empty
                    Serial.println("\t(Empty input)");
                }
                else
                {                               // If there's data in the buffer
                    buffer[bufferIndex] = '\0'; // Terminate the string
                    parseInt(buffer);           // Parse the input
                    bufferIndex = 0;            // Reset the index of the buffer to overwrite it with the next number
                }
                if (serialByte == END_MARKER)
                { // If the end marker is received
                    Serial.println("Message finished:");
                    printArrayOfInts(); // Print the values
                    Serial.println();
                    receiving = false; // Stop receivinng
                }
                return;
            }
            else
            {                                       // If the received byte is not a special character and the buffer is not full yet
                buffer[bufferIndex++] = serialByte; // Write the new data into the buffer
            }
            return; // Optional (check for next byte before executing the loop, may prevent the RX buffer from overflowing)
        }           // end if (receiving)
    }               // end if (Serial.available() > 0)
} // end of loop

bool isWhiteSpace(char character)
{
    if (character == ' ')
        return true;
    if (character == '\r')
        return true;
    if (character == '\n')
        return true;
    return false;
}

void parseInt(char *input)
{
    Serial.print("\tInput:\t");
    Serial.println(input);
    if (arrayOfIntsIndex >= arrayOfIntsLen)
    {
        Serial.println("Error: array of ints is full");
        return;
    }
    int value = atoi(input);
    arrayOfInts[arrayOfIntsIndex++] = value;
}

void printArrayOfInts()
{
    for (uint8_t i = 0; i < arrayOfIntsIndex; i++)
    {
        Serial.print(arrayOfInts[i]);
        Serial.print(' ');
    }
    Serial.println();
}
