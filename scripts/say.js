var func_exec = function( command, sender, dest, argu )
{
	if( this.irc.isAdmin( sender[1] ) )
	{
		if( command == "say" )
		{
			this.irc.send( dest, argu.join(" ") );
		}
		else if( command == "action" )
		{
			this.irc.action( dest, argu.join(" ") );
		}
		else if( command == "notice" )
		{
			this.irc.notice( dest, argu.join(" ") );
		}
	}
}

var func_hook = function()
{
	return "say,notice,action";
}
