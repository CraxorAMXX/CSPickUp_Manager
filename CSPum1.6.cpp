#include <amxmodx> 
#include <amxmisc> 
#include <fakemeta> 
#include <nvault> 
#include <engine> 
#include <cstrike> 
#include <hamsandwich>
 

new const PLUGIN[] = "CS Pick up manager", 
	 VERSION[] = "1.6",  
	  AUTHOR[] = "Craxor";


#pragma semicolon 1;
      
new const gKeyList[][] = 
{ 
	"" , 
	"p228" , 
	"" , 
	"scout" , 
	"hegrenade" , 
	"xm1014" , 
	"c4" , 
	"mac10" , 
	"aug" , 
	"smokegrenade" , 
	"elite" , 
	"fiveseven" , 
	"ump45" , 
	"sg550" , 
	"galil" , 
	"famas" , 
	"usp" , 
	"glock18" , 
	"" , 
	"mp5navy" , 
	"m249" , 
	"m3" , 
	"m4a1" , 
	"tmp" , 
	"g3sg1" , 
	"flashbang" , 
	"deagle" , 
	"sg552" , 
	"ak47" , 
	"" , 
	"p90" , 
	"vest",
	"vesthelm",
	"shield"
}; 


new const ShowMessages[][] = 
{
	" ~ Conter-Strike Pick Up Manager Commands to use ~ ",
	"amx_cspum_addkey < Key to Block >",
	"amx_cspum_remkey < Key to Remov >",
	"amx_cspum_keylist",
	"amx_cspum_blocklist",
	"amx_cspum_blockedlist",
	"amx_cspum_resetlist" 
};
 

enum WeaponTypes
{
	WT_Weaponbox,
	WT_Armoury,
	WT_Shield
}


enum BlockType
{
	BT_Remove = 1,
	BT_BlockPickup
}


enum FunctionType
{
	AddKey,
	RemKey,
	MenuKey
}


enum _:GetWeaponIndexValues
{
	GWI_NotFound = -1
}

new giTypeCvar, giTeamCvar, giAdminsCvar;
new gNewVault; 


new bool:gBlockWeapons[34];  

new const gModelFile[]  = "models/w_%s.mdl";  

const CSW_SHIELD	= 33; 
const XoCArmoury	= 4; 
const m_iCount		= 35;


#define IsArmoury(%1)		(%1[0]=='a'&&%1[1]=='r'&&%1[7]=='_'&&%1[8]=='e'&&%1[12]=='t'&&%1[13]=='y')  
#define IsWpBox(%1)		(%1[0]=='w'&&%1[1]=='e'&&%1[5]=='n'&&%1[7]=='o'&&%1[8]=='x')
#define IsAShield(%1)		(%1[0]=='w'&&%1[1]=='e'&&%1[7]=='s'&&%1[8]=='h'&&%1[9]=='i'&&%1[11]=='l'&&%1[12]=='d') 


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
	
	register_concmd( "amx_cspum_addkey" , "addkey" , ADMIN_BAN , " < Weapon key-name to block > " ); 
	register_concmd( "amx_cspum_remkey" , "remkey" , ADMIN_BAN , " < Weapon key-name to block > " );
 
	register_concmd( "amx_cspum_blocklist" , "blockall" , ADMIN_BAN  );
	register_concmd( "amx_cspum_resetlist" , "resetall" , ADMIN_BAN  );

	register_concmd( "amx_cspum_keylist" , "showkeylist" , ADMIN_BAN ); 
	register_concmd( "amx_cspum_blockedlist", "blockedlist" , ADMIN_BAN );

	register_concmd( "amx_cspum", "cspumcmd" , ADMIN_BAN );

	register_concmd( "say .pickupmenu", "createmenu", ADMIN_BAN );
	register_concmd( "say_team .pickupmenu", "createmenu", ADMIN_BAN );

	new EntityPlayerClass [] = "player";

	register_dictionary( "cspum.txt" );
	
	register_touch( "armoury_entity" , EntityPlayerClass , "player_touch" );  
	register_touch( "weaponbox" , EntityPlayerClass , "player_touch" );  
	register_touch( "weapon_shield" , EntityPlayerClass , "player_touch" );  
	
	/*
		cspum_type: What happened with the entity when a player touch it.
			"1" - Remove the weapon when you touch it.
			"2" - Just blocking picking up the weapon.
	*/

	giTypeCvar = register_cvar( "cspum_type" , "2" ); 


	/*
		cspum_team: Block pick-up only for specified team.
			"1" - Terrorists.
			"2" - Counter-Terrorists.
			"3" - Spectators.
			"4" - For all teams.
	*/
	giTeamCvar = register_cvar( "cspum_team" , "3" );


	/*
		cspum_admins: Allow admins to pickup even if the weapons is blocked ?
			"1" - Yes.
			"0" - No.
	*/
	giAdminsCvar = register_cvar( "cspum_admins", "1" );
} 


