// Bezel for Repackaged R/C + Arduino KAP Controller
// by David Wheeler
// January 2015

// base dimensions in terms of 1/16 inch

function toMm(x) = 25.4 * x / 16;

baseLength = toMm(48);        // overall length (x)
baseWidth = toMm(27);         // overall width (y)
baseThick = toMm(2);          // thickness
baseRadius = toMm(3);         // round radius (and screw offset from edge)
midScrew = toMm(24.5);        // x value of middle screw line
screwDepth = toMm(0.5);       // depth of counter sink on screw locations
screwHeadRadius = toMm(2);    // radius of counter sink (to accom #4 screw head)
drillCenterThick = toMm(0.25);// thickness of material left at center
jsX = toMm(36);               // joystick hole center (x)
jsY = toMm(14);               //                      (y)
lcdX=toMm(13.5);              // center of lcd area (x)
lcdY=toMm(13.5);              // center of lcd area (y)
lcdW=toMm(17);                // width of lcd area (x)
lcdH=toMm(17);                // height of lcd area (y)
jsCenterZ=toMm(-4);           // center of joystick ball (z)
jsSphereR=toMm(8.5);          // joystick ball radius
jsOpeningR = toMm(6.5);       // radius of joystick opening
jsOpeningSides=24;      // 24-sided polygon for joystick opening.
lcdBevelSlope=2;        // slope inside lcd window

// The bezel is constructed as a base plate, rectangular with round corners
// From this we subtract openings for the LCD and joystick.  We also
// subtract countersunk areas for the screw heads.  Each of these subtracted
// volumes is called a punch here.


// Create the base plate, a rectangle with rounded corners
module base(l=baseLength, w=baseWidth, z=baseThick, r=baseRadius) {
	// corners
	for (x = [r, l-r]) {
		for (y = [r, w-r]) {
			translate([x, y, 0]) cylinder(r=r, h=z, $fn=100);
		}
	}

	// plate
	translate([r, 0, 0]) cube([l-2*r, w, z]);
	translate([0,r,0]) cube([l, w-2*r, z]);
}

// The shape of one counter sink.
// A small indentation in the center is to help guide drilling
module screwPunch() {
	translate([0,0,baseThick-screwDepth]) // raise up to leave some material
	cylinder(r=screwHeadRadius, h=baseThick, $fn=100);

	// drill guide
	translate([0,0,drillCenterThick])
	cylinder(r1=0, r2=screwHeadRadius/2,h=baseThick, $fn=100);
}

// The set of all size screw head counter sinks
module screwPunches() {
	for (y = [baseRadius, baseWidth-baseRadius]) {
		for (x = [baseRadius, baseLength-baseRadius]) {
			translate([x, y, 0]) screwPunch();
		}
		translate([midScrew, y, 0]) screwPunch();
	}
}

// Window for joystick.
module jsPunch() {
	translate([jsX,jsY,jsCenterZ]) { 
		cylinder(r=jsOpeningR, h=6*baseThick, $fn=jsOpeningSides);

		// Spherical part of Joystick
		sphere(r=jsSphereR, $fn=100);
	}
}

// Window for LCD
module lcdPunch() {
	lcdSize = lcdW;
	below = 1.0;  // extent of punch below x-y plane
	shrink = below/lcdBevelSlope;
	above = 4.0;  // extent of punch above x-y plane
	expand = above/lcdBevelSlope;
	bottom = lcdSize - shrink;
	top = lcdSize + expand;
	scale = top / bottom;

	echo(top=top);
	echo(bottom=bottom);
	echo(scale=scale);
	echo(version());

	translate([lcdX,lcdY,0])    // position window
	translate([0, 0, -below])
	linear_extrude(height=below+above, convexity=10, scale=scale, slices=100, $fn=100)
	translate([-bottom/2, -bottom/2, 0])
	square(bottom, bottom);
}

// Create bezel as a base with windows and countersinks subtracted
module bezel() {
	difference() {
		base();
		lcdPunch();
		screwPunches();
		jsPunch();
	};	
}

// Make the bezel
bezel();



