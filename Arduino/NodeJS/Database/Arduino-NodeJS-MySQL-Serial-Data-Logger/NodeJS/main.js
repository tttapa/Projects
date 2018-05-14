#!/usr/bin/env node

// ------------ MySQL ------------ //

const MySQL = require('mysql');

const con = MySQL.createConnection({
    host: "localhost",
    port: "3306",
    user: "username",
    password: "password",
    database: "Arduino",
    charset: "utf8mb4_general_ci"
});

con.connect((err) => {
    if (err)
        console.error(err);
    console.log("Connected to database");
});

function insertValueIntoDatabase(value) {
    const sql = 'INSERT INTO `Data` (`data`) VALUES (?);';
    con.query(sql, [value], function (err, result) {
        if (err)
            console.error(err);
    });
}

// ------------ Serial ------------ //

const SerialPort = require('serialport');

const baudRate = 115200;

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
    const port = new SerialPort(comName, { baudRate: baudRate },
        (err) => {
            if (err)
                console.error(err);
        });
    
    // Attach a callback function to handle incomming data
    port.on('data', receiveSerial);
    console.log("Connected to Arduino");
});

// A class for reading lines of text
class TextParser {
    constructor() {
        this.string = '';
    }
    static isEndMarker(char) {
        return char == '\r' || char == '\n'; // New line characters (NL & CR)
    }
    parse(char) {
        if (this.clear) {
            this.string = '';
            this.clear = false;
        }
        if (TextParser.isEndMarker(char)) {
            if (this.string.length > 0) {
                this.clear = true;
                return true;
            }
            return false;
        } else {
            this.string += char;
        }
    }
    get message() {
        return this.string;
    }
}

const parser = new TextParser;

function receiveSerial(dataBuf) {
    let str = dataBuf.toString();
    // Loop over all characters
    for (let i = 0; i < str.length; i++) {
        // Parse the character
        if (parser.parse(str[i])) {
            // If a complete line has been received,
            // insert it into the database
            insertValueIntoDatabase(parser.message);
        }
    }
}