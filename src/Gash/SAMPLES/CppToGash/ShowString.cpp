// --------------------------------------------------------------
// |                                                            |
// |   This file was generated by Gash.  If you edit it, your   |
// |   changes will be lost when it is regenerated.  If you     |
// |   want to modify this file, you should make your changes   |
// |   to the origional Gash source and regenerate this file.   |
// |                                                            |
// --------------------------------------------------------------

const char* g_xlib_string_ShowString = "\
<Library>\
	<Class Name=\"ShowString\" ID=\"11\" ParID=\"0\" Parent=\"Object\">\
		<Procedure Name=\"New\" ID=\"0\">\
			<Var Name=\"Dest\" Class=\"ShowString\" Mod=\"All\" />\
			<Bin Data=\"14fdffffff0b0000000e10fdffffff0a020000000f00\" />\
		</Procedure>\
		<Procedure Name=\"ShowString\" ID=\"1\">\
			<Var Name=\"str\" Class=\"String\" Mod=\"All\" />\
			<Bin Data=\"0e110e10010000000a030000001303000000010000000f1115020000001a02000000000000000e100100000010020000000c000000000f100100000010fdffffff0a050000000f00\" />\
		</Procedure>\
		<Method Name=\"Init\" ID=\"2\">\
			<Bin Data=\"00\" />\
		</Method>\
	</Class>\
	<Class Name=\"String\" ID=\"7\" ParID=\"0\" Parent=\"Object\">\
		<Var Name=\"unicodeString\" Class=\"Wrapper\" />\
		<Procedure Name=\"New\" ID=\"3\">\
			<Var Name=\"Dest\" Class=\"String\" />\
			<Bin Data=\"14fdffffff070000000e10fdffffff0a040000000f00\" />\
		</Procedure>\
		<Method Name=\"Init\" ID=\"4\">\
			<Var Name=\"This\" Class=\"String\" />\
			<Bin Data=\"1116000000001800000000fdffffff00000000111701000000fdffffff000000000e10010000000c010000000f00\" />\
		</Method>\
	</Class>\
	<Class Name=\"Popups\" ID=\"8\" ParID=\"0\" Parent=\"Object\">\
		<Procedure Name=\"MessageBox\" ID=\"5\">\
			<Var Name=\"Title\" Class=\"String\" />\
			<Var Name=\"Message\" Class=\"String\" />\
			<Bin Data=\"0e10fcffffff10fdffffff0c020000000f00\" />\
		</Procedure>\
	</Class>\
	<CallBacks>\
		<CB Name=\"WString.LoadStringFromTable\" ID=\"0\" />\
		<CB Name=\"WString.Constructor\" ID=\"1\" />\
		<CB Name=\"WPopups.MessageBox\" ID=\"2\" />\
	</CallBacks>\
	<StringTable>\
		<String Val=\"Show String\" ID=\"0\" />\
	</StringTable>\
</Library>\
";