const END       = 0o300; /* indicates end of packet */
const ESC       = 0o333;
const ESC_END   = 0o334;
const ESC_ESC   = 0o335;

// A class for reading SLIP packets
class SLIPParser {
    constructor() {
        this.buffer = new Uint8Array(1024);
        this.index = 0;
        this.prevESC = false;
    }
    parse(byte) {
        if (byte == END) {
            let length = this.index;
            this.index = 0;
            this.prevESC = false;
            return length;
        } else if (byte == ESC) {
            this.prevESC = true;
            return 0;
        } else if (byte == ESC_END) {
            if (this.prevESC)
                byte = END;
        } else if (byte == ESC_ESC) {
            if (this.prevESC)
                byte = ESC;
        }
        this.buffer[this.index++] = byte;
        this.prevESC = false;
        return 0;
    }
    get message() {
        return this.buffer;
    }
}

class SLIPEncoder {
    constructor() {
        this.buffer = new Uint8Array(1024);
    }
    encode(buffer) {
        let index = 0;
        buffer.forEach(byte => {
            if (byte == END) {
                this.buffer[index++] = ESC;
                this.buffer[index++] = ESC_END;
            } else if (byte == ESC) {
                this.buffer[index++] = ESC;
                this.buffer[index++] = ESC_ESC;
            } else {
                this.buffer[index++] = byte;
            }
        });
        return index;
    }
    get message() {
        return this.buffer;
    }
}

module.exports = {SLIPParser, SLIPEncoder};