public plugin_cfg() 
{ 
	new szModel[ 12 + 14 ] , szVal[ 2 ] , iTS; 
	
	for( new i = 1 ; i < sizeof gKeyList ; i++ ) 
	{ 
		if ( gKeyList[ i ][ 0 ] != EOS ) 
		{ 
			formatex( szModel , charsmax( szModel ) , gModelFile , gKeyList[ i ] ); 
			
			if ( nvault_lookup( gNewVault , szModel , szVal , charsmax( szVal ) , iTS ) ) 
			{ 
				gBlockWeapons[ i ] = true; 
			} 
		} 
	} 
} 


public plugin_end()  
{ 
	nvault_close( gNewVault );  
} 

public createmenu( id , level , cid ) 
{ 
	if( !cmd_access( id , level , cid , 1 ) ) 
		return PLUGIN_HANDLED; 

	new menu = menu_create( "\r[ \yCS Pick Up Manager Menu \r]" , "menu_handle" );
	new szBuffer[64];
	
	for( new i = 1; i < sizeof gKeyList ; i++ )
	{
		if ( gKeyList[ i ][ 0 ] != EOS ) 
		{ 
			formatex( szBuffer, charsmax( szBuffer ) , gBlockWeapons[ i ] ? "[ BLOCKED ] %s" : "[ FREE ] %s ", gKeyList[ i ] );
			menu_additem( menu , szBuffer , gKeyList[ i ] , ADMIN_BAN );
		}
	}

	menu_setprop( menu, MPROP_EXIT, MEXIT_ALL );

	menu_display( id , menu , 0 );
	return PLUGIN_HANDLED;
}

public menu_handle( id , menu , item )
{
	if( item == MENU_EXIT )
	{
		menu_destroy( menu );
		return PLUGIN_HANDLED;
	}

	new szInfo[20], _acc, _callback, ItemName[20], szModelFile[ 12 + sizeof gModelFile ], NewItemName[30];

	menu_item_getinfo( menu , item , _acc , szInfo , charsmax( szInfo ), ItemName, charsmax( ItemName ) , _callback );

	new iFoundIndex = GetWeaponIndex( id , szInfo, MenuKey );

	formatex( szModelFile , charsmax( szModelFile ) , gModelFile , gKeyList[ iFoundIndex ] ); 
	formatex( NewItemName , charsmax( NewItemName ) , !gBlockWeapons [ iFoundIndex ] ? "[ BLOCKED ] %s" : "[ FREE ] %s", szInfo );

	menu_item_setname( menu , item , NewItemName );
	

	if( gBlockWeapons[ iFoundIndex ] )
	{
		gBlockWeapons[ iFoundIndex ] = false;
		nvault_remove( gNewVault, szModelFile ); 

		new iEntity;


		if ( BlockType:get_pcvar_num( giTypeCvar ) == BT_Remove )
		{
			while ( ( iEntity = find_ent_by_class( iEntity , "armoury_entity" ) ) )
			{
				if ( pev_valid( iEntity ) && cs_get_armoury_type( iEntity ) == iFoundIndex )
					ExecuteHam( Ham_CS_Restart , iEntity );
			}
		}
	}

	else
	{
		gBlockWeapons[ iFoundIndex ] = true;
		nvault_set( gNewVault , szModelFile , "1" ); 
	}
	
	menu_display( id , menu , 0 );
	return PLUGIN_HANDLED;
}	


