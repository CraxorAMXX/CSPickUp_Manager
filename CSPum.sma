/*
	Plugin in-work(not ready), i wanne thanks to Bugsy, ConnorMcLeod, HamletEagle, Arkshine, addonszz, and everybody who will use this plugin.
*/

#include <amxmodx>
#include <amxmisc>
#include <fakemeta>
#include <nvault>
#include <engine>

new const PLUGIN[]    = "CS Pick up manager",
	 VERSION[]    = "0.1",
	  AUTHOR[]    = "Craxor";

new const gKeyList[ ] [ ] =
{
	"========== All weapons-key avaible to add to add in list ========== ",
	"p228",
	"shield",
	"scout",
	"hegrenade",
	"xm1014",
	"c4",
	"mac10",
	"aug",
	"smokegrenade",
	"elite",
	"fiveseven",
	"ump45",
	"sg550",
	"galil",
	"famas",
	"usp",
	"glock18",
	"mp5navy",
	"m249",
	"m3",
	"m4a1",
	"tmp",
	"g3sg1",
	"flashbang",
	"deagle",
	"sg552",
	"ak47",
	"p90"
};

new giTypeCvar;
new gNewVault;

public plugin_init( )
{
	register_plugin
	(
		.plugin_name = PLUGIN, 
		.version     = VERSION, 
 		.author      = AUTHOR
	);

	gNewVault = nvault_open( "cspick_up_manager_vault" );

	if( gNewVault == INVALID_HANDLE )
		set_fail_state( "Problems openning cspick up manager vault." );

 	register_concmd( "amx_cspum_addkey" ,"addkey", ADMIN_BAN, " < Weapon key-name to block > ");
	register_concmd( "amx_cspum_remkey" ,"remkey", ADMIN_BAN, " < Weapon key-name to block > ");

	register_clcmd( "amx_cspum_keylist", "_ShowKeyLists" );

	giTypeCvar = register_cvar("cspum_type", "2" );

	register_touch("armoury_entity", "player", "OnPlayerTouchArmoury"); 
	register_touch("weaponbox", "player", "OnPlayerTouchWeaponBox"); 
	register_touch("weapon_shield", "player", "OnPlayerTouchShield"); 
}


public addkey( id, level, cid )
{
	if( !cmd_access( id, level, cid, 2 ) )
		return PLUGIN_HANDLED;

	new Arg1[16];
	read_argv( 1 , Arg1, charsmax( Arg1 ) );

	new adder[64];
	formatex( adder, charsmax( adder ), "models/w_%s.mdl", Arg1 );

	new szVaultData[ 60 ] , iTS;
    
	if( nvault_lookup( gNewVault, adder , szVaultData, charsmax( szVaultData ) , iTS ) )
	{
		client_print( id, 2, "This key is already saved in the pick up manager list." );
		return PLUGIN_HANDLED;
	}

	for( new i=0; i < sizeof ( gKeyList ); i++ )
	{
		if ( !containi( gKeyList[i], Arg1 ) )
		{
			client_print( id , 2, "Unnable to found this key, type amx_cspum_keylist for all keys avaible to insert." );
			return PLUGIN_HANDLED;
		}
		else
		{	
			client_print( id , 2, "You succesfully added %s in list!", Arg1 );
			nvault_set( gNewVault, adder , "1" );
			return PLUGIN_HANDLED;
		}
	}
	
	return PLUGIN_HANDLED;
}

public remkey( id, level, cid )
{
	if( !cmd_access( id, level, cid, 2 ) )
 		return PLUGIN_HANDLED;

	new Arg1[16];
	read_argv( 1, Arg1, charsmax( Arg1 ) );

 	new adder[64];
	formatex( adder, charsmax( adder ), "models/w_%s.mdl", Arg1 );

	new szVaultData[ 60 ] , iTS;
    
	if( !nvault_lookup( gNewVault, adder , szVaultData, charsmax( szVaultData ) , iTS ) )
	{
		client_print( id, 2, "Sorry, this key is not saved yet." );
		return PLUGIN_HANDLED;
	}

	nvault_remove( gNewVault, adder );

	return PLUGIN_HANDLED;
}

public _ShowKeyLists( id )
{
	if( !is_user_admin( id ) )
	{
		client_print( id, print_console, "You've no acccess to this command!" );
		return PLUGIN_HANDLED;
	}

	for( new i = 0; i < sizeof( gKeyList ); i++ )
		client_print( id, 2, " %s", gKeyList[i] );

	return PLUGIN_HANDLED;
}
public OnPlayerTouchWeaponBox( ent , id ) 
{ 
	if( !ent || !id )
		return -1;

	new szWeaponModel[40];
	pev( ent, pev_model, szWeaponModel, charsmax( szWeaponModel ) )

	new iCvarValue = get_pcvar_num( giTypeCvar );

	new szVaultData[ 60 ] , iTS;
    
	if( nvault_lookup( gNewVault, szWeaponModel , szVaultData, charsmax( szVaultData ) , iTS ) )
	{
		switch( iCvarValue )
		{
			case 1: engfunc( EngFunc_RemoveEntity, ent );
			case 2: return PLUGIN_HANDLED;
			default: return PLUGIN_CONTINUE;
		}
	}

	return PLUGIN_CONTINUE;
} 

public OnPlayerTouchArmoury( ent , id ) 
	OnPlayerTouchWeaponBox( ent, id );

public OnPlayerTouchShield( ent, id )
	OnPlayerTouchWeaponBox( ent, id );

public plugin_end( ) nvault_close( gNewVault ); 
