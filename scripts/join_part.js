var func_exec = function( command, sender, dest, argu )
{
	if( this.irc.isAdmin( sender[1] ) )
	{
		if( command == "join" )
		{
			this.irc.sendRaw( "JOIN " + dest );
			for(i = 0;i < argu.length; ++i)
			{
				this.irc.sendRaw( "JOIN " + argu[i] );
			}
		}
		else if( command == "quit" )
		{
			var msg_quit = '';
			if( argu.length )
			{
				msg_quit = argu.join( " " );
			}
			else
			{
				msg_quit = this.irc.getValue( "bot/quit_msg" );
			}
			
			this.irc.sendRaw( "QUIT " + msg_quit );
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
				
				this.irc.sendRaw( "PART " + dest + " " + part_msg );
			}
		}
	}
}

var func_hook = function()
{
	return "join,part,quit";
}
