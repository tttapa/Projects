#!/usr/bin/env node

/* 
  A simple Node.js script that acts as a bridge between
  an Arduino or Teensy that uses the OSC (Open Sounc Control)
  protocol over SLIP (Serial Line Internet Protocol)
  and a program that uses the OSC protocol over UDP.

  Usage:

        node main.js [local-port [remote-port [remote-address]]]

    If no ports are specified, port 8888 is used (for both local send/receive port
    and remote receive port). The default remote address is 'localhost'.

    If a local port is specified, but no remote port or address, 
    port 9999 is used as the remote port, and the default remote address is 'localhost'.

    If a local port and a remote port are specified, but no remote address,
    'localhost' is used as remote address.

    When a UDP message on the local port is received, the remote address will
    be automatically updated to the sender of that UDP message. The remote port
    will not be altered.

    The local port corresponds to the 'send' port in the audio software.
    The remote port corresponds to the 'receive' port in the audio software.

    The script will automatically look for Arduino or Teensy boards connected
    via USB. If no such board can be find, the first COM port will be selected.
    You can specify a port to override the search for Arduinos.

  Dependencies:

    Node.js: https://nodejs.org/
    node-serialport: https://github.com/node-serialport/node-serialport
*/

let comName;
// comName = '/dev/ttyUSB0'; // Uncomment this line to select a specific port instead of searching for an Arduino.

// ------------ UDP ------------ //
//#region UDP

let localPort = 8888;
if (process.argv.length > 2)
    localPort = parseInt(process.argv[2]);

let remotePort = 9999;
if (process.argv.length > 3)
    remotePort = parseInt(process.argv[3]);

let remoteAddr = 'localhost';
if (process.argv.length > 4)
    remoteAddr = process.argv[4];

const dgram = require('dgram');
const server = dgram.createSocket('udp4');

server.on('error', (err) => {
    console.log(`server error:\r\n${err.stack}`);
    server.close();
});

server.on('message', (message, rinfo) => {
    console.log(`>>> ${message.join(' ')} (${rinfo.address}:${rinfo.port})`);
    remoteAddr = rinfo.address;
    sendSerial(message);
});

server.on('listening', () => {
    const address = server.address();
    console.log(`Server listening on port ${address.port}`);
});

server.bind(localPort);

function sendUDP(message) {
    console.log(`<<< ${message.join(' ')} (${remoteAddr}:${remotePort})`);
    server.send(
        message, remotePort, remoteAddr,
        (err) => { if (err) console.error(`Unable to send UDP packet: ${err}`); }
    );
}

//#endregion

// ------------ Serial ------------ //
//#region Serial

const SerialPort = require('serialport');

let port;
const baudRate = 115200;

if (comName) {
    openPort(comName);
} else {
    SerialPort.list((err, ports) => {
        if (err)
            console.error(err);
        if (ports.length == 0)
            console.error("No Serial ports found");

        // Iterate over all the serial ports, and look for an Arduino or Teensy
        ports.some((port) => {
            if (port.manufacturer
                && port.manufacturer.match(/(?:.*Teensy.*)|(?:.*Arduino.*)/)) {
                comName = port.comName;
                console.log('Found Arduino');
                console.log('\t' + port.comName);
                console.log('\t\t' + port.pnpId);
                console.log('\t\t' + port.manufacturer);
                return true;
            }
            return false;
        });

        if (!comName) {
            comName = ports[0].comName;
            console.warn('No Arduino found, selecting first COM port (' + comName + ')');
        }

        openPort(comName);
    });
}

function openPort(comName) {
    // Open the port
    port = new SerialPort(comName, { baudRate: baudRate },
        (err) => {
            if (err)
                console.error(err);
        });

    // Attach a callback function to handle incomming data
    port.on('data', receiveSerial);
    console.log("Connected to Arduino");
}

//#endregion

// ----------- SLIP ------------ //
//#region SLIP
const SLIP = require('./SLIP.js');

const parser = new SLIP.SLIPParser;

function receiveSerial(dataBuf) {
    // Loop over all received bytes
    for (let i = 0; i < dataBuf.length; i++) {
        // Parse the byte
        let length = parser.parse(dataBuf[i])
        if (length > 0) {
            sendUDP(parser.message);
        }
    }
}

const encoder = new SLIP.SLIPEncoder;

function sendSerial(dataBuf) {
    // Encode the data using the SLIP protocol
    encoder.encode(dataBuf);
    let out = Buffer.from(encoder.message);
    // Send the encoded data over the Serial port
    port.write(out);
}

//#endregion
