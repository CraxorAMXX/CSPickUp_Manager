#include <amxmodx> 
#include <amxmisc> 
#include <fakemeta> 
#include <nvault> 
#include <engine> 
#include <cstrike> 
#include <hamsandwich> 

new const PLUGIN[] = "CS Pick up manager", 
	 VERSION[] = "1.2",  
	  AUTHOR[] = "Craxor"; 


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
	"shield"

}; 


new giTypeCvar; 
new gNewVault; 


new gBlockWeapons;  
new gIgnoreWeapons = ( ( 1 << 0 ) | ( 1 << 2 ) | ( 1 << 18 ) | ( 1 << 29 ) ); 


new const gModelFile[] = "models/w_%s.mdl";  

const CSW_SHIELD = 31; 
const XoCArmoury = 4 
const m_iCount = 35
const m_iType = 34;

#define IsArmoury(%1)        (%1[0]=='a'&&%1[1]=='r'&&%1[7]=='_'&&%1[8]=='e'&&%1[12]=='t'&&%1[13]=='y')  
#define IsWpBox(%1)        (%1[0]=='w'&&%1[1]=='e'&&%1[5]=='n'&&%1[7]=='o'&&%1[8]=='x')
#define IsAShield(%1)        (%1[0]=='w'&&%1[1]=='e'&&%1[7]=='s'&&%1[8]=='h'&&%1[9]=='i'&&%1[11]=='l'&&%1[12]=='d') 

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
 
	register_concmd( "amx_cspum_blocklist" , "block_all" , ADMIN_BAN  );
	register_concmd( "amx_cspum_resetlist" , "reset" , ADMIN_BAN  );

	register_concmd( "amx_cspum_keylist" , "showkeylist" , ADMIN_BAN ); 

	new EntityPlayerClass [] = "player";
	
	register_touch( "armoury_entity" , EntityPlayerClass , "player_touch" );  
	register_touch( "weaponbox" , EntityPlayerClass , "player_touch" );  
	register_touch( "weapon_shield" , EntityPlayerClass , "player_touch" );  
	
	/*
		cspum_type 
			"1"
	*/
	giTypeCvar = register_cvar( "cspum_type" , "2" ); 
} 

public plugin_cfg() 
{ 
	new szModel[ 12 + 14 ] , szVal[ 2 ] , iTS; 
	
	for( new i = 0 ; i < sizeof ( gKeyList ) ; i++ ) 
	{ 
		if ( !( gIgnoreWeapons & ( 1 << i ) ) ) 
		{ 
			formatex( szModel , charsmax( szModel ) , gModelFile , gKeyList[ i ] ); 
			
			if ( nvault_lookup( gNewVault , szModel , szVal , charsmax( szVal ) , iTS ) ) 
			{ 
				gBlockWeapons |= ( 1 << i ); 
			} 
		} 
	} 
} 

public plugin_end()  
{ 
	nvault_close( gNewVault );  
} 

public addkey( id , level , cid ) 
{ 
	if( !cmd_access( id , level , cid , 2 ) ) 
		return PLUGIN_HANDLED; 
	
	new szWeaponArg[ 13 ] , szModelFile[ 12 + 14 ] , szVaultData[ 2 ] , iTS , iFoundIndex = -1; 

	read_argv( 1, szWeaponArg, charsmax( szWeaponArg ) );

	
	formatex( szModelFile , charsmax( szModelFile ), gModelFile , szWeaponArg ); 
	
	if( nvault_lookup( gNewVault , szModelFile , szVaultData, charsmax( szVaultData ) , iTS ) ) 
	{ 
		client_print( id , print_console , "This key is already saved in the pick up manager list." ); 
		return PLUGIN_HANDLED; 
	} 
	
	for( new i = 0 ; i < sizeof ( gKeyList ) ; i++ ) 
	{ 
		if ( containi( gKeyList[ i ] , szWeaponArg ) > -1 && !( gIgnoreWeapons & ( 1 << i ) ) ) 
		{ 
			iFoundIndex = i; 
			break; 
		} 
	} 
	
	if ( iFoundIndex == -1 ) 
	{ 
		client_print( id , print_console, "Unable to find the key '%s', type amx_cspum_keylist for all keys available to insert." , szWeaponArg ); 
	} 

	else 
	{ 
		client_print( id , print_console , "You successfully added '%s' to the list!" , szWeaponArg ); 
		
		gBlockWeapons |= ( 1 << iFoundIndex ); 
		
		nvault_set( gNewVault , szModelFile , "1" ); 
	} 
	
	return PLUGIN_HANDLED; 
}


