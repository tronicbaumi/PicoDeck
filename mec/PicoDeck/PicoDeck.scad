// PicoDeck.scad - Pultgehaeuse (Keilform) mit separater Bodenplatte
//
// Oberteil: vorne 20 mm, hinten 60 mm hoch, 120 mm breit.
// Alle Kanten ausser an der Bodenplatte sind mit R3 verrundet.
// Bodenplatte: wird mit 4 Schrauben (M3) von unten in das Oberteil geschraubt
// und rastet zusaetzlich mit Schnapp-Noppen ein.
//
// Koordinaten: x = Breite, y = Tiefe (0 = vorne), z = Hoehe

$fn = 48;

// ---- Auswahl ----
part = "both";   // [both:Oberteil + Bodenplatte nebeneinander, assembly:zusammengebaut, top:nur Oberteil, bottom:nur Bodenplatte]

// ---- Gehaeuse ----
W  = 120;   // Breite
D  = 75;   // Tiefe
Hf = 20;    // Hoehe vorne
Hb = 60;    // Hoehe hinten
r  = 3;     // Kantenradius
t  = 2.4;   // Wandstaerke

// ---- Bodenplatte / Verbindung ----
plate_t = 3;      // Dicke der Bodenplatte
lip_h   = 3;      // Hoehe der Zentrierlippe
lip_w   = 1.6;    // Wandstaerke der Lippe
tol     = 0.25;   // Spiel zwischen Lippe und Gehaeusewand
post_d  = 7;      // Schraubdom Aussendurchmesser
post_h_d = 2.6;   // Kernloch fuer M3 (selbstschneidend)
screw_d = 3.4;    // Durchgangsloch in der Bodenplatte
rib_w   = 2;      // Stegbreite zwischen Schraubdom und Wand
// ---- Montage fuer Lochrasterplatine 70 x 50 mm (Abstandsbolzen auf der Bodenplatte) ----
pcb_size    = [70, 50];   // Platine (x = Breite, y = Tiefe)
pcb_hole_in = 2;          // Lochmitte von den Platinenkanten
pcb_post_d  = 6;          // Durchmesser der Bolzen
pcb_post_h  = 6;          // Hoehe ueber der Bodenplatte
pcb_screw_d = 2.6;        // Kernloch fuer M3 (selbstschneidend)
pcb_back    = 3;          // Abstand der Platinenhinterkante zur Innenseite der Rueckwand
// ---- USB-C Durchbruch in der Rueckwand (Arduino Micro auf Stiftleisten ueber der Lochrasterplatine) ----
pcb_t        = 1.6;    // Dicke der Lochrasterplatine
header_h     = 8.5;    // Hoehe der Buchsenleisten (Platinenoberseite bis Micro-Unterseite)
micro_t      = 1.6;    // Dicke der Arduino-Micro-Platine
usb_conn_h   = 3.2;    // Hoehe der USB-C-Buchse ueber der Micro-Platine
usb_cut      = [13, 7.5];   // Durchbruch (Breite, Hoehe): Stecker-Spritzling ca. 12.4 x 6.5 mm
usb_x        = 0;      // seitlicher Versatz zur Gehaeusemitte
post_in = 7.5; // Abstand der Schraubdome von den Aussenkanten

// ---- Bedienelemente (Position entlang der Schraegen, s = Abstand von der Vorderkante) ----
btn_d   = 12.3;
btn_s   = 12;
btn_pitch = 24;
enc_d   = 7.3;
enc_s   = 36;
enc_pitch = 44;
// OLED 0.96" 128x64 laut Datenblatt: Platine 27.3 x 27.3 (+-0.5), aktive Flaeche 21.74 x 10.86,
// 4 Loecher 2.5 mm im Raster 20.7 x 23.2. Das Modul sitzt mit dem Pinheader zur Rueckseite (hoeheres s).
oled_s      = 60;
oled_win    = [22.8, 12.00];   // Ausschnitt (Breite, Laenge entlang der Schraege) = aktive Flaeche + ca. 0.35 Rand
oled_mod    = [28.3, 28.3];   // Modultasche = Platine 27.3 + Toleranz 0.5 + Spiel
oled_win_dy = 1.76;           // aktive Flaeche liegt 1.76 mm Richtung Pinheader aus der Platinenmitte
oled_frame  = 1.6;            // Wand des Halterahmens
oled_hole   = [20.7, 23.2];   // Lochraster der Befestigungsloecher
oled_boss_d = 3.4;            // Auflage um die Loecher (Pad am Modul 3.5)
oled_peg_d  = 2.2;            // Zapfen durch die 2.5-mm-Loecher
oled_pcb_t  = 1.0;            // Platinendicke (Annahme)
oled_boss_h = 1.6;            // Dach-Innenseite bis Platinenvorderseite = Glasdicke ca. 1.4 (Annahme) + 0.2 Spiel

a = atan((Hb - Hf) / D);      // Neigungswinkel der Oberseite

