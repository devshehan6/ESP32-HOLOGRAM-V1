#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>

// ============================================================
// WIFI SETTINGS
// ============================================================

const char* ssid = "DESKTOP-5J8MM72 4221_";
const char* password = "11111112";


// ============================================================
// PIN SETTINGS
// ============================================================

// Output pins  ***********   Change it your choice   ***********
const int pins[7] = {
  21, 20, 10, 9, 8, 7, 6
};

// IR sensor input *********** Change it your choice  ***********
const int IR_PIN = 5;


// ============================================================
// SEQUENCE SETTINGS
// ============================================================

#define MAX_ROWS 100

// Delay between sequence steps
// 1 microsecond -> 1,000,000 microseconds (1 second)
unsigned long delayUs = 100000;

// Number of sequence rows
int t = 4;

// Pattern matrix
int patterns[7][MAX_ROWS];

int rowCount = 4;

// ============================================================
// WEB SERVER
// ============================================================
WebServer server(80);
Preferences prefs;


// ============================================================
// SEQUENCE STATE
// ============================================================
int curStep = 0;
bool sequenceRunning = false;
// Used to detect HIGH -> LOW transition
bool lastIRState = HIGH;
// Timing
unsigned long lastStepMicros = 0;


// ============================================================
// WEB PAGE
// ============================================================

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">
<meta name="viewport"
      content="width=device-width,initial-scale=1,maximum-scale=1">
<title>MECH-SEQ-DVSH</title>

<style>
*{
    box-sizing:border-box;
    font-family:'Courier New',Courier,monospace;
    margin:0;
    padding:0
}
body{
    background:#080808;
    color:#ddd
}
.h{
    background:#101010;
    border-bottom:2px solid #f59e0b;
    padding:10px 12px;
    display:flex;
    justify-content:space-between;
    align-items:center;
    position:sticky;
    top:0;
    z-index:20
}
.b{
    border:1px solid #f59e0b;
    color:#f59e0b;
    background:#f59e0b22;
    padding:3px 8px;
    font-size:10px
}
.c{
    max-width:900px;
    margin:0 auto;
    padding:12px;
    display:flex;
    flex-direction:column;
    gap:12px
}
.p{
    background:#151515;
    border:1px solid #262626;
    border-radius:10px;
    padding:14px
}
.tt{
    font-size:10px;
    color:#777;
    letter-spacing:1px;
    display:flex;
    justify-content:space-between;
    margin-bottom:10px
}
.dsp{
    background:#000;
    border:1px solid #333;
    color:#f59e0b;
    font-size:26px;
    text-align:center;
    padding:8px;
    border-radius:6px;
    letter-spacing:2px
}
.r{
    display:flex;
    gap:10px;
    flex-wrap:wrap
}
.r>*{
    flex:1;
    min-width:120px
}
.ni{
    background:#000;
    border:1px solid #333;
    color:#fff;
    padding:10px;
    border-radius:6px;
    width:100%;
    text-align:center;
    font-size:18px
}
.btn{
    background:#222;
    border:1px solid #333;
    color:#fff;
    padding:10px 12px;
    border-radius:6px;
    cursor:pointer;
    font-size:12px
}
.btn:active{
    transform:scale(.98)
}
.bp{
    background:#f59e0b;
    color:#000;
    font-weight:700;
    border-color:#f59e0b
}
table{
    width:100%;
    border-collapse:collapse
}
th{
    font-size:10px;
    color:#555;
    padding:8px 2px;
    border-bottom:1px solid #222
}
td{
    padding:8px 2px;
    text-align:center;
    border-bottom:1px solid #181818
}
.tog{
    width:52px;
    height:30px;
    background:#111;
    border:1px solid #333;
    border-radius:20px;
    position:relative;
    cursor:pointer;
    margin:0 auto;
    transition:.15s
}
.tog.on{
    background:#f59e0b22;
    border-color:#f59e0b;
    box-shadow:0 0 8px #f59e0b33
}
.kn{
    width:22px;
    height:22px;
    background:#555;
    border-radius:50%;
    position:absolute;
    left:3px;
    top:3px;
    transition:.2s;
    display:flex;
    align-items:center;
    justify-content:center;
    font-size:11px;
    color:#000;
    font-weight:700
}
.tog.on .kn{
    left:25px;
    background:#f59e0b
}
.dot{
    width:14px;
    height:14px;
    border-radius:50%;
    background:#1a1a1a;
    border:1px solid #333;
    display:inline-block
}
.dot.on{
    background:#22c55e;
    box-shadow:0 0 8px #22c55e
}
.live-bar{
    height:4px;
    background:#222;
    border-radius:2px;
    overflow:hidden;
    margin-top:8px
}
.live-fill{
    height:100%;
    background:#f59e0b;
    transition:width .1s
}
@media(max-width:600px){
    .c{
        padding:8px
    }
    .p{
        padding:10px
    }
    .r{
        flex-direction:column
    }
}
</style>
</head>
<body>
<div class="h">
    <div style=" font-weight:700; letter-spacing:2px; font-size:13px ">
        MECH-SEQ // S3
    </div>
    <div style="
        display:flex; gap:8px; align-items:center ">
        <span style="font-size:10px">
            IR TRIGGER
        </span>
        <div class="b" id="ip">
            WAITING
        </div>
    </div>
