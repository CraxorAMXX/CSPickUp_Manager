#include <amxmodx> 
#include <amxmisc> 
#include <fakemeta> 
#include <nvault> 
#include <engine> 
#include <cstrike> 
#include <hamsandwich> 

new const PLUGIN[] = "CS Pick up manager", 
	 VERSION[] = "1.3",  
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
	"shield",
	"kevlar"

}; 


new const g_ArmouryTypes[] = 
{
    CSW_MP5NAVY ,
    CSW_TMP ,
    CSW_P90 , 
    CSW_MAC10 ,
    CSW_AK47 , 
    CSW_SG552 , 
    CSW_M4A1 , 
    CSW_AUG , 
    CSW_SCOUT , 
    CSW_G3SG1 , 
    CSW_AWP , 
    CSW_M3 , 
    CSW_XM1014 , 
    CSW_M249 , 
    CSW_FLASHBANG , 
    CSW_HEGRENADE , 
    CSW_VEST , 
    CSW_VESTHELM , 
    CSW_SMOKEGRENADE
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
	RemKey
}


enum _:GetWeaponIndexValues
{
	GWI_NotFound = -1,
	GWI_Duplicate
}

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
	register_concmd( "amx_cspum_blockedlist", "blockedlist" , ADMIN_BAN );

	new EntityPlayerClass [] = "player";
	
	register_touch( "armoury_entity" , EntityPlayerClass , "player_touch" );  
	register_touch( "weaponbox" , EntityPlayerClass , "player_touch" );  
	register_touch( "weapon_shield" , EntityPlayerClass , "player_touch" );  
	
	/*
		cspum_type 
			"1" - Remove the weapon when you touch it.
			"2" - Just blocking picking up the weapon.
	*/

	giTypeCvar = register_cvar( "cspum_type" , "2" ); 
} 

public plugin_cfg() 
{ 
	new szModel[ 12 + 14 ] , szVal[ 2 ] , iTS; 
	
	for( new i = 1 ; i < sizeof gKeyList ; i++ ) 
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
	
	new szWeaponArg[ 13 ] , szModelFile[ 12 + sizeof gModelFile  ] , iFoundIndex; 
	
	read_argv( 1 , szWeaponArg, charsmax( szWeaponArg ) );
	
	iFoundIndex = GetWeaponIndex( id , szWeaponArg , AddKey );

	if( iFoundIndex == GWI_NotFound )
		return PLUGIN_HANDLED;
	

	gBlockWeapons |= ( 1 << iFoundIndex ); 

	formatex( szModelFile , charsmax( szModelFile ) , gModelFile , gKeyList[ iFoundIndex ] ); 
	nvault_set( gNewVault , szModelFile , "1" ); 
	
	client_print( id , print_console , "You successfully added '%s' to the list!" , gKeyList[ iFoundIndex ] ); 

	return PLUGIN_HANDLED; 
}


public remkey( id, level, cid ) 
{ 
	if( !cmd_access( id, level, cid, 2 ) ) 
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
			if ( ( g_ArmouryTypes[get_pdata_int( iEntity , m_iType , XoCArmoury ) ]  == iFoundIndex ) && pev_valid( iEntity ) )
				ExecuteHam( Ham_CS_Restart , iEntity );
		}
	}
	
	gBlockWeapons &= ~( 1 << iFoundIndex ); 
	
	formatex( szModelFile , charsmax( szModelFile ) , gModelFile , gKeyList[ iFoundIndex ] ); 
	nvault_remove( gNewVault , szModelFile ); 

	client_print( id , print_console, "You succesfully removed '%s' from the list!", gKeyList[ iFoundIndex ] );

	return PLUGIN_HANDLED; 
} 

