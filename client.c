#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>
void error( const char *msg )
{
	perror( msg );
	exit( 0 );
}


void * pthread( void *arg );


int main( int argc, char *argv[] )
{
	int			sockfd, portno, n;
	struct sockaddr_in	serv_addr;
	struct hostent		*server;

	pthread_t tidp;

	char buffer[256];
	if ( argc < 2 )
	{
		fprintf( stderr, "usage %s hostname port\n", argv[0] );
		exit( 0 );
	}
	portno	= atoi( argv[1] );
	sockfd	= socket( AF_INET, SOCK_STREAM, 0 );
	if ( sockfd < 0 )
		error( "ERROR opening socket" );


	/*server = gethostbyname(argv[1]);
	 * if (server == NULL) {
	 *  fprintf(stderr,"ERROR, no such host\n");
	 *  exit(0);
	 * }
	 * bzero((char *) &serv_addr, sizeof(serv_addr));
	 *
	 * bcopy((char *)server->h_addr,
	 *   (char *)&serv_addr.sin_addr.s_addr,
	 *   server->h_length);
	 */
	serv_addr.sin_family		= AF_INET;
	serv_addr.sin_addr.s_addr	= inet_addr( "127.0.0.1" );
	serv_addr.sin_port		= htons( portno );
	if ( connect( sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr) ) < 0 )
		error( "ERROR connecting" );
	printf( "Please enter the message: \n" );
	bzero( buffer, 256 );

	if ( (pthread_create( &tidp, NULL, pthread, (void *) &sockfd ) ) == -1 )
	{
		printf( "create error!\n" );
		return(1);
	}

	while ( 1 )
	{
		bzero( buffer, 256 );
		n = read( sockfd, buffer, 255 );
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
	close( sockfd );
	return(0);
}


void * pthread( void *arg )
{
	int	sockfd = *(int *) arg;
	char	buffer[256];
	while ( 1 )
	{
		//printf( "s:" );
		fgets( buffer, 255, stdin );
		int	len	= strlen( buffer );
    if(buffer[len-1]=='\n')
      buffer[--len]='\0';
		int	n	= write( sockfd, buffer, len );
		if ( n < 0 )
			error( "ERROR writing to socket" );
	}
	return(NULL);
}


