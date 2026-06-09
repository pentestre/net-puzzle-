let g = null;
let canvas = document.getElementById('canvas');
let initialOrientations = []; 

const EMPTY = 0;
const ENDPOINT = 1;
const SEGMENT = 2;
const CORNER = 3;
const TEE = 4;
const CROSS = 5;
const NB_SHAPES = 6;

const NORTH = 0;
const EAST = 1;
const SOUTH = 2;
const WEST = 3;
const NB_DIRS = 4;

let _square2str = [
    [" ", " ", " ", " "],
    ["^", ">", "v", "<"],
    ["|", "-", "|", "-"],
    ["└", "┌", "┐", "┘"],
    ["┴", "├", "┬", "┤"],
    ["+", "+", "+", "+"],
];

function square2str(s, o) {
    return _square2str[s][o];
}

function drawGame(g, canvas) {
    const ctx = canvas.getContext('2d');
    const nb_rows = Module._nb_rows(g);
    const nb_cols = Module._nb_cols(g);

    const cell_width = canvas.width / nb_cols;
    const cell_height = canvas.height / nb_rows;

    ctx.textAlign = 'center';
    ctx.textBaseline = 'middle';
    ctx.font = Math.min(cell_width, cell_height) * 0.6 + "px Arial";

    ctx.clearRect(0, 0, canvas.width, canvas.height);

    for (let row = 0; row < nb_rows; row++) {
        for (let col = 0; col < nb_cols; col++) {
            const s = Module._get_piece_shape(g, row, col);
            const o = Module._get_piece_orientation(g, row, col);
            const symbol = square2str(s, o);

            const x = col * cell_width + cell_width / 2;
            const y = row * cell_height + cell_height / 2;

            ctx.fillText(symbol, x, y);
            ctx.strokeStyle = 'black';
            ctx.lineWidth = 1;
            ctx.strokeRect(col * cell_width, row * cell_height, cell_width, cell_height);
        }
    }
}

function handleClick(event) {
    const rect = canvas.getBoundingClientRect();
    const x = event.clientX - rect.left;
    const y = event.clientY - rect.top;
    const nb_rows = Module._nb_rows(g);
    const nb_cols = Module._nb_cols(g);
    const cell_width = canvas.width / nb_cols;
    const cell_height = canvas.height / nb_rows;

    const col = Math.floor(x / cell_width);
    const row = Math.floor(y / cell_height);

    Module._play_move(g, row, col, 1); 
    drawGame(g, canvas);
}

function adjustCanvasSize() {
    const maxCanvasWidth = window.innerWidth * 0.8;
    const maxCanvasHeight = window.innerHeight * 0.7;
    const cellSize = Math.min(maxCanvasWidth / 4, maxCanvasHeight / 4, 80);

    canvas.width = cellSize * 4;
    canvas.height = cellSize * 4;
}

function saveInitialOrientations() {
    initialOrientations = [];
    for (let row = 0; row < 4; row++) {
        initialOrientations[row] = [];
        for (let col = 0; col < 4; col++) {
            initialOrientations[row][col] = Module._get_piece_orientation(g, row, col);
        }
    }
}

function resetToInitialState() {
    if (!initialOrientations.length) return;
    
    for (let row = 0; row < 4; row++) {
        for (let col = 0; col < 4; col++) {
            const currentOrientation = Module._get_piece_orientation(g, row, col);
            const targetOrientation = initialOrientations[row][col];
            const turnsNeeded = (targetOrientation - currentOrientation + 4) % 4;
            
            if (turnsNeeded > 0) {
                Module._play_move(g, row, col, turnsNeeded);
            }
        }
    }
}

function start() {
    console.log("call start routine");
    g = Module._new_random(4, 4, false, 0, 0);
    Module._restart(g); 
    saveInitialOrientations(); 
    
    adjustCanvasSize();
    drawGame(g, canvas);

    canvas.removeEventListener('click', handleClick);
    canvas.addEventListener('click', handleClick);
}

function setupButtons() {
    document.getElementById('restartBtn').onclick = function() {
        console.log("Restart clicked");
        resetToInitialState();
        drawGame(g, canvas);
    };

    document.getElementById('undoBtn').onclick = function() {
        console.log("Undo clicked");
        Module._undo(g);
        drawGame(g, canvas);
    };

    document.getElementById('redoBtn').onclick = function() {
        console.log("Redo clicked");
        Module._redo(g);
        drawGame(g, canvas);
    };

    document.getElementById('solveBtn').onclick = function() {
        console.log("Solve clicked");
        setTimeout(() => {
            Module._solve(g);
            drawGame(g, canvas);
        }, 100); 
    };

    document.getElementById('randomBtn').onclick = function() {
        console.log("Random clicked");
        Module._restart(g);
        saveInitialOrientations(); 
        drawGame(g, canvas);
    };

    window.addEventListener('resize', () => {
        if (g) {
            adjustCanvasSize();
            drawGame(g, canvas);
        }
    });
}

Module.onRuntimeInitialized = () => {
    setupButtons();
    start();
};