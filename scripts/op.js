var func_exec = function( command, sender, dest, argu )
{
	if( this.irc.isAdmin( sender[1] ) )
	{
		var toUp = argu.join( " " );
		if( toUp == "" )
		{
			toUp = sender[0];
		}
		
		if( command == "!op" )
		{
			this.irc.sendRaw( "MODE " + dest + " +o " + toUp );
		}
		else if( command == "!hop" )
		{
			this.irc.sendRaw( "MODE " + dest + " +h " + toUp );
		}
		else if( command == "!deop" )
		{
			this.irc.sendRaw( "MODE " + dest + " -o " + toUp );
		}
		else if( command == "!dehop" )
		{
			this.irc.sendRaw( "MODE " + dest + " -h " + toUp );
		}
		else if( command == "!devoice" )
		{
			this.irc.sendRaw( "MODE " + dest + " -v " + toUp );
		}
		else // command = !voice
		{
			this.irc.sendRaw( "MODE " + dest + " +v " + toUp );
		}
	}
}

var func_hook = function()
{
	return "!op,!hop,!voice,!deop,!dehop,!devoice";
}
