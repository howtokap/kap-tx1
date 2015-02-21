# 3D model of the bezel.

##Files:
* bezel.scad : OpenSCAD code to generate the bezel.
* bezel.stl : STL file produced from bezel.scad.  

To 3D print the bezel, load bezel.stl into Makerware or other software.

To modify the bezel, you can open bezel.scad in OpenSCAD:

```bash
$ openscad bezel.scad
```

From there, edit to suit.  Press F5 to update the rendering.  To export a new
.STL file, compile with F6 then perform File -> Export -> Export as STL...
