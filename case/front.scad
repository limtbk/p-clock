px = 205;
py = 68;
nx = 17;
ny = 5;
sl = 10;
cp = 11;

/*
translate([0,0,8]) {
difference() {
	cube([px,py,5]);
	translate([4,5,-2.5]) {
		cylinder(10,2,2);
	}
	translate([4,py-5,-2.5]) {
		cylinder(10,2,2);
	}
	translate([px-4,5,-2.5]) {
		cylinder(10,2,2);
	}
	translate([px-4,py-5,-2.5]) {
		cylinder(10,2,2);
	}
}
}
*/
translate([0,0,0]) {
	difference() {
		cube([px,py,7]);

		for (i = [0:nx-1]) {
			for (j = [0:ny-1]) {
				translate([i*cp + (px-((nx-1)*cp+sl))/2, j*cp + (py-((ny-1)*cp+sl))/2, 0]) {
					cube(sl,sl,10);
				}
				
			}	
		}
	}

	translate([4,5,7]) {
		union() {
			cylinder(10,2,2);
			cylinder(1,4,4);
		}
	}
	translate([4,py-5,7]) {
		union() {
			cylinder(10,2,2);
			cylinder(1,4,4);
		}
	}
	translate([px-4,5,7]) {
		union() {
			cylinder(10,2,2);
			cylinder(1,4,4);
		}
	}
	translate([px-4,py-5,7]) {
		union() {
			cylinder(10,2,2);
			cylinder(1,4,4);
		}
	}


}

