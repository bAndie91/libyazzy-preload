
__attribute__ ((constructor)) void f()
{
	setvbuf (stdout, NULL, _IOLBF, 0);
}
