#!/usr/bin/lua

for i = 0, 255 do
	x = i / 256.0 * math.pi * 2 
	y = math.cos(x)
	print(string.format("%+.7f, ", y))
end
	

