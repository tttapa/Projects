let WS;
let pingTimeOut;
let pingInterval;
let serverTimeout;

const serverTimeoutTime = 20000;  // reload after 20 seconds

const pingTimeoutTime = 3000;  // 3 seconds

function startWS() {
    /* 
    Connect to the ESP8266's WebSocket server on port 81.
    */
    console.log("Start WebSocket");
    WS = new WebSocket('ws://' + location.hostname + ':81/', ['arduino']);
    /*
    Set the WebSocket event handlers
    */
    WS.onopen = function () {
        console.log("WebSocket Connected")
        pingInterval = setInterval(ping, pingTimeoutTime);  // start sending out ping messages
    };
    WS.onerror = function (error) {
        console.error("WebSocket Error ", error);
    };
    WS.onclose = function (ev) {  // When the WebSocket connection is closed
        console.error("WebSocket Closed ", ev);
        location.reload();  // reload the web page
    }
    WS.onmessage = function (ev) {
        online();

        clearTimeout(pingTimeOut);  // message received, so we're online, clear pingTimeout

        clearTimeout(serverTimeout);  // message received, so we're online, clear serverTimeout
        /* 
        If we don't receive a response of the server in time, reload the page.
        The server will also automatically close our connection if we're silent for a while.
        */
        serverTimeout = setTimeout(function () {
            console.error("Server timeout");
            location.reload();
        }, serverTimeoutTime);

        console.log(ev.data);

        /*
        Handle the incoming message:
        - If the message starts with '#', the server sent the number of buttons. Add them to the web page.
        - If the message contains one ':', it's a button state update, so update the button.
          The number before the colon is the index of the button, the number after the colon is the new state.
        - If the message is not of this format, just ignore it and return.
        */
        if (ev.data.charAt(0) === '#') {
            let nb_outputs = parseInt(ev.data.substring(1, 3), 16);
            displayAllButtons(nb_outputs);
            return;
        }

        let split_data = ev.data.split(':', 2);
        if (split_data.length != 2) {
            return;
        }
        let output = split_data[0];
        let state = split_data[1] === '1';

        updateButton(output, state);
    }
}

/* 
Send ping messages to the server.
If the server doesn't respond in time, let the user know that the connection was lost by calling the offline function.
The timeout will be cleared in the WS.onmessage event if a response arrives in time.
*/
function ping() {
    if (WS.readyState !== WebSocket.OPEN) {  // if the WebSocket connection is not open
        console.error("Connection not open: " + WS.readyState);
        offline();
        return;
    }
    WS.send("p");  // send ping to server
    pingTimeOut = setTimeout(function () {
        console.error("Ping timeout");
        offline();
    }, pingTimeoutTime);
}

/*
Sends a new button state to the server.
This is the onchange event of each input button.
*/
function sendButtonState() {
    if (WS.readyState !== WebSocket.OPEN) {  // if the WebSocket connection is not open
        console.error("Connection not open: " + WS.readyState);
        offline();
        return;
    }
    let button = this.id;
    let state = this.checked;
    console.log("Button " + button + ": " + state);
    WS.send(button + ":" + (state ? "1" : "0"));
}

/*
Add a given number of buttons to the HTML web page.
*/
function displayAllButtons(nb_buttons) {
    console.log("DisplayAllButtons: " + nb_buttons);
    for (button = 0; button < nb_buttons; button++) {
        displayButton(button);
    }
}

/*
Add one HTML button to the web page.
https://www.w3schools.com/howto/howto_css_switch.asp
*/
function displayButton(button) {
    let buttondiv = document.createElement("div");
    buttondiv.innerHTML =
        `<h2>Output ${button + 1}</h2>\
                <label class="switch">
                    <input id="${byte_to_str(button)}" type="checkbox">
                    <div class="slider round"></div>
                </label>`;
    let checkbox = buttondiv.getElementsByTagName("input")[0];
    checkbox.onchange = sendButtonState;
    document.getElementById("buttonContainer").appendChild(buttondiv);
}

/*
Some functions to generate HEX strings from numbers.
*/
function byte_to_str(val) {
    return nibble_to_hex(val >> 4) + nibble_to_hex(val);
}
function nibble_to_hex(nibble) {
    nibble &= 0xF;
    return String.fromCharCode(nibble > 9 ? nibble - 10 + 'A'.charCodeAt(0) : nibble + '0'.charCodeAt(0));
}

/*
Change the state of a button with a given id.
*/
function updateButton(button, state) { 
    let checkbox = document.getElementById(button);
    checkbox.checked = state;
}

/*
When there is no connection, add a div to the bottom of the page to notify the user that 
the information on the screen is no longer up to date, because there's no connection to the ESP8266.
Also blur the control panel, and make it non-clickable.
*/
let is_offline = false;

function offline() {
    if (is_offline)
        return;
    console.error("OFFLINE");
    is_offline = true;
    let offlineDiv = document.createElement("div");
    offlineDiv.id = "offlineDiv";
    offlineDiv.appendChild(document.createTextNode("Connection lost"));

    document.body.appendChild(offlineDiv);

    let buttonContainer = document.getElementById("buttonContainer");
    buttonContainer.style.filter = "blur(3px)";
    buttonContainer.style.pointerEvents = "none";
}

/*
When the control panel is back online, remove the div with the "Connection lost" notification,
unblur the control panel, and make it clickable again.
Also request the states of all buttons, by sending "?" to the server.
*/
function online() {
    if (!is_offline)
        return;
    console.log("ONLINE");
    is_offline = false;
    let offlineDiv = document.getElementById("offlineDiv");
    if (offlineDiv)
        document.body.removeChild(offlineDiv);
    let buttonContainer = document.getElementById("buttonContainer");
    buttonContainer.style.filter = "none";
    buttonContainer.style.pointerEvents = "auto";

    if (WS.readyState !== WebSocket.OPEN) {
        console.error("Connection not open: " + WS.readyState);
        return;
    }
    WS.send("?");
}

startWS();  // actually start the WebSocket connection