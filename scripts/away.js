var func_exec = function( command, sender, dest, argu )
{
	if( this.irc.isAdmin( sender[2] ) )
	{
		if( command == "away" )
		{
			var reason = argu.join(" ");
			if( reason == "" )
			{
				reason = this.irc.getValue( "bot/away_msg" )
			}
			this.irc.sendRaw( "away " + reason );
		}
		else // command = back
		{
			this.irc.sendRaw( "away" );
		}
	}
}

var func_hook = function()
{
	return "away,back";
}