public addkey( id , level , cid ) 
{ 
	if( !cmd_access( id , level , cid , 2 ) ) 
		return PLUGIN_HANDLED; 
	
	new szWeaponArg[ 13 ] , szModelFile[ 12 + sizeof gModelFile  ] , iFoundIndex; 
	
	read_argv( 1 , szWeaponArg, charsmax( szWeaponArg ) );
	
	iFoundIndex = GetWeaponIndex( id , szWeaponArg , AddKey );

	if( iFoundIndex == GWI_NotFound )
		return PLUGIN_HANDLED;
	

	gBlockWeapons [ iFoundIndex ] = true;

	formatex( szModelFile , charsmax( szModelFile ) , gModelFile , gKeyList[ iFoundIndex ] ); 
	nvault_set( gNewVault , szModelFile , "1" ); 
	
	client_print( id , print_console , "%L" , id , "KEY_ADDED" , gKeyList[ iFoundIndex ] ); 

	return PLUGIN_HANDLED; 
}



public remkey( id , level , cid ) 
{ 
	if( !cmd_access( id , level , cid , 2 ) ) 
		return PLUGIN_HANDLED; 
	
	new szWeaponArg[ 13 ] , szModelFile[ 12 + sizeof( gModelFile ) ] , iFoundIndex , iEntity; 

	read_argv( 1 , szWeaponArg , charsmax( szWeaponArg ) );

	iFoundIndex = GetWeaponIndex( id , szWeaponArg , RemKey );
	

	if( iFoundIndex == GWI_NotFound )
		return PLUGIN_HANDLED;

	if ( BlockType:get_pcvar_num( giTypeCvar ) == BT_Remove )
	{
		while ( ( iEntity = find_ent_by_class( iEntity , "armoury_entity" ) ) )
		{
			if ( pev_valid( iEntity ) && cs_get_armoury_type( iEntity ) == iFoundIndex )
				ExecuteHam( Ham_CS_Restart , iEntity );
		}
	}
	
	
	formatex( szModelFile , charsmax( szModelFile ) , gModelFile , gKeyList[ iFoundIndex ] ); 
	nvault_remove( gNewVault , szModelFile );

	gBlockWeapons [ iFoundIndex ] = false;

	client_print( id , print_console , "%L" , id , "KEY_REMOVED" , gKeyList[ iFoundIndex ] ); 

	return PLUGIN_HANDLED; 
} 


public blockall( id , level , cid )
{ 
	if( !cmd_access( id , level , cid , 1 ) ) 
		return PLUGIN_HANDLED; 
	

	new szModelFile[ 12 + 14 ]; 

	for( new i = 1 ; i < sizeof gKeyList ; i++ ) 
	{
		if ( gKeyList[ i ][ 0 ] != EOS && !gBlockWeapons [ i ] )
		{
			formatex( szModelFile , charsmax( szModelFile ), gModelFile , gKeyList[ i ] );
			gBlockWeapons [ i ] = true;

			nvault_set( gNewVault , szModelFile , "1" );  
		}
	}
 
	client_print( id , print_console , "%L" , id , "WEAPONS_BLOCKED" );  	 

	return PLUGIN_HANDLED; 
}


public resetall( id , level , cid )
{
	if( !cmd_access( id , level , cid , 1 ) ) 
		return PLUGIN_HANDLED;

	new iEntity;
	
	nvault_prune( gNewVault, 0, get_systime() );
	client_print( id , print_console, "%L" , id , "RESET_WEAPONS" );

	if ( BlockType:get_pcvar_num( giTypeCvar ) == BT_Remove )
	{
		while ( ( iEntity = find_ent_by_class( iEntity , "armoury_entity" ) ) )
		{
			if ( pev_valid( iEntity ) && cs_get_armoury_type( iEntity ) )
				ExecuteHam( Ham_CS_Restart , iEntity );
		}
	}
	
	for( new i = 1; i < sizeof gKeyList; i++ )
		gBlockWeapons [ i ] = false;
		
	return PLUGIN_HANDLED;
}


public showkeylist( id , level, cid ) 
{ 
	if( !cmd_access( id , level , cid , 1 ) ) 
		return PLUGIN_HANDLED;
	
	client_print( id , print_console , "========== %L ========== " , id , "AVAIBLE_WEAPONS" ); 
	
	for( new i = 1; i < sizeof gKeyList ; i++ ) 
	{ 
		if ( gKeyList[ i ][ 0 ] ) 
			client_print( id , print_console , " %s" , gKeyList[ i ]  ); 
	} 
	
	return PLUGIN_HANDLED; 
} 


public blockedlist( id , level , cid )
{
	if( !cmd_access( id , level, cid, 1 ) )
		return PLUGIN_HANDLED;

	client_print( id , print_console , " ~~ %L ~~~ " , id , "BLOCKED_WEAPONS" );

	for( new i = 1; i < sizeof gKeyList ; i++ )
	{
		if ( gKeyList[ i ][ 0 ] != EOS && gBlockWeapons [ i ]  )
				client_print( id , print_console , "%s", gKeyList[ i ] );
	}
	return PLUGIN_HANDLED;
}

