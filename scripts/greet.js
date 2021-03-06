var greetings = new Array();

var func_exec = function( command, sender, dest, argu )
{
	if( this.irc.isAdmin( sender[2] ) )
	{
		if( dest == "add" )
		{
			var nick = argu.shift();
			this.irc.addValue( "greet/" + nick, argu.join( " " ) );
			this.irc.send( sender[0], "greet ajout�, merci" );
			greetings[ nick ] = argu.join( " " );
		}
		else if( dest == "nb" )
		{
			for (var cle in greetings)
			{
				this.irc.send( sender[0], cle + " : " + greetings[ cle ] );
			}
		}
	}
}

var func_hook = function()
{
	var keys = this.irc.getValues( "greet" );
	for(i = 0;i < keys.length;i++)
	{
		greetings[ keys[i] ] = this.irc.getValue( "greet/" + keys[i] );
	}
	
	return "greet";
}

var func_hook_event = function()
{
	return "JOIN,YOU_JOIN";
}

var func_event = function( command, sender, dest, argu )
{
	if( command == "JOIN" )
	{
		if( greetings[ sender[1] ] )
		{
			this.irc.send( dest, "[" + sender[0] + "] " + greetings[ sender[1] ] );
		}
	}
	else
	{
		this.irc.send( dest, "Ohayo minna" );
	}
}
