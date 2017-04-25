///c_(hex_color)
//Example: c_($FF00FF) = magenta
return ((argument0 & $FF) << 16) | (argument0 & $FF00) | ((argument0 >> 16) & $FF);