public cspumcmd( id , level , cid )
{
	if( !cmd_access( id , level, cid, 1 ) )
		return PLUGIN_HANDLED;

	for( new i = 0; i < sizeof ShowMessages ; i++ )
		client_print( id , print_console , ShowMessages[ i ] );

	return PLUGIN_HANDLED;
}


public player_touch( ent , id )  
{  
	if( !pev_valid ( ent ) || !id || !( pev( ent , pev_flags ) & FL_ONGROUND ) )  
		return -1; 
	
	new iWeaponID , szClassName[ 15 ] , WeaponTypes:wtType;

	
	pev( ent , pev_classname , szClassName , charsmax( szClassName ) ); 
	
	if( IsArmoury( szClassName ) )
	{
		iWeaponID = cs_get_armoury_type( ent );
		wtType = WT_Armoury;
	}
	else if ( IsWpBox( szClassName ) )
	{
		iWeaponID = cs_get_weaponbox_type( ent );
		wtType = WT_Weaponbox;
	}
	else if ( IsAShield( szClassName ) )
	{
		iWeaponID = CSW_SHIELD;
		wtType = WT_Shield;
	}
	else
	{
		iWeaponID = 0;
	}

	if( is_user_admin(id) && get_pcvar_num( giAdminsCvar ) || get_pcvar_num(giTeamCvar) != get_user_team(id) ) 
	{ 	
		iWeaponID = 0;
	}

	if ( gBlockWeapons [ iWeaponID ] )  
	{
		switch( get_pcvar_num( giTypeCvar ) ) 
		{ 
			case BT_Remove:
			{
				switch ( wtType )
				{
					case WT_Armoury:
					{
						set_pdata_int( ent , m_iCount , 0 , XoCArmoury );
						set_pev( ent , pev_solid , SOLID_NOT );
					
					}

					case WT_Weaponbox: call_think( ent );
					
					case WT_Shield: engfunc( EngFunc_RemoveEntity, ent );
					
				}
			}
			case BT_BlockPickup: return PLUGIN_HANDLED;
			default: return PLUGIN_CONTINUE;
		} 
	} 
	
	return PLUGIN_CONTINUE; 
}  


cs_get_weaponbox_type( iWeaponBox ) 
{ 
	new iWeapon;
	new const m_rgpPlayerItems_CWeaponBox[ 6 ] = { 34 , 35 , ... };  

	for ( new i = 1 ; i < sizeof m_rgpPlayerItems_CWeaponBox ; i++ ) 
	{ 
		if( ( iWeapon = get_pdata_cbase( iWeaponBox , m_rgpPlayerItems_CWeaponBox[ i ] , XoCArmoury ) ) > 0 ) 
		{ 
			return cs_get_weapon_id( iWeapon ); 
		} 
	} 

	return 0;
}


GetWeaponIndex( id , const szKeyword[] , FunctionType:Type )
{
	new iFound = GWI_NotFound;
	
	for ( new iWeapon = 1 ; iWeapon < sizeof( gKeyList ) ; iWeapon++ )
	{
		if ( equali( gKeyList[ iWeapon ] , szKeyword ) )
		{
			iFound = iWeapon;
		}
	}
	
	new iReturn = iFound;

	switch ( iFound )
	{
		case GWI_NotFound: 
		{
			client_print( id , print_console, "%L" , id , "WEAPON_NOTFOUND" , szKeyword );
			iReturn = GWI_NotFound; 
		}
		default:
		{
			switch(Type)
			{
				case AddKey:
				{
					if ( gBlockWeapons [ iFound ] )
					{
						client_print( id , print_console , "%L" , id , "ALREADY_SAVED" , gKeyList[ iFound ] );
						iReturn = GWI_NotFound;	
					}
				}
				case RemKey:
				{
					if ( !gBlockWeapons [ iFound ] )
					{
						client_print( id , print_console , "%L" , id , "ALREADY_REMOVED" , gKeyList[ iFound ] ); 
						iReturn = GWI_NotFound;	
					}
				}

				case MenuKey:
				{
					client_print( id , print_center , "CSPick up manager list has been updated." );
				}
			}	
		}
	}
	
	return iReturn;
}