</div>
<div class="c">
<div class="p">
    <div class="tt">
        <span>
            STEP DELAY [1µs - 1s]
        </span>
        <span id="delayLabel">
            100000µs
        </span>
    </div>
    <div class="r">
        <div class="dsp" id="delayDisplay">
            0100000
        </div>
        <div style="flex:2">
            <input type="number" id="delayInput" class="ni" min="1" max="1000000" value="100000">
            <div style=" display:flex; gap:6px; margin-top:8px ">
                <button class="btn" onclick="changeDelay(-1000)">
                    -1ms
                </button>
                <button class="btn" onclick="changeDelay(-1)">
                    -1µs
                </button>
                <button class="btn" onclick="changeDelay(1)">
                    +1µs
                </button>
                <button class="btn" onclick="changeDelay(1000)">
                    +1ms
                </button>
            </div>
        </div>
    </div>
</div>
<div class="p">
    <div class="tt">
        <span>
            IR SENSOR
        </span>
        <span id="irStatus">
            HIGH
        </span>
    </div>
    <div
        id="irDisplay"
        style=" background:#000;
            border:1px solid #333;
            color:#f59e0b;
            font-size:24px;
            text-align:center;
            padding:15px;
            border-radius:6px ">
        WAITING FOR IR
    </div>
</div>
<div class="p">
    <div class="tt">
        <span>
            CURRENT SEQUENCE
        </span>
        <span id="stepInfo">
            STEP 0/4
        </span>
    </div>

    <div
        id="pinV"
        style="
            display:flex;
            justify-content:space-between;
            background:#000;
            border:1px solid #222;
            padding:12px;
            border-radius:8px " >
    </div>
    <div class="live-bar">
        <div class="live-fill" id="liveFill" style="width:0%" >
        </div>
    </div>
    <div
        id="binPrev"
        style="
            margin-top:8px;
            font-size:11px;
            color:#666;
            text-align:center;
            letter-spacing:2px " >
    </div>
</div>
<div class="p">
    <div class="tt">
        <span>
            TURN COUNT = TABLE ROWS
        </span>
        <span style="color:#f59e0b">
            MANUAL EDIT
        </span>

    </div>
    <div style="
        display:flex;
        gap:8px;
        align-items:center ">

        <button class="btn" onclick="changeT(-1)">
            -
        </button>
        <input
            class="ni"
            id="tI"
            type="number"
            min="1"
            value="4"
            style="font-size:22px" >
        <button
            class="btn"
            onclick="changeT(1)">
            +
        </button>
        <button
            class="btn bp"
            onclick="applyT()"
            style="flex:0">
            SET ROWS
        </button>
    </div>
</div> 

