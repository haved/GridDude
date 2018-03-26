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

function drawLoading(ctx, theta) {
	ctx.fillStyle="#EEEEEE";
	ctx.fillRect(0, 0, width, height);

	ctx.lineWidth = 5;
	const sin = Math.sin(theta)*60;
	const cos = Math.cos(theta)*60;
	let grad = ctx.createLinearGradient(width/2-sin, height/2+cos, width/2+sin, height/2-cos);
	grad.addColorStop("0","black");
	grad.addColorStop("1","white");
	ctx.strokeStyle=grad;

	ctx.beginPath();
	ctx.arc(width/2, height/2, 50, theta-Math.PI/2, theta+Math.PI);
	ctx.stroke();

	requestAnimationFrame(()=>drawLoading(ctx, theta+0.04));
}


function drawGrid(ctx) {
	fixSize(ctx);
	drawLoading(ctx, 0);
}

function start() {
	$(window).resize(function() {drawGrid(ctx);});

	const ctx = document.getElementById("dude_grid").getContext("2d");
	drawGrid(ctx);
}

$(document).ready(start);

