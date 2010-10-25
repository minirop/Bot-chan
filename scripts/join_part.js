var func_exec = function( command, sender, dest, argu )
{
	if( this.irc.isAdmin( sender[2] ) )
	{
		if( command == "join" || command == "!goto" )
		{
			this.irc.join( dest );
			for(i = 0;i < argu.length; ++i)
			{
				this.irc.join( argu[i] );
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
				
				this.irc.leave( dest, part_msg );
			}
		}
	}
}

var func_hook = function()
{
	return "!goto,join,part,!degage";
}
