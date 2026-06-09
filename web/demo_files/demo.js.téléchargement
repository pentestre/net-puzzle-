// demo.js

Module.onRuntimeInitialized = () => { start(); }

const EMPTY = 0;      // empty shape
const ENDPOINT = 1;   // endpoint shape
const SEGMENT = 2;    // segment shape
const CORNER = 3;     // corner shape
const TEE = 4;        // tee shape
const CROSS = 5;      // cross shape
const NB_SHAPES = 6;  // nb of shapes

const NORTH = 0;      // north
const EAST = 1;       // east
const SOUTH = 2;      // south
const WEST = 3;       // west 
const NB_DIRS = 4;    // nb of directions


let _square2str = [
    [" ", " ", " ", " "],  // empty
    ["^", ">", "v", "<"],  // endpoint
    ["|", "-", "|", "-"],  // segment
    ["└", "┌", "┐", "┘"],  // corner
    ["┴", "├", "┬", "┤"],  // tee
    ["+", "+", "+", "+"],  // cross
];

function square2str(s, o) {
    return _square2str[s][o];
}


function printGame(g) {
    var text = "";
    var nb_rows = Module._nb_rows(g);
    var nb_cols = Module._nb_cols(g);
    for (var row = 0; row < nb_rows; row++) {
        for (var col = 0; col < nb_cols; col++) {
            var s = Module._get_piece_shape(g, row, col);
            var o = Module._get_piece_orientation(g, row, col);
            text += square2str(s, o);
        }
        text += "\n";
    }

    // put this text in <div> element with ID 'result'
    var elm = document.getElementById('result');
    elm.innerHTML = text;
}

function start() {
    console.log("call start routine");
    var g = Module._new_default();
    Module._play_move(g, 0, 0, 1);
    printGame(g);
    Module._delete(g);
}

