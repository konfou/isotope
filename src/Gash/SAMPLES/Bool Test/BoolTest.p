class Hello extends Object
{
	proc Main()
	{
		Bool b
		Bool.New(b)
		Asm.Increment(b)
		String s
		String.New(s)
		String.fromInteger(s, b)
		Popups.MessageBox(s, 'Hello!!!!!!!!!!!!!!!!')
	}

}

class Bool extends Integer
{

}

