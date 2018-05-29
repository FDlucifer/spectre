#include "headers.h"
#include <ctype.h>

void help_command( args *arguments );
void host_command( args *arguments );
void ls_command( args *arguments );
void cat_command( args *arguments );
void time_command( args *arguments );
void probe_command( args *arguments );
void ps_command( args *arguments );
void kill_command( args *arguments );
void exec_command( args *arguments );
void connect_command( args *arguments );
void exit_command( args *arguments );

commands command_list[ MAX_COMMANDS ] = {
   { "help",    "Get help about available system commands.", help_command },
   { "host",    "Get info on the current host.", host_command },
   { "ls",      "List the files on the current host.", ls_command },
   { "cat",     "Cat the contents of a file to the screen.", cat_command },
   { "time",    "Get the current system time.", time_command },
   { "probe",   "Probe a system to determine its type.", probe_command },
   { "ps",      "List the processes on the current host.", ps_command },
   { "kill",    "Kill a process on the current system.", kill_command },
   { "exec",    "Create a process from an executable file.", exec_command },
   { "connect", "Connect to a system using its IP address.", connect_command },
   { "exit",    "Exit the current system.", exit_command },
};

void help_command( args *arguments )
{
   char line[MAX_MSG_SIZE];

   for ( int i = 0 ; i < MAX_COMMANDS ; i++ )
   {
      sprintf( line, "%-12s | %s", 
               command_list[ i ].name, command_list[ i ].help );
      add_message( line );
   }
}

void host_command( args *arguments )
{
   char line[MAX_MSG_SIZE];

   sprintf( line, "%s (%s)", systems[ current_system( ) ].hostinfo, 
               systems[ current_system( ) ].ip_address );

   add_message( line );

   return;
}

void ls_command( args *arguments )
{
   char line[MAX_MSG_SIZE];

   for ( int i = 0 ; i < MAX_FILES ; i++ )
   {
      if ( systems[ current_system( )].filesystem.files[i].active )
      {
         sprintf( line, "%s %5u %-17s (%d)", 
                  systems[ current_system( ) ].filesystem.files[i].attributes,
                  (unsigned int)strlen( 
                     systems[ current_system( ) ].filesystem.files[i].contents),
                  systems[ current_system( ) ].filesystem.files[i].filename,
                  systems[ current_system( ) ].filesystem.files[i].quantity );

         add_message( line );
      }
   }
}

void cat_file( int file_index )
{
   char line[ MAX_MSG_SIZE ] = { 0 };
   char *file;
   int wrindex = 0, len;

   file = systems[ current_system( ) ].
                        filesystem.files[ file_index ].contents;

   len = strlen( file );

   for ( int i = 0 ; i < len ; i++ )
   {
      if ( file[ i ] == '\n' )
      {
         line[ wrindex ] = 0;
         add_message( line );
         wrindex = 0;
         line[ 0 ] = 0;
      }
      else line[ wrindex++ ] = file[ i ];
   }
   
   return;
}

void cat_command( args *arguments )
{
   int file_index;

   if ( arguments->num_args < 2 ) return;

   file_index = find_file( arguments->args[ 1 ] );

   if ( file_index != -1 )
   {
      cat_file( file_index );
      return;
   }

   add_message( "File not found." );

   return;
}

void time_command( args *arguments )
{
   char line[MAX_MSG_SIZE];

   sprintf( line, "%7.2f", 
            (float)(GameTime/100 + systems[ current_system( ) ].flags.timezone ) );

   add_message( line );

   return;
}

void probe_command( args *arguments )
{
   if ( arguments->num_args < 2 ) return;

   for ( int i = 0 ; i < NUM_SYSTEMS ; i++ )
   {
      int size = MAX( strlen( systems[ i ].ip_address ),
                      strlen( arguments->args[ 1 ] ) );

      if ( strncmp( systems[ i ].ip_address, 
            arguments->args[ 1 ], size ) == 0 )
      {
         if ( systems[ i ].flags.discoverable )
         {
            if ( systems[ i ].flags.probeable )
            {
               char line[MAX_MSG_SIZE];

               sprintf( line, "%s (%s)", systems[ i ].hostinfo, 
                           systems[ i ].ip_address );

               add_message( line );
            }
            else
            {
               add_message( "Host found but cannot probe." );
            }

            return;
         }

      }

   }

   add_message( "Host not found." );

   return;
}

void ps_command( args *arguments )
{
   char line[ MAX_MSG_SIZE ];
   char *state_names[] = { "INVALID", "INSTALLING", "RUNNING", 
                           "SLEEPING", "ZOMBIE" };
   processes_t *processes = &systems[ current_system( ) ].processes;

   add_message( "PID  NAME              STATE" );

   for ( int i = 0 ; i < MAX_PROCESSES ; i++ )
   {
      if ( processes->process[i].flags.active )
      {
         sprintf( line, "%4d %-17s %s", processes->process[i].pid, 
                  processes->process[i].name, 
                  state_names[ processes->process[i].state ] );
         add_message( line );
      }
   }

   return;
}

void kill_command( args *arguments )
{
   char line[ MAX_MSG_SIZE ];
   processes_t *processes = &systems[ current_system( ) ].processes;

   if ( arguments->num_args < 2 ) return;

   for ( int i = 0 ; i < MAX_PROCESSES ; i++ )
   {
      if ( processes->process[ i ].pid == atoi( arguments->args[ 1 ] ) )
      {
         if ( processes->process[ i ].flags.active )
         {
            if ( processes->process[ i ].flags.killable )
            {
               processes->process[ i ].flags.active = 0;
               sprintf( line, "[%4d]+ Killed  %s",
                        processes->process[ i ].pid,
                        processes->process[ i ].name );
               add_message( line );
            }
            else
            {
               add_message( "Process not responding to kill request." );
            }
         }
         return;
      }
   }

   add_message( "Process not found." );

   return;
}

void exec_command( args *arguments )
{
   int  file_index, process_index;
   unsigned int arg = 0; 

   if ( arguments->num_args < 2 ) return;

   file_index = find_file( arguments->args[ 1 ] );

   if ( file_index != -1 )
   {
      process_index = find_empty_process( );

      if ( process_index != -1 )
      {
         if ( arguments->num_args == 3 )
         {
            arg = ( unsigned int ) atoi( arguments->args[ 2 ] );
         }
         create_process_from_file( 
            process_index, file_index, arg );
      }
      else
      {
         add_message( "Process space is full." );
      }

   }

   return;
}

void connect_command( args *arguments )
{
   char line[ MAX_MSG_SIZE ];
   int connection;

   if ( arguments->num_args < 2 ) return;

   connection = find_system( arguments->args[ 1 ] );

   if ( connection != -1 )
   {
      push_system( current_system( ) );
      set_current_system( connection );

      sprintf( line, "Connected to %s", systems[ connection ].ip_address );
      add_message( line );
   }
   else
   {
      add_message( "Could not connect to system." );
   }
   
   return;
}

void exit_command( args *arguments )
{
   if ( system_stack_empty( ) )
   {
      add_message( "Cannot exit." );
   }
   else
   {
      set_current_system( pop_system( ) );
      add_message( "Connection terminated." );
   }

   return;
}
