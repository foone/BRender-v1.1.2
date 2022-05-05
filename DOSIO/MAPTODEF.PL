while(<>) {
	last if(/Publics by Value/);
}

while(<>) {
	print "#define SYM$2 0x$1\n" if(process && /[0-9A-F]{4}:([0-9A-F]{4})\s+(\w+)/);
}