public block_all( id, level, cid )
{ 
	if( !cmd_access( id , level , cid , 1 ) ) 
		return PLUGIN_HANDLED; 
	

	new szModelFile[ 12 + 14 ]; 

	for( new i = 1 ; i < sizeof gKeyList ; i++ ) 
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

	new iEntity;
	
	nvault_prune( gNewVault, 0, get_systime() );
	client_print( id, print_console, "You've succesfully reseted all the list!" );

	if ( BlockType:get_pcvar_num( giTypeCvar ) == BT_Remove )
	{
		while ( ( iEntity = find_ent_by_class( iEntity , "armoury_entity" ) ) )
		{
			if ( ( g_ArmouryTypes[get_pdata_int( iEntity , m_iType , XoCArmoury ) ] ) && pev_valid( iEntity ) )
				ExecuteHam( Ham_CS_Restart , iEntity );
		}
	}
	
	gBlockWeapons = 0;
		
	return PLUGIN_HANDLED;
}

public showkeylist( id , level, cid ) 
{ 
	if( !cmd_access( id, level, cid, 1 ) ) 
		return PLUGIN_HANDLED;
	
	client_print( id , print_console , "========== All weapons available to add to the list ========== " ); 
	
	for( new i = 1; i < sizeof gKeyList ; i++ ) 
	{ 
		if ( gKeyList[ i ][ 0 ] && !( gIgnoreWeapons & ( 1 << i ) ) ) 
			client_print( id , print_console , " %s" , gKeyList[ i ]  ); 
	} 
	
	return PLUGIN_HANDLED; 
} 

public blockedlist( id, level, cid )
{
	if( !cmd_access( id , level, cid, 1 ) )
		return PLUGIN_HANDLED;

	client_print( id , print_console , " ~~ All blocked weapons ~~~ " );

	for( new i = 1; i < sizeof gKeyList ; i++ )
	{
		if ( gKeyList[ i ][ 0 ] && ( gBlockWeapons & ( 1 << i ) ) )
				client_print( id , print_console , "%s", gKeyList[ i ] );
	}
	return PLUGIN_HANDLED;
}

public player_touch( ent , id )  
{  
	if( !pev_valid( ent ) || !id || !( pev( ent , pev_flags) & FL_ONGROUND ) )  
		return -1; 
	
	new iWeaponID , szClassName[ 15 ] , WeaponTypes:wtType; 
	
	pev( ent , pev_classname , szClassName , charsmax( szClassName ) ); 

	
	if( IsArmoury( szClassName ) )
	{
		iWeaponID = g_ArmouryTypes[get_pdata_int( ent , m_iType , XoCArmoury ) ];
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

	if ( iWeaponID && ( gBlockWeapons & ( 1 << iWeaponID ) ) )  
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
			case BT_BlockPickup: 
			{	
				return PLUGIN_HANDLED; 
			}
			default: 
			{
				return PLUGIN_CONTINUE; 
			}
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

GetWeaponIndex(id, const szKeyword[], FunctionType:Type)
{
	new iFound = GWI_NotFound;
	
	for ( new iWeapon = 1 ; iWeapon < sizeof( gKeyList ) ; iWeapon++ )
	{
		if ( containi( gKeyList[ iWeapon ] , szKeyword ) > -1 )
		{
			if ( iFound > GWI_NotFound )
			{
				iFound = GWI_Duplicate;
				break;
			}
			else
			{
				iFound = iWeapon;
			}
		}
	}
	
	new iReturn = iFound 

	switch ( iFound )
	{
		case GWI_NotFound: 
		{
			client_print( id , print_console, "Unable to find the key '%s', type amx_cspum_keylist for all keys available to insert." , szKeyword ); 
			iReturn = GWI_NotFound; 
		}
		case GWI_Duplicate:
		{	
			client_print( id , print_console, "Please be more specific with the weapon you want to add/remove, type amx_cspum_blockedlist / amx_cspum_keylist." , szKeyword ); 
			iReturn = GWI_NotFound; 
		}
		default:
		{
			switch(Type)
			{
				case AddKey:
				{
					if ( gBlockWeapons & ( 1 << iFound ) )
					{
						client_print( id , print_console , "'%s' is already saved in the pick up manager list." , gKeyList[ iFound ] ); 
						iReturn = GWI_NotFound	
					}
				}
				case RemKey:
				{
					if ( !( gBlockWeapons & ( 1 << iFound ) ) )
					{
						client_print( id , print_console , "'%s' is not currently in the pick up manager list." , gKeyList[ iFound ] ); 
						iReturn = GWI_NotFound	
					}
				}
			}	
		}
	}
	
	return iReturn
}
