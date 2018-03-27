let canvasWidth = 0;
let canvasHeight = 0;

function fixSize(ctx) {
	const canvas = ctx.canvas;

	const heightAvail = window.innerHeight-$("#navBar").outerHeight(true)-$("#footer").outerHeight(true);
	const widthAvail = $("#content").width();
	canvasWidth = Math.min(heightAvail, widthAvail);
	canvasHeight = canvasWidth;
	canvas.width = canvasWidth;
	canvas.height = canvasHeight;
}

let loading = false;

let grid = null;
let highestGridVal = 0;

function animateLoading(ctx, theta) {

	if(!loading) {
		drawGrid(ctx);
		return;
	}

	ctx.fillStyle="#EEEEEE";
	ctx.fillRect(0, 0, canvasWidth, canvasHeight);

	ctx.lineWidth = 5;
	const sin = Math.sin(theta)*50;
	const cos = Math.cos(theta)*50;
	let grad = ctx.createLinearGradient(canvasWidth/2-sin, canvasHeight/2+cos, canvasWidth/2+sin, canvasHeight/2-cos);
	grad.addColorStop("0","#000000FF");
	grad.addColorStop("1","#00000000");
	ctx.strokeStyle=grad;

	ctx.beginPath();
	ctx.arc(canvasWidth/2, canvasHeight/2, 50, theta-Math.PI/2, theta+Math.PI);
	ctx.stroke();

	requestAnimationFrame(()=>animateLoading(ctx, theta+0.04));
}

function drawGrid(ctx) {
	fixSize(ctx);
	if(loading)
		return;

	if(grid === null) {
		loadGridData();
		animateLoading(ctx, 0);
		return;
	}
	else {
		ctx.fillStyle="#EEEEEE";
		ctx.fillRect(0, 0, canvasWidth, canvasHeight);

		const gridWidth = grid['width'];
		const gridHeight = grid['height'];
		for(let x = 0; x < gridWidth; x++) {
			for(let y = 0; y < gridHeight; y++) {
				const index = x+y*gridWidth;
				const data = grid['data'][index];
				const color = data > 0 ? (data/highestGridVal*0.8)+0.2 : 0.0;

				ctx.fillStyle = `rgba(${0x45}, ${0x27}, ${0xA0}, ${color})`;
				ctx.fillRect(canvasWidth/gridWidth*x, canvasHeight/gridHeight*y, canvasWidth/gridWidth, canvasHeight/gridHeight);
			}
		}

		ctx.strokeStyle="#AAAAAA";
		ctx.lineWidth=1;
		for(let x = 1; x < gridWidth; x++) {
			ctx.beginPath();
			const l = canvasWidth/gridWidth*x;
			ctx.moveTo(l, 0);
			ctx.lineTo(l, canvasHeight);
			ctx.stroke();
		}
		for(let y = 1; y < gridHeight; y++) {
			ctx.beginPath();
			const l = canvasHeight/gridHeight*y;
			ctx.moveTo(0, l);
			ctx.lineTo(canvasWidth, l);
			ctx.stroke();
		}
	}
}

function loadGridData() {
	loading = true;
	$.getJSON("grid.json").done(( data )=> {
		grid = data;
		highestGridVal = Math.max(...grid['data']);
		loading = false;
	}).fail(()=>{
		alert("Failed getting data from server, trying again in 5 seconds");
		setTimeout(loadGridData, 5000);
	});
}

const ctx = document.getElementById("dude_grid").getContext("2d");
function refresh() {
	grid = null;
	drawGrid(ctx);
}

function start() {
	$(window).resize(()=>drawGrid(ctx));
	refresh();
}

$(document).ready(start);