<div class="p"> 
    <div class="tt"> 
        <span>
            PATTERN MATRIX L1-L7
        </span>
        <div style=" display:flex; gap:6px  ">
            <button
                class="btn"
                onclick="exportData()">
                EXPORT
            </button>
            <button class="btn" onclick="document.getElementById('fileInput').click() ">
                IMPORT
            </button>
            <input id="fileInput" type="file" hidden accept=".json" onchange="importData(event)" >
            <button class="btn bp" onclick="saveESP()">
                SAVE TO ESP
            </button>
        </div>
    </div>
    <div style="overflow:auto">
        <table>
            <thead>
                <tr>
                    <th>#</th>
                    <th>
                        L1<br>
                        <span style="font-size:8px">
                            21
                        </span>
                    </th>

                    <th>
                        L2<br>
                        <span style="font-size:8px">
                            20
                        </span>
                    </th>

                    <th>
                        L3<br>
                        <span style="font-size:8px">
                            10
                        </span>
                    </th>

                    <th>
                        L4<br>
                        <span style="font-size:8px">
                            9
                        </span>
                    </th>

                    <th>
                        L5<br>
                        <span style="font-size:8px">
                            8
                        </span>
                    </th>

                    <th>
                        L6<br>
                        <span style="font-size:8px">
                            7
                        </span>
                    </th>

                    <th>
                        L7<br>
                        <span style="font-size:8px">
                            6
                        </span>
                    </th>
                    <th></th>
                </tr>
            </thead>
            <tbody id="tableBody">
            </tbody>
        </table>
    </div>
    <button
        class="btn"
        style="
            width:100%;
            margin-top:10px;
            border-style:dashed;
            padding:12px
        "
        onclick="addRow()">

        + Add a new line

    </button>
</div>
</div>
<script>
// ============================================================
// JAVASCRIPT VARIABLES
// ============================================================

let delayUs = 100000;

let t = 4;

let rowCount = 4;

let patterns = [
    [1,1,1,1],
    [1,0,0,0],
    [1,0,0,0],
    [1,1,1,1],
    [1,0,0,0],
    [1,0,0,0],
    [1,1,1,1]
];

let cur = 0;


// ============================================================
// INITIALIZE
// ============================================================

function init(){

    let saved =
        localStorage.getItem("mech-v5");


    if(saved){

        try{

            let d =
                JSON.parse(saved);

            delayUs = d.delayUs;
            t = d.t;
            rowCount = d.rowCount;
            patterns = d.patterns;

        }
        catch(e){

            console.log(e);

        }

    }


    render();

    updateUI();


    fetch("/api/get")

        .then(r => r.json())

        .then(d => {

            if(d.delayUs !== undefined){

                delayUs = d.delayUs;

                t = d.t;

                rowCount = d.rowCount;

                patterns = d.patterns;

                render();

                updateUI();

            }

        });

}


// ============================================================
// LOCAL STORAGE
// ============================================================

function saveLocal(){

    localStorage.setItem(
        "mech-v5",

        JSON.stringify({

            delayUs,
            t,
            rowCount,
            patterns

        })

    );

}


// ============================================================
// UPDATE UI
// ============================================================

function updateUI(){

    document.getElementById(
        "delayDisplay"
    ).textContent =
        String(delayUs).padStart(7,"0");


    document.getElementById(
        "delayLabel"
    ).textContent =
        delayUs + "µs";


    document.getElementById(
        "delayInput"
    ).value =
        delayUs;


    document.getElementById(
        "tI"
    ).value =
        t;


    document.getElementById(
        "stepInfo"
    ).textContent =
        "STEP " + cur + "/" + t;


    let pinV =
        document.getElementById("pinV");

    pinV.innerHTML = "";


    let binary = "";


    for(let i = 0; i < 7; i++){

        let value =
            patterns[i]
            ? patterns[i][cur] || 0
            : 0;


        binary += value;


        pinV.innerHTML += `

            <div style="text-align:center">

                <div
                    class="dot ${value ? "on" : ""}">
                </div>

                <div style="
                    font-size:10px;
                    margin-top:4px;
                    color:${value ? "#f59e0b" : "#555"}
                ">
                    ${value}
                </div>

            </div>

        `;

    }


    document.getElementById(
        "binPrev"
    ).textContent =
        binary +
        " | 21,20,10,9,8,7,6";


    document.getElementById(
        "liveFill"
    ).style.width =
        ((cur / t) * 100) + "%";


    saveLocal();

}


