let WS;
let WStimeout;
let pingInterval;

const pingTimeout = 1500;  // 1.5 seconds

function startWS() {
    console.log("Start WebSocket");
    WS = new WebSocket('ws://' + location.hostname + ':81/', ['arduino']);
    WS.onopen = function () {
        online();
        pingInterval = setInterval(ping, pingTimeout);
    };
    WS.onerror = function (error) {
        console.error('WebSocket Error ', error);
    };
    WS.onclose = function (ev) {
        console.error("WebSocket Closed ", ev);
        WSclose();
    }
    WS.onmessage = function (ev) {
        clearTimeout(WStimeout);

        console.log(ev.data);
        if (ev.data.charAt(0) === '#') {
            let nb_outputs = parseInt(ev.data.substring(1, 3), 16)
            displayAllButtons(nb_outputs);
            return;
        }

        let split_data = ev.data.split(':', 2);
        if (split_data.length != 2) {
            return;
        }
        let output = split_data[0];
        let state = split_data[1] === '1';
        console.log("Output " + output + ": " + state);

        updateButton(output, state);
    }
}

function ping() {
    if (WS.readyState !== WebSocket.OPEN) {
        console.error("Connection not open: " + WS.readyState);
        return;
    }
    if(!WS) {
        console.error("No WebSocket");
        return;
    }
    WS.send("p");  // send ping to server
    WStimeout = setTimeout(function () {
        console.error("Ping timeout");
        WSclose();
    }, pingTimeout);
}

function WSclose() {
    clearInterval(pingInterval);
    offline();
    WS = null;  // delete the current WebSocket
    setTimeout(startWS, 5000);  // try connecting to WebSocket server again in 5 seconds
}

function sendButtonState() {
    if (WS.readyState !== WebSocket.OPEN) {
        console.error("Connection not open: " + WS.readyState);
        return;
    }
    if(!WS) {
        console.error("No WebSocket");
        return;
    }
    let button = this.id;
    let state = this.checked;
    console.log("Button " + button + ": " + state);
    WS.send(button + ":" + (state ? "1" : "0"));
}

function displayAllButtons(nb_buttons) {
    console.log("DisplayAllButtons: " + nb_buttons);
    for (button = 0; button < nb_buttons; button++) {
        displayButton(button);
    }
}

function displayButton(button) {     // Add one HTML button to the web page
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

function deleteButtons() {
    let buttonContainer = document.getElementById("buttonContainer");
    while (buttonContainer.firstChild) {
        buttonContainer.removeChild(buttonContainer.firstChild);
    }
}

function byte_to_str(val) {
    return nibble_to_hex(val >> 4) + nibble_to_hex(val);
}

function nibble_to_hex(nibble) {
    nibble &= 0xF;
    return String.fromCharCode(nibble > 9 ? nibble - 10 + 'A'.charCodeAt(0) : nibble + '0'.charCodeAt(0));
}

function updateButton(button, state) {  // change the state of a button with a given id
    let checkbox = document.getElementById(button);
    checkbox.checked = state;
}

let is_offline = false;

function offline() {
    if (is_offline)
        return;
    is_offline = true;
    let offlineDiv = document.createElement("div");
    offlineDiv.id = "offlineDiv";
    offlineDiv.appendChild(document.createTextNode("Connection lost"));

    document.body.appendChild(offlineDiv);

    let buttonContainer = document.getElementById("buttonContainer");
    buttonContainer.style.filter = "blur(3px)";
    buttonContainer.style.pointerEvents = "none";
}

function online() {
    is_offline = false;
    deleteButtons();
    let offlineDiv = document.getElementById("offlineDiv");
    if (offlineDiv)
        document.body.removeChild(offlineDiv);
    let buttonContainer = document.getElementById("buttonContainer");
    buttonContainer.style.filter = "none";
    buttonContainer.style.pointerEvents = "auto";
}

startWS();