// Oberflaeche: Hoehe bei Tiefe y
function ztop(y) = Hf + tan(a) * y;
// Punkt auf der Schraege im Abstand s von der Vorderkante
function sl_y(s) = s * cos(a);
function sl_z(s) = Hf + s * sin(a);

// Bolzenpositionen: Platine mittig in x, hinten an der Rueckwand
function pcb_posts() =
    let (x0 = W / 2 - pcb_size[0] / 2,
         y1 = D - t - pcb_back,
         y0 = y1 - pcb_size[1])
    [for (x = [x0 + pcb_hole_in, x0 + pcb_size[0] - pcb_hole_in],
          y = [y0 + pcb_hole_in, y1 - pcb_hole_in]) [x, y]];

screws = [[post_in, post_in], [W - post_in, post_in],
          [post_in, D - post_in], [W - post_in, D - post_in]];

if (part == "both") {
    top();
    translate([W + 15, 0, plate_t]) bottom();
} else if (part == "assembly") {
    top();
    bottom();
} else if (part == "top") {
    top();
} else {
    translate([0, 0, plate_t]) bottom();
}

// ================= Oberteil =================

module top()
{
    difference() {
        union() {
            difference() {
                wedge(0, W, 0, D, 0, r, 0);
                wedge(t, W - t, t, D - t, t / cos(a), max(r - t, 0.6), -1);
            }
            // Schraubdome haengen am Dach
            for (p = screws)
                translate([p[0], p[1], 0])
                    cylinder(d = post_d, h = ztop(p[1] - post_d / 2) - 1.2);
            // Stege verbinden die Schraubdome mit Seiten- und Front-/Rueckwand
            intersection() {
                post_ribs(rib_w);
                wedge(t - 0.3, W - t + 0.3, t - 0.3, D - t + 0.3, (t - 0.3) / cos(a), max(r - t + 0.3, 0.6), 0);
            }
        }
        // Schraublocher
        for (p = screws)
            translate([p[0], p[1], -0.1]) cylinder(d = post_h_d, h = 16);
        // Rastnuten
        snap_features(groove = true);
        controls_cut();
        usb_cutout();
    }
    oled_frame();
}

// Stege von jedem Schraubdom zur naechsten Seitenwand (x) und zur Front-/Rueckwand (y)
// (hoch ausgefuehrt, werden durch Dach und Waende begrenzt)
module post_ribs(w)
{
    for (p = screws) {
        wx = p[0] < W / 2 ? 0 : W;
        wy = p[1] < D / 2 ? 0 : D;
        translate([min(p[0], wx), p[1] - w / 2, 0]) cube([abs(p[0] - wx), w, Hb]);
        translate([p[0] - w / 2, min(p[1], wy), 0]) cube([w, abs(p[1] - wy), Hb]);
    }
}

// Keil mit verrundeter Oberseite und ebenem Boden (zb)
// off = senkrechter Versatz der Dachebene nach unten
module wedge(x0, x1, y0, y1, off, rr, zb)
{
    hull() {
        for (x = [x0 + rr, x1 - rr], y = [y0 + rr, y1 - rr]) {
            translate([x, y, ztop(y) - off - rr / cos(a)]) sphere(r = rr);
            translate([x, y, zb]) cylinder(r = rr, h = 0.01);
        }
    }
}

// Lokales Koordinatensystem auf der Schraege: x = Breite, y = entlang der
// Schraege (weg von der Vorderkante), z = Normale nach aussen
module on_slope(s, x)
{
    translate([x, sl_y(s), sl_z(s)]) rotate([a, 0, 0]) children();
}

module controls_cut()
{
    // 4 Taster
    for (i = [-1.5 : 1 : 1.5])
        on_slope(btn_s, W / 2 + i * btn_pitch)
            cylinder(d = btn_d, h = 20, center = true);
    // 2 Encoder
    for (i = [-0.5, 0.5])
        on_slope(enc_s, W / 2 + i * enc_pitch)
            cylinder(d = enc_d, h = 20, center = true);
    // OLED-Fenster
    on_slope(oled_s, W / 2)
        translate([0, oled_win_dy, 0])
            cube([oled_win[0], oled_win[1], 20], center = true);
}

// Hoehe der Mitte der USB-Buchse ueber der Bodenplatte
function usb_z() = pcb_post_h + pcb_t + header_h + micro_t + usb_conn_h / 2;

// USB-C Durchbruch mittig in der Rueckwand (abgerundetes Rechteck)
module usb_cutout()
{
    rr = 1.5;
    translate([W / 2 + usb_x, D - t - 1, usb_z()])
        rotate([-90, 0, 0])
            linear_extrude(t + 2)
                hull() for (sx = [-1, 1], sz = [-1, 1])
                    translate([sx * (usb_cut[0] / 2 - rr), sz * (usb_cut[1] / 2 - rr)]) circle(r = rr);
}

