/* The port number is passed as an argument */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
void error( const char *msg )
{
	perror( msg );
	exit( 1 );
}


void * pthread( void *arg );


int main( int argc, char *argv[] )
{
	int			sockfd, newsockfd, portno;
	socklen_t		clilen;
	char			buffer[256];
	struct sockaddr_in	serv_addr, cli_addr;
	int			n;
	if ( argc < 2 )
	{
		fprintf( stderr, "ERROR, no port provided\n" );
		exit( 1 );
	}
	sockfd = socket( AF_INET, SOCK_STREAM, 0 );
	if ( sockfd < 0 )
		error( "ERROR opening socket" );
	bzero( (char *) &serv_addr, sizeof(serv_addr) );
	portno				= atoi( argv[1] );
	serv_addr.sin_family		= AF_INET;
	serv_addr.sin_addr.s_addr	= INADDR_ANY;
	serv_addr.sin_port		= htons( portno );
	if ( bind( sockfd, (struct sockaddr *) &serv_addr,
		   sizeof(serv_addr) ) < 0 )
		error( "ERROR on binding" );
	listen( sockfd, 5 );
	clilen		= sizeof(cli_addr);
	newsockfd	= accept( sockfd,
				  (struct sockaddr *) &cli_addr,
				  &clilen );

	if ( newsockfd < 0 )
		error( "ERROR on accept" );
	pthread_t tidp;

	if ( (pthread_create( &tidp, NULL, pthread, (void *) &newsockfd ) ) == -1 )
	{
		printf( "create error!\n" );
		return(1);
	}
  printf( "Please enter the message: \n" );
	while ( 1 )
	{
		bzero( buffer, 256 );
		n = read( newsockfd, buffer, 255 );
		if ( n < 0 )
			error( "ERROR reading from socket" );
		printf( "r: %s\n", buffer );
	}
  /*
	if ( pthread_join( tidp, NULL ) )
	{
		printf( "thread is not exit...\n" );
		return(-2);
	}*/
	close( newsockfd );
	close( sockfd );
	return(0);
}


void * pthread( void *arg )
{
	int	newsockfd = *(int *) arg;
	char	buffer[256];
	while ( 1 )
	{
		//printf( "s: " );
		fgets(buffer, 255, stdin);
		int	len	= strlen( buffer );
    if(buffer[len-1]=='\n')
      buffer[--len]='\0';
		int	n	= write( newsockfd, buffer, len );
		if ( n < 0 )
			error( "ERROR writing to socket" );
	}
	return(NULL);
}


