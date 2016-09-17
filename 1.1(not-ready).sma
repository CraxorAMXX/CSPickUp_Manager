#include <amxmodx> 
#include <amxmisc> 
#include <fakemeta> 
#include <nvault> 
#include <engine> 
#include <cstrike> 
#include <hamsandwich> 

new const PLUGIN[] = "CS Pick up manager", 
	 VERSION[] = "1.1", 
	  AUTHOR[] = "Craxor"; 


static const gKeyList[][] = 
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
 
	register_concmd( "amx_cspum_block_all" , "addallkey" , ADMIN_BAN , " You've blocked all keys from the list! " );
	register_concmd( "amx_cspum_reset" , "remallkey" , ADMIN_BAN , " The keylist has been reseted!" );

	/*   To do
	====
	register_clcmd( "amx_cspum_blocked_keylist" , "_ShowBlockedKeysList" );
	=====
		*/
	register_clcmd( "amx_cspum_keylist" , "_ShowKeyLists" ); 
	
	register_touch( "armoury_entity" , "player" , "OnPlayerTouchItem" );  
	register_touch( "weaponbox" , "player" , "OnPlayerTouchItem" );  
	register_touch( "weapon_shield" , "player" , "OnPlayerTouchItem" );  
	
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
	
	
	if ( !read_argv( 1 , szWeaponArg , charsmax( szWeaponArg ) ) ) 
	{ 
		client_print( id , print_console , "No weapon was specified, please try again. amx_cspum_addkey <weapon>" ); 
		return PLUGIN_HANDLED; 
	} 
	
	formatex( szModelFile , charsmax( szModelFile ), gModelFile , szWeaponArg ); 
	
	if( nvault_lookup( gNewVault , szModelFile , szVaultData, charsmax( szVaultData ) , iTS ) ) 
	{ 
		client_print( id , print_console , "This key is already saved in the pick up manager list." ); 
		return PLUGIN_HANDLED; 
	} 
	
	for( new i = 0 ; i < sizeof ( gKeyList ) ; i++ ) 
	{ 
		if ( containi( gKeyList[ i ] , szWeaponArg ) > -1 ) 
		{ 
			iFoundIndex = i; 
			break; 
		} 
	} 
	
	if ( iFoundIndex == -1 ) 
	{ 
		client_print( id , 2, "Unable to find the key '%s', type amx_cspum_keylist for all keys available to insert." , szWeaponArg ); 
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
	
	
	if ( !read_argv( 1 , szWeaponArg , charsmax( szWeaponArg ) ) ) 
	{ 
		client_print( id , print_console , "No weapon was specified, please try again. amx_cspum_remkey <weapon>" ); 
		return PLUGIN_HANDLED; 
	} 
	
	formatex( szModelFile , charsmax( szModelFile ) , gModelFile , szWeaponArg ); 
	
	if( !nvault_lookup( gNewVault , szModelFile , szVaultData , charsmax( szVaultData ) , iTS ) ) 
	{ 
		client_print( id , print_console , "Sorry, this key is not saved yet." ); 
		return PLUGIN_HANDLED; 
	} 
	
	for( new i = 0 ; i < sizeof ( gKeyList ) ; i++ ) 
	{ 
		if ( containi( gKeyList[ i ] , szWeaponArg ) > -1 ) 
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

public addallkey( id, level, cid )
{ 
	if( !cmd_access( id , level , cid , 1 ) ) 
		return PLUGIN_HANDLED; 
	
	new szModelFile[ 12 + 14 ]; 

	for( new i = 0 ; i < sizeof ( gKeyList ) ; i++ ) 
	{ 
		formatex( szModelFile , charsmax( szModelFile ), gModelFile , gKeyList[ i ] );
		gBlockWeapons |= ( 1 << i );

		nvault_set( gNewVault , szModelFile , "1" );  
	}  	 
	return PLUGIN_HANDLED; 
}

public remallkey( id, level, cid )
{
	if( !cmd_access( id , level , cid , 1 ) ) 
		return PLUGIN_HANDLED;


	nvault_prune( gNewVault, 0, get_systime( ) );
	client_print( id, print_console, "You've succesfully reseted all the list!" );

	// We update the bitfield.
	for( new i = 33; i >= sizeof ( gKeyList ) ; i-- ) 
		gBlockWeapons = ( 1 << 0 );
		

	return PLUGIN_HANDLED;
}

public _ShowKeyLists( id ) 
{ 
	if( !is_user_admin( id ) ) 
	{ 
		client_print( id, print_console, "You have no access to this command!" ); 
		return PLUGIN_HANDLED; 
	} 
	
	client_print( id , print_console , "========== All weapons available to add to the list ========== " ); 
	
	for( new i = 0; i < sizeof( gKeyList ); i++ ) 
	{ 
		if ( gKeyList[ i ][ 0 ] ) 
			client_print( id , print_console , " %s" , gKeyList[ i ]  ); 
	} 
	
	return PLUGIN_HANDLED; 
} 

public OnPlayerTouchItem( ent , id )  
{  
	if( !ent || !id || !( pev( ent , pev_flags) & FL_ONGROUND ) )  
		return -1; 
	
	static iWeaponID , iCvarValue , szClassName[ 8 ]; 
	
	pev( ent , pev_classname , szClassName , charsmax( szClassName ) ); 
	
	switch ( szClassName[ 6 ] ) 
	{ 
		case 'b': iWeaponID = cs_get_weaponbox_type( ent ); 
		case 'y': iWeaponID = cs_get_armoury_type( ent ); 
		case '_': iWeaponID = CSW_SHIELD; 
	} 
	
	if ( iWeaponID && ( gBlockWeapons & ( 1 << iWeaponID ) ) )  
	{ 
		iCvarValue = get_pcvar_num( giTypeCvar ); 
		
		switch( iCvarValue ) 
		{ 
			case 1: engfunc( EngFunc_RemoveEntity, ent ); 
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
	new const XO_CWEAPONBOX = 4; 

	for ( new i = 1 ; i <= 5 ; i++ ) 
	{ 
		if( ( iWeapon = get_pdata_cbase( iWeaponBox , m_rgpPlayerItems_CWeaponBox[ i ] , XO_CWEAPONBOX ) ) > 0 ) 
		{ 
			return cs_get_weapon_id( iWeapon ) 
		} 
	} 

	return 0 
}  