// ============================================================
// CHANGE DELAY
// ============================================================

function changeDelay(value){

    delayUs += value;


    if(delayUs < 1)
        delayUs = 1;


    if(delayUs > 1000000)
        delayUs = 1000000;


    updateUI();

}


// ============================================================
// DIRECT DELAY INPUT
// ============================================================

document
.getElementById("delayInput")
.addEventListener("change", function(){

    let value =
        parseInt(this.value);


    if(isNaN(value))
        value = 100000;


    value =
        Math.max(
            1,
            Math.min(
                1000000,
                value
            )
        );


    delayUs = value;

    updateUI();

});


// ============================================================
// RENDER TABLE
// ============================================================

function render(){

    let tb =
        document.getElementById("tableBody");


    tb.innerHTML = "";


    for(let r = 0; r < rowCount; r++){

        let tr =
            document.createElement("tr");


        let html = `

            <td style="
                font-size:12px;
                color:#666
            ">
                ${r}
            </td>

        `;


        for(let c = 0; c < 7; c++){

            let value =
                patterns[c]
                ? patterns[c][r] || 0
                : 0;


            html += `

                <td>

                    <div
                        class="tog ${value ? "on" : ""}"
                        onclick="
                            toggleCell(${c},${r})
                        "
                    >

                        <div class="kn">
                            ${value}
                        </div>

                    </div>

                </td>

            `;

        }


        html += `

            <td>

                <button
                    class="btn"
                    onclick="
                        deleteRow(${r})
                    ">
                    X
                </button>

            </td>

        `;


        tr.innerHTML = html;


        tb.appendChild(tr);

    }

}


// ============================================================
// TOGGLE PATTERN CELL
// ============================================================

function toggleCell(c,r){

    patterns[c][r] =
        patterns[c][r]
        ? 0
        : 1;


    render();

    updateUI();

}


// ============================================================
// CHANGE ROW COUNT
// ============================================================

function changeT(value){

    let newT =
        Math.min(
            100,
            Math.max(
                1,
                t + value
            )
        );


    document.getElementById(
        "tI"
    ).value =
        newT;

}


// ============================================================
// APPLY ROW COUNT
// ============================================================

function applyT(){

    let newT =
        parseInt(
            document.getElementById("tI").value
        ) || 1;


    newT =
        Math.min(
            100,
            Math.max(
                1,
                newT
            )
        );


    if(newT < rowCount){

        if(
            !confirm(
                "Rows අඩු කිරීමේදී data remove වේ. Continue?"
            )
        ){

            return;

        }


        for(let c = 0; c < 7; c++){

            patterns[c] =
                patterns[c].slice(
                    0,
                    newT
                );

        }

    }


    else if(newT > rowCount){

        for(let c = 0; c < 7; c++){

            for(
                let i = rowCount;
                i < newT;
                i++
            ){

                patterns[c].push(0);

            }

        }

    }


    rowCount = newT;

    t = newT;

    cur = 0;


    render();

    updateUI();

}


// ============================================================
// ADD NEW ROW
// ============================================================

function addRow(){

    for(let c = 0; c < 7; c++){

        patterns[c].push(0);

    }


    rowCount++;

    t = rowCount;

    cur = 0;


    render();

    updateUI();

}


// ============================================================
// DELETE ROW
// ============================================================

function deleteRow(r){

    if(rowCount <= 1)
        return;


    for(let c = 0; c < 7; c++){

        patterns[c].splice(
            r,
            1
        );

    }


    rowCount--;

    t = rowCount;


    if(cur >= rowCount)
        cur = 0;


    render();

    updateUI();

}


// ============================================================
// EXPORT
// ============================================================

function exportData(){

    let data = {

        delayUs,
        t,
        rowCount,
        patterns,

        pins:[
            21,
            20,
            10,
            9,
            8,
            7,
            6
        ],

        irPin:5

    };


    let blob =
        new Blob(
            [
                JSON.stringify(
                    data,
                    null,
                    2
                )
            ],
            {
                type:"application/json"
            }
        );


    let url =
        URL.createObjectURL(blob);


    let a =
        document.createElement("a");


    a.href = url;

    a.download =
        "seq.json";


    a.click();

}


