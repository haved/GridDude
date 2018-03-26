let width = 0;
let height = 0;

const container = document.getElementById("content");

function fixSize(ctx) {
	const canvas = ctx.canvas;
	width = Math.min(container.scrollWidth, container.scrollHeight);
	height = width;
	canvas.width = width;
	canvas.height = height;
}

function drawGrid(ctx) {
	fixSize(ctx);
}

function drawLoading(ctx) {
	fixSize(ctx);
	ctx.fillRect(0, 0, width, height);
}

function start() {
	const ctx = document.getElementById("dude_grid").getContext("2d");
	drawLoading(ctx);
}
