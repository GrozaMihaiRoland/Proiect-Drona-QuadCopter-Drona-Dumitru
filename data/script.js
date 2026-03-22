
var gateway = `ws://${window.location.hostname}/ws`;
var websocket;
// Init web socket when the page loads
window.addEventListener('load', onload);

function onload(event) {
    initWebSocket();
}


function initWebSocket() {
    console.log('Trying to open a WebSocket connection…');
    websocket = new WebSocket(gateway);
    websocket.onopen = onOpen;
    websocket.onclose = onClose;
    websocket.onmessage = onMessage;
}

// When websocket is established, call the getReadings() function
function onOpen(event) {
    console.log('Connection opened');
    
}

function onClose(event) {
    console.log('Connection closed');
    setTimeout(initWebSocket, 2000);
}

// Function that receives the message from the ESP32 with the readings
function onMessage(event) {
    console.log(event.data);
    var myObj = JSON.parse(event.data);
    var keys = Object.keys(myObj);

    for (var i = 0; i < keys.length; i++){
        var key = keys[i];
        document.getElementById(key).innerHTML = myObj[key];
    }
}

let red = 0;
let green = 0;
let blue = 0;

const buttons = document.querySelectorAll("button");

buttons.forEach(button => {
    button.addEventListener("click", () => {
        /*let data = {
            R: button.id === 'R' ? 255 : 0,
            G: button.id === 'G' ? 255 : 0,
            B: button.id === 'B' ? 255 : 0
        };*/

        switch (button.id) {
            case 'R':
                red = 1 - red; 
                break;
            case 'G':
                green = 1 - green; 
                break;
            case 'B':
                blue = 1 - blue; 
                break;
            default:
                break;
        }

        let data = {
            R: 255*red,
            G: 255*green,
            B: 255*blue
        };

        websocket.send(JSON.stringify(data));
    });
});