// ============================================================
// IMPORT
// ============================================================

function importData(event){

    let file =
        event.target.files[0];


    if(!file)
        return;


    let reader =
        new FileReader();


    reader.onload =
        function(e){

            let data =
                JSON.parse(
                    e.target.result
                );


            delayUs =
                data.delayUs;


            t =
                data.t;
            rowCount =
                data.rowCount;
            patterns =
                data.patterns;
            cur = 0;
            render();
            updateUI();

        };
    reader.readAsText(file);
}

function saveESP(){
    fetch(
        "/api/save",
        {
            method:"POST",

            headers:{
                "Content-Type":
                    "application/json"
            },
            body:
                JSON.stringify({
                    delayUs,
                    t,
                    rowCount,
                    patterns
                })
        }
    )
    .then(r => r.text())
    .then(message => {
        alert(message);
    });
}

init();
</script>
</body>
</html>
)rawliteral";

void handleRoot() {
    server.send_P(
        200,
        "text/html",
        index_html
    );

}


// ============================================================
// SEND CURRENT DATA
// ============================================================
void handleGet() {

    String json = "{";

    json += "\"delayUs\":";
    json += String(delayUs);

    json += ",\"t\":";
    json += String(t);

    json += ",\"rowCount\":";
    json += String(rowCount);

    json += ",\"patterns\":[";


    for(int c = 0; c < 7; c++){
        json += "[";
        for(int r = 0; r < rowCount; r++){
            json +=
                String(patterns[c][r]);
            if(r < rowCount - 1)
                json += ",";
        }
        json += "]";
        if(c < 6)
            json += ",";
    }

    json += "]}";
    server.send(
        200,
        "application/json",
        json
    );

}

// ============================================================
// SAVE DATA
// ============================================================
void handleSave() {

    String body =
        server.arg("plain");

    int p =
        body.indexOf("\"delayUs\"");

    if(p >= 0){
        delayUs =
            strtoul(
                body
                .substring(
                    body.indexOf(":", p) + 1
                )
                .c_str(),
                nullptr,
                10
            );
    }
    
    p =
        body.indexOf("\"t\"");
    if(p >= 0){
        t =
            body
            .substring(
                body.indexOf(":", p) + 1
            )
            .toInt();
    }
    p =
        body.indexOf("\"rowCount\"");
    if(p >= 0){
        rowCount =
            body
            .substring(
                body.indexOf(":", p) + 1
            )
            .toInt();
    }

    // -------------------------------
    // Patterns
    // -------------------------------
    int patternStart =
        body.indexOf("\"patterns\"");

    if(patternStart >= 0){
        int col = 0;
        int row = 0;
        for(int c = 0; c < 7; c++){
            for(int r = 0; r < MAX_ROWS; r++){
                patterns[c][r] = 0;
            }
        }
        int start =
            body.indexOf(
                "[[",
                patternStart
            );
        for(
            int i = start;
            i < body.length() && col < 7;
            i++
        ){
            char ch =
                body[i];
            if(ch == '0' || ch == '1'){
                patterns[col][row] =
                    ch - '0';
                row++;
                if(row >= rowCount){
                    row = 0;
                    col++;
                }
            }
        }
    }

    if(delayUs < 1)
        delayUs = 1;
    if(delayUs > 1000000)
        delayUs = 1000000;
    if(t < 1)
        t = 1;
    if(t > rowCount)
        t = rowCount;
    if(rowCount < 1)
        rowCount = 1;
    if(rowCount > MAX_ROWS)
        rowCount = MAX_ROWS;

    // -------------------------------
    // Save Preferences
    // -------------------------------
    prefs.begin(
        "seq",
        false
    );
    prefs.putULong(
        "delayUs",
        delayUs
    );
    prefs.putInt(
        "t",
        t
    );
    prefs.putInt(
        "rows",
        rowCount
    );
    prefs.end();
    server.send(
        200,
        "text/plain",
        "Saved! delay=" +
        String(delayUs) +
        "us t=" +
        String(t) +
        " rows=" +
        String(rowCount)
    );
}


