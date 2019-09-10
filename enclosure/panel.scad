encoder_zero_x = 100-(45/2)-45-20;
encoder_zero_y = 130-35;
encoder_hole_size = 6;

difference() {
    cube([200, 180, 3]);
    translate([100-(75/2), 40, -5])
        cube([75, 48, 20]);
    translate([100-(45/2)-45, 129, -2])
        cylinder(h=7, r1=encoder_hole_size, r2=encoder_hole_size);
    translate([100-(45/2), 129, -2])
        cylinder(h=7, r1=encoder_hole_size, r2=encoder_hole_size);
    translate([100+(45/2), 130, -2])
        cylinder(h=7, r1=encoder_hole_size, r2=encoder_hole_size);
    translate([100+(45/2)+45, 130, -2])
        cylinder(h=7, r1=encoder_hole_size, r2=encoder_hole_size);   
}

translate([75.5, 32.5, 3]) hook();
translate([114.5, 32.5, 3]) hook();
translate([81.5, 95.5, 3]) rotate([0, 0, 180]) hook();

translate([encoder_zero_x+10, encoder_zero_y+45, 0])
        standoff();
translate([encoder_zero_x+10, encoder_zero_y+5, 0])
        standoff();
translate([encoder_zero_x+165, encoder_zero_y+45, 0])
        standoff();
translate([encoder_zero_x+165, encoder_zero_y+5, 0])
        standoff();

module hook() {
    cube([6, 3, 7.2]);
    translate([6, 7, 7.2])
    rotate([90, 0, 0])
    rotate([0, 270, 0])
    linear_extrude(height=6) {
        polygon([[0,0], [7,0], [7,3], [0, 0]]);
    }      
}

module standoff() {
    difference() {
        cylinder(h=24, r1=3, r2=3);
        translate([0, 0, 15])
            cylinder(h=10, r1=1.5, r2=1.5);
    }
}