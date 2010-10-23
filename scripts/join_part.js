var func_exec = function( command, sender, dest, argu )
{
	if( this.irc.isAdmin( sender[1] ) )
	{
		if( command == "join" || command == "!goto" )
		{
			this.irc.join( "JOIN " + dest );
			for(i = 0;i < argu.length; ++i)
			{
				this.irc.join( "JOIN " + argu[i] );
			}
		}
		else if( command == "!degage" )
		{
			this.irc.deconnection();
		}
		else // command = part
		{
			if( dest != "" )
			{
				var part_msg = argu.join(" ");
				if( part_msg == "" )
				{
					part_msg = this.irc.getValue( "bot/quit_msg" );
				}
				
				this.irc.leave( "PART " + dest + " " + part_msg );
			}
		}
	}
}

var func_hook = function()
{
	return "!goto,join,part,!degage";
}
