# Icons for R/C KAP Controller

A number of icons are built into the software, stored in .xbm format
as C data arrays.  Most of these icons were generated using ImageMagick.
The overall process went like this:

run bash scripts mkicons and mkicons2

These produce all the arcs, the circle and 4 of the 6 mode icons in
gif format in the out/ subdirectory.  They also produces versions of the
arc icons displayed within the circle.  These were to help visualize
how the two elements would look together.

Two additional mode icons, quad.gif and omni.gif, were developed
directly with Gimp.  These are located here, in the graphics folder.

All the GIF files were then converted to .XBM format manually using
Gimp and written to the final/ folder.  In the process, the arc icons
were cropped to their smallest rectangle and the offsets of the crops
noted.  This information was recorded in final/offsets.txt.

Finally, all of the .xbm files and the offset information were
included in kaptx.ino.