public remkey( id, level, cid ) 
{ 
	if( !cmd_access( id, level, cid, 2 ) ) 
		return PLUGIN_HANDLED; 
	
	new szWeaponArg[ 13 ] , szModelFile[ 12 + 14 ] , szVaultData[ 2 ] , iTS , iFoundIndex; 

	read_argv( 1, szWeaponArg, charsmax( szWeaponArg ) );
	
	
	formatex( szModelFile , charsmax( szModelFile ) , gModelFile , szWeaponArg ); 
	
	if( !nvault_lookup( gNewVault , szModelFile , szVaultData , charsmax( szVaultData ) , iTS ) ) 
	{ 
		client_print( id , print_console , "Sorry, this key is not saved yet." ); 
		return PLUGIN_HANDLED; 
	} 
	
	for( new i = 0 ; i < sizeof ( gKeyList ) ; i++ ) 
	{ 
		if ( containi( gKeyList[ i ] , szWeaponArg ) > -1 && !( gIgnoreWeapons & ( 1 << i ) ) ) 
		{ 
			iFoundIndex = i; 
			break; 
		} 
	} 
	
	client_print( id , print_console , "You succesfully removed '%s' from the list!" , szWeaponArg ); 
	
	gBlockWeapons &= ~( 1 << iFoundIndex ); 
	
	nvault_remove( gNewVault , szModelFile ); 
	
	return PLUGIN_HANDLED; 
} 

public block_all( id, level, cid )
{ 
	if( !cmd_access( id , level , cid , 1 ) ) 
		return PLUGIN_HANDLED; 
	

	new szModelFile[ 12 + 14 ]; 

	for( new i = 0 ; i < sizeof ( gKeyList ) ; i++ ) 
	{
		if ( !( gIgnoreWeapons & ( 1 << i ) ) && !( gBlockWeapons & ( 1 << i ) ) )
		{
			formatex( szModelFile , charsmax( szModelFile ), gModelFile , gKeyList[ i ] );
			gBlockWeapons |= ( 1 << i );

			nvault_set( gNewVault , szModelFile , "1" );  
		}
	}
 
	client_print( id, print_console, "All the weapons have been added inside of the list!" ); 	 

	return PLUGIN_HANDLED; 
}

public reset( id, level, cid )
{
	if( !cmd_access( id , level , cid , 1 ) ) 
		return PLUGIN_HANDLED;


	nvault_prune( gNewVault, 0, get_systime( ) );
	client_print( id, print_console, "You've succesfully reseted all the list!" );

	gBlockWeapons = 0;
		

	return PLUGIN_HANDLED;
}

public showkeylist( id , level, cid ) 
{ 
	if( !cmd_access( id, level, cid, 1 ) ) 
		return PLUGIN_HANDLED;
	
	client_print( id , print_console , "========== All weapons available to add to the list ========== " ); 
	
	for( new i = 0; i < sizeof( gKeyList ); i++ ) 
	{ 
		if ( gKeyList[ i ][ 0 ] && !( gIgnoreWeapons & ( 1 << i ) ) ) 
			client_print( id , print_console , " %s" , gKeyList[ i ]  ); 
	} 
	
	return PLUGIN_HANDLED; 
} 

public player_touch( ent , id )  
{  
	if( !pev_valid( ent ) || !id || !( pev( ent , pev_flags) & FL_ONGROUND ) )  
		return -1; 
	
	new iWeaponID , iCvarValue , szClassName[ 15 ], iWeaponType; 
	
	pev( ent , pev_classname , szClassName , charsmax( szClassName ) ); 
	
	if( IsArmoury( szClassName ) )
	{
		iWeaponID = get_pdata_int( ent , m_iType , XoCArmoury )  
		iWeaponType = 1;
		client_print( id, print_chat, "%s ( Armoury )", szClassName );
	}

	else if ( IsWpBox( szClassName ) )
	{
		iWeaponID = cs_get_weaponbox_type( ent );
		iWeaponType = 1;
		client_print( id, print_chat, "%s ( box )", szClassName );
	}

	else if ( IsAShield( szClassName ) )
	{
		client_print( id, print_chat, "%s ( shield)", szClassName );
		iWeaponID = CSW_SHIELD;
		iWeaponType = 0;
	}

	else
	{
		client_print( id, print_chat, "Nothing? %s", szClassName );
		iWeaponID = 0;
		iWeaponType = 0;
	
	}

	if ( iWeaponID && ( gBlockWeapons & ( 1 << iWeaponID ) ) )  
	{ 
		iCvarValue = get_pcvar_num( giTypeCvar ); 
		
		switch( iCvarValue ) 
		{ 
			case 1:
			{

				if( iWeaponType )
					call_think( ent );
				else
					engfunc( EngFunc_RemoveEntity, ent );

			}
			case 2: return PLUGIN_HANDLED; 
			default: return PLUGIN_CONTINUE; 
		} 
	} 
	
	return PLUGIN_CONTINUE; 
}  

cs_get_weaponbox_type( iWeaponBox ) 
{ 
	new iWeapon 
	new const m_rgpPlayerItems_CWeaponBox[ 6 ] = { 34 , 35 , ... };  

	for ( new i = 1 ; i <= 5 ; i++ ) 
	{ 
		if( ( iWeapon = get_pdata_cbase( iWeaponBox , m_rgpPlayerItems_CWeaponBox[ i ] , XoCArmoury ) ) > 0 ) 
		{ 
			return cs_get_weapon_id( iWeapon ) 
		} 
	} 

	return 0 
}  