// Halterahmen fuer das OLED-Modul auf der Dachinnenseite
module oled_frame()
{
    on_slope(oled_s, W / 2)
        difference() {
            translate([0, 0, -t - 4])
                linear_extrude(4 + 0.5)
                    square([oled_mod[0] + 2 * oled_frame, oled_mod[1] + 2 * oled_frame], center = true);
            // Modultasche, nach unten offen
            translate([-oled_mod[0] / 2, -oled_mod[1] / 2, -t - 5])
                cube([oled_mod[0], oled_mod[1], 6]);   // bis ins Dach, damit das Fenster frei bleibt
        }
    // Zapfen fuer die 4 Befestigungsloecher des Moduls (Datenblatt: 4x 2,5 mm, 20,7 x 23,2 mm)
    on_slope(oled_s, W / 2)
        for (sx = [-1, 1], sy = [-1, 1])
            translate([sx * oled_hole[0] / 2, sy * oled_hole[1] / 2, -t]) {
                translate([0, 0, -oled_boss_h]) cylinder(d = oled_boss_d, h = oled_boss_h + 0.5);
                translate([0, 0, -oled_boss_h - oled_pcb_t - 0.4]) cylinder(d = oled_peg_d, h = oled_pcb_t + 0.4 + 0.1);
            }
}

// ================= Bodenplatte =================

module bottom()
{
    difference() {
        union() {
            // Platte
            translate([0, 0, -plate_t]) linear_extrude(plate_t) rrect(0, W, 0, D, r);
            // Zentrierlippe (Ring), federt leicht
            difference() {
                linear_extrude(lip_h) lip_outline(t + tol + lip_w, t + tol);
                for (p = screws)
                    translate([p[0], p[1], -0.1]) cylinder(d = post_d + 0.8, h = lip_h + 1);
                translate([0, 0, -0.1]) intersection() {   // Aussparung fuer die Stege
                    post_ribs(rib_w + 0.8);
                    cube([W, D, lip_h + 1]);
                }
            }
            snap_features(groove = false);
            // Abstandsbolzen fuer die Lochrasterplatine
            for (p = pcb_posts())
                translate([p[0], p[1], -0.01]) cylinder(d = pcb_post_d, h = pcb_post_h + 0.01);
        }
        // Gewindekerne der Bolzen (Sackloch von oben)
        for (p = pcb_posts())
            translate([p[0], p[1], pcb_post_h - 5]) cylinder(d = pcb_screw_d, h = 5.1);
        // Senkschrauben von unten
        for (p = screws)
            translate([p[0], p[1], 0]) {
                translate([0, 0, -plate_t - 0.01]) cylinder(d1 = 6.4, d2 = screw_d, h = 1.7);
                translate([0, 0, -plate_t - 0.1]) cylinder(d = screw_d, h = plate_t + 0.2);
            }
    }
}

module rrect(x0, x1, y0, y1, rr)
{
    hull() for (x = [x0 + rr, x1 - rr], y = [y0 + rr, y1 - rr])
        translate([x, y]) circle(r = rr);
}

// Ring zwischen Innenkante i_in (innen) und i_out (aussen)
module lip_outline(i_in, i_out)
{
    difference() {
        rrect(i_out, W - i_out, i_out, D - i_out, max(r - i_out, 0.5));
        rrect(i_in,  W - i_in,  i_in,  D - i_in,  max(r - i_in, 0.3));
    }
}

// Rast-Noppen (Bodenplatte) bzw. Rastnuten (Oberteil) in der Mitte aller 4 Seiten
module snap_features(groove)
{
    bz  = lip_h / 2;
    len = 10;
    lo  = t + tol;       // Aussenkante der Lippe
    for (side = [0 : 3]) {
        // Position auf der jeweiligen Seite, Richtung nach aussen
        if (groove) {
            snap_at(side, t, 0.9, len + 2, bz);
        } else {
            snap_at(side, lo, 0.8, len, bz, -0.2);
        }
    }
}

module snap_at(side, face, rad, len, z, shift = 0)
{
    // face = Abstand der Wandflaeche von der Aussenkante
    // shift: Kugelmitte relativ zur Flaeche (negativ = nach innen)
    d = face - shift;
    if (side == 0) hull() { translate([d, D / 2 - len / 2, z]) sphere(r = rad, $fn = 24);
                            translate([d, D / 2 + len / 2, z]) sphere(r = rad, $fn = 24); }
    if (side == 1) hull() { translate([W - d, D / 2 - len / 2, z]) sphere(r = rad, $fn = 24);
                            translate([W - d, D / 2 + len / 2, z]) sphere(r = rad, $fn = 24); }
    if (side == 2) hull() { translate([W / 2 - len / 2, d, z]) sphere(r = rad, $fn = 24);
                            translate([W / 2 + len / 2, d, z]) sphere(r = rad, $fn = 24); }
    if (side == 3) hull() { translate([W / 2 - len / 2, D - d, z]) sphere(r = rad, $fn = 24);
                            translate([W / 2 + len / 2, D - d, z]) sphere(r = rad, $fn = 24); }
}
