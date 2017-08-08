let WS;
let WStimeout;
let pingInterval;
let serverTimeout;

const serverTimeoutTime = 20000;  // reload after 20 seconds

const pingTimeout = 3000;  // 3 seconds

function startWS() {
    console.log("Start WebSocket");
    WS = new WebSocket('ws://' + location.hostname + ':81/', ['arduino']);
    WS.onopen = function () {
        console.log("WebSocket Connected +++++++++++++++++++++++++++")
        online();
        pingInterval = setInterval(ping, pingTimeout);
        if (serverTimeout)
            clearTimeout(serverTimeout);
    };
    WS.onerror = function (error) {
        console.error('WebSocket Error ', error);
    };
    WS.onclose = function (ev) {
        console.error("WebSocket Closed ----------------------------", ev);
        location.reload();
    }
    WS.onmessage = function (ev) {
        clearTimeout(WStimeout);
        online();

        if (serverTimeout)
            clearTimeout(serverTimeout);
        serverTimeout = setTimeout(function () {
            console.error("Server timeout");
            location.reload();
        }, serverTimeoutTime);

        console.log(ev.data);
        if (ev.data.charAt(0) === '#') {
            let nb_outputs = parseInt(ev.data.substring(1, 3), 16);
            deleteButtons();
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
    WS.send("p");  // send ping to server
    WStimeout = setTimeout(function () {
        console.error("Ping timeout");
        offline();
    }, pingTimeout);
}

function sendButtonState() {
    if (WS.readyState !== WebSocket.OPEN) {
        console.error("Connection not open: " + WS.readyState);
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

function online() {
    if (!is_offline)
        return;
    console.log("ONLINE");
    is_offline = false;
    // deleteButtons();
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

startWS();