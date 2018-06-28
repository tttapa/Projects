#!/usr/bin/env node

// ------------ UDP ------------ //

let localPort = 8000;
if (process.argv.length > 2) 
    localPort = parseInt(process.argv[2]);


let remotePort = localPort;
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

server.on('message', (msg, rinfo) => {
  console.log(`>>> ${msg.join(' ')} (${rinfo.address}:${rinfo.port})`);
  remoteAddr = rinfo.address;
  // remotePort = rinfo.port;
  sendSerial(msg);
});

server.on('listening', () => {
  const address = server.address();
  console.log(`Server listening on port ${address.port}`);
});

server.bind(localPort);

function sendUDP(message, length) {
    console.log(`<<< ${parser.message.slice(0, length).join(' ')} (${remoteAddr}:${remotePort})`);
    server.send(message, 0, length, remotePort, remoteAddr, (err) => { if (err) console.error(`Unable to send UDP packet: ${err}`); });
}

// ------------ Serial ------------ //

const SerialPort = require('serialport');

let port;
const baudRate = 1000000;

SerialPort.list((err, ports) => {
    if (err)
        console.error(err);
    if (ports.length == 0)
        console.error("No Serial ports found");

    // Iterate over all the serial ports, and look for an Arduino
    let comName = null;
    ports.some((port) => {
        if (port.manufacturer
            && port.manufacturer.match(/Arduino/)) {
            comName = port.comName;
            console.log('Found Arduino');
            console.log('\t' + port.comName);
            console.log('\t\t' + port.pnpId);
            console.log('\t\t' + port.manufacturer);
            return true;
        }
        return false;
    });

    if (comName == null) {
        comName = ports[0].comName;
        console.warn('No Arduino found, selecting first COM port (' + comName + ')');
    }

    // Open the port
    port = new SerialPort(comName, { baudRate: baudRate },
        (err) => {
            if (err)
                console.error(err);
        });
    
    // Attach a callback function to handle incomming data
    port.on('data', receiveSerial);
    console.log("Connected to Arduino");
});

// ----------- SLIP ------------ //

const SLIP = require('./SLIP.js');

const parser = new SLIP.SLIPParser;

function receiveSerial(dataBuf) {
    // Loop over all bytes
    for (let i = 0; i < dataBuf.length; i++) {
        // Parse the byte
        let length = parser.parse(dataBuf[i])
        if (length > 0) {
            sendUDP(parser.message, length);
        }
    }
}

const encoder = new SLIP.SLIPEncoder;

function sendSerial(dataBuf) {
    let length = encoder.encode(dataBuf);
    port.write(Buffer.from(encoder.message, 0, length));
}