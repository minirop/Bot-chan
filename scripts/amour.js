var func_exec = function( command, sender, dest, argu )
{
	if( argu.length == 2 )
	{
		var lettres = new Array();
		lettres['a'] = 0;
		lettres['m'] = 0;
		lettres['o'] = 0;
		lettres['u'] = 0;
		lettres['r'] = 0;
		
		var argu0 = argu[0].toLowerCase();
		var argu1 = argu[1].toLowerCase();
		
		for(i = 0;i < argu0.length; ++i)
		{
			lettres[argu0[i]] = lettres[argu0[i]]+1;
		}
		for(i = 0;i < argu1.length; ++i)
		{
			lettres[argu1[i]] = lettres[argu1[i]]+1;
		}
		
		var tab4 = new Array();
		tab4[0] = ((lettres["a"] + lettres["m"]) % 10) + parseInt((lettres["a"] + lettres["m"]) / 10,10);
		tab4[1] = ((lettres["m"] + lettres["o"]) % 10) + parseInt((lettres["m"] + lettres["o"]) / 10,10);
		tab4[2] = ((lettres["o"] + lettres["u"]) % 10) + parseInt((lettres["o"] + lettres["u"]) / 10,10);
		tab4[3] = ((lettres["u"] + lettres["r"]) % 10) + parseInt((lettres["u"] + lettres["r"]) / 10,10);
		
		var tab3 = new Array();
		tab3[0] = ((tab4[0] + tab4[1]) % 10) + parseInt((tab4[0] + tab4[1]) / 10,10);
		tab3[1] = ((tab4[1] + tab4[2]) % 10) + parseInt((tab4[1] + tab4[2]) / 10,10);
		tab3[2] = ((tab4[2] + tab4[3]) % 10) + parseInt((tab4[2] + tab4[3]) / 10,10);
		
		var tab = new Array();
		tab[0] = ((tab3[0] + tab3[1]) % 10) + parseInt((tab3[0] + tab3[1]) / 10,10);
		tab[1] = ((tab3[1] + tab3[2]) % 10) + parseInt((tab3[1] + tab3[2]) / 10,10);
		
		this.irc.send( dest, "compatibilité entre " + argu[0] + " et " + argu[1] + " : " + tab[0] + tab[1] + "%" );
	}
	else
	{
		this.irc.notice( sender[0], "trop ou pas assez d'arguments" );
	}
}

var func_hook = function()
{
	return "!amour";
}
