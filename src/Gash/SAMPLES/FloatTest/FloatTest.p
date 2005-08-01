class Hello extends Object
{

	proc Main()
	{
		Float f
		Float.New(f)
		Float.fromString(f, '3.14159')
		Integer n
		Asm.New(n)
		Asm.Set(n, 6)
		Float ff
		Float.New(ff)
		Float.fromInteger(ff, n)
		Float.add(f, ff)
		String s
		String.New(s)
		Float.toString(f, s)
		Popups.MessageBox(s, 'Hello!!!!!!!!!!!!!!!!')
	}

}


