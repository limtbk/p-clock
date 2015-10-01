px = 190;
py = 70;
nx = 17;
ny = 5;
sl = 8.5;
cp = 10;
frth = 8;
wth = 4;

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
		cube([px,py,frth]);

		for (i = [0:nx-1]) {
			for (j = [0:ny-1]) {
				translate([i*cp + (px-((nx-1)*cp+sl))/2, j*cp + (py-((ny-1)*cp+sl))/2, 0]) {
					cube(sl,sl,10);
				}
				
			}	
		}
	}

	translate([5,5,frth]) {
		union() {
			cylinder(10,2,2);
			cylinder(1,4,4);
		}
	}
	translate([5,py-5,frth]) {
		union() {
			cylinder(10,2,2);
			cylinder(1,4,4);
		}
	}
	translate([px-5,5,frth]) {
		union() {
			cylinder(10,2,2);
			cylinder(1,4,4);
		}
	}
	translate([px-5,py-5,frth]) {
		union() {
			cylinder(10,2,2);
			cylinder(1,4,4);
		}
	}

	translate([-wth,-wth,-4]) {
        difference() {
            cube([px+2*wth,py+2*wth,50]);
            translate([wth,wth,0]) {
                cube([px,py,50]);
            }
        }
    }
}

