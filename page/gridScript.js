let width = 0;
let height = 0;

function fixSize(ctx) {
	const canvas = ctx.canvas;

	const heightAvail = window.innerHeight-$("#navBar").outerHeight(true)-$("#footer").outerHeight(true);
	const widthAvail = $("#content").width();
	width = Math.min(heightAvail, widthAvail);
	height = width;
	canvas.width = width;
	canvas.height = height;
}

function drawLoading(ctx) {
	ctx.fillRect(0, 0, width, height);
}


function drawGrid(ctx) {
	fixSize(ctx);
	drawLoading(ctx);
}

function start() {
	$(window).resize(function() {drawGrid(ctx);});

	const ctx = document.getElementById("dude_grid").getContext("2d");
	drawGrid(ctx);
}

$(document).ready(start);

