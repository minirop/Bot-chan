var func_exec = function( command, sender, dest, argu )
{
	if( this.irc.isAdmin( sender[1] ) )
	{
		if( command == "!nick" )
		{
			this.irc.sendRaw( "nick " + argu.join( " " ) );
		}
		else // command = nick
		{
			this.irc.sendRaw( "nick " + dest + " " + argu.join( " " ) );
		}
	}
}

var func_hook = function()
{
	return "!nick,nick";
}