// ============================================================
// APPLY CURRENT SEQUENCE STEP
// ============================================================
void applyCurrentStep() {
    for(int i = 0; i < 7; i++){
        digitalWrite(
            pins[i],
            patterns[i][curStep]
            ? HIGH
            : LOW
        );
    }
}


// ============================================================
// START SEQUENCE FROM BEGINNING
// ============================================================
void startSequence() {
    curStep = 0;
    sequenceRunning = true;
    lastStepMicros =
        micros();
    applyCurrentStep();
}


// ============================================================
// CHECK IR SENSOR
// ============================================================
void checkIR() {
    bool irState =
        digitalRead(IR_PIN);
    if(
        lastIRState == HIGH &&
        irState == LOW
    ){
        startSequence();
    }
    lastIRState =
        irState;
    if(irState == LOW){
    }
}

void runSequence() {
    if(!sequenceRunning)
        return;
    unsigned long now =
        micros();
    if(
        (unsigned long)(
            now - lastStepMicros
        ) >= delayUs
    ){
        lastStepMicros =
            now;
        curStep++;
        if(curStep >= t){
            sequenceRunning =
                false;

            curStep = 0;
            for(int i = 0; i < 7; i++){
                digitalWrite(
                    pins[i],
                    LOW
                );
            }
            return;
        }
        applyCurrentStep();
    }
}

void setup() {
    Serial.begin(115200);
    for(int i = 0; i < 7; i++){
        pinMode(
            pins[i],
            OUTPUT
        );
        digitalWrite(
            pins[i],
            LOW
        );
    }
    pinMode(
        IR_PIN,
        INPUT_PULLUP
    );

    lastIRState =
        digitalRead(IR_PIN);
    for(int c = 0; c < 7; c++){
        for(int r = 0; r < MAX_ROWS; r++){
            patterns[c][r] = 0;
        }
    }
    int defaultPattern[7][4] = {
        {1,1,1,1},
        {1,0,0,0},
        {1,1,1,1},
        {1,0,0,0},
        {1,1,1,1},
        {1,0,0,0},
        {1,1,1,1}

    };
    for(int c = 0; c < 7; c++){
        for(int r = 0; r < 4; r++){
            patterns[c][r] =
                defaultPattern[c][r];
        }
    }
    prefs.begin(
        "seq",
        true
    );
    delayUs =
        prefs.getULong(
            "delayUs",
            100000
        );
    t =
        prefs.getInt(
            "t",
            4
        );
    rowCount =
        prefs.getInt(
            "rows",
            4
        );
    prefs.end();
    if(delayUs < 1)
        delayUs = 1;
    if(delayUs > 1000000)
        delayUs = 1000000;
    if(t < 1)
        t = 1;
    if(rowCount < 1)
        rowCount = 1;
    if(rowCount > MAX_ROWS)
        rowCount = MAX_ROWS;
    if(t > rowCount)
        t = rowCount;

    WiFi.begin(
        ssid,
        password
    );

    int attempts = 0;
    while(
        WiFi.status() != WL_CONNECTED &&
        attempts < 20
    ){
        delay(500);
        attempts++;
    }
    if(
        WiFi.status() != WL_CONNECTED
    ){
        WiFi.softAP(
            "MECH-SEQ-S3",
            "12345678"
        );
    }
    
    server.on(
        "/",
        handleRoot
    );
    server.on(
        "/api/get",
        handleGet
    );
    server.on(
        "/api/save",
        HTTP_POST,
        handleSave
    );
    server.begin();
    Serial.println();
    Serial.println(
        "MECH-SEQ STARTED"
    );
    Serial.print(
        "IR PIN: GPIO"
    );
    Serial.println(IR_PIN);
    Serial.print(
        "Delay: "
    );
    Serial.print(delayUs);
    Serial.println(
        " us"
    );
}

void loop() {
    // Web server
    server.handleClient();
    // Check IR trigger
    checkIR();
    // Run sequence
    runSequence();
}
