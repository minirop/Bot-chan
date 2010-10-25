var func_exec = function( command, sender, dest, argu )
{
	if( this.irc.isAdmin( sender[2] ) )
	{
		if( command == "!bye" )
		{
			this.irc.sendRaw( "kick " + dest + " " + argu.join( " " ) );
		}
		else if( command == "!adios" )
		{
			this.irc.sendRaw( "ban " + dest + " " + argu.join( " " ) );
		}
		else if( command == "kick" )
		{
			this.irc.sendRaw( "kick " + dest + " " + argu.join( " " ) );
		}
		else // command = ban
		{
			this.irc.sendRaw( "ban " + dest + " " + argu.join( " " ) );
		}
	}
}

var func_hook = function()
{
	return "!bye,!adios,kick,ban";
}
