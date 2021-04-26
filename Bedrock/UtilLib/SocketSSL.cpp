#include "SocketSSL.h"
#include "ExternalLibraries/openssl/include/openssl/ssl.h"
#include "ExternalLibraries/openssl/include/openssl/bio.h"
#pragma comment(lib,"ExternalLibraries/openssl/lib/libeay32.lib")
#pragma comment(lib,"ExternalLibraries/openssl/lib/ssleay32.lib")

CSocketSSL::CSocketSSL( void )
{
  ssl = NULL;
}

CSocketSSL::~CSocketSSL( void )
{
}

TS32 CSocketSSL::Connect( const CString &Server, const TU32 Port )
{
  if ( !CSocket::Connect( Server, Port ) ) return 0;

  SSL_library_init();
  SSL_load_error_strings();

  SSL_METHOD * meth = SSLv3_method();
  ctx = SSL_CTX_new( meth );

  ssl = SSL_new( (SSL_CTX*)ctx );
  BIO * sbio = BIO_new_socket( Socket, BIO_NOCLOSE );
  SSL_set_bio( (SSL*)ssl, sbio, sbio );
  if ( SSL_connect( (SSL*)ssl ) != 1 )
  {
    LogSSLError();
    return 0;
  }

  return 1;
}

TS32 CSocketSSL::Close()
{
  if ( ssl )
  {
    SSL_shutdown( (SSL*)ssl );
    SSL_free( (SSL*)ssl );
    SSL_CTX_free( (SSL_CTX*)ctx );
    ssl = ctx = NULL;
  }
  return CSocket::Close();
}

TS32 CSocketSSL::ReadStream( void *lpBuf, TU32 nCount )
{
  return SSL_read( (SSL*)ssl, lpBuf, nCount );
}

TS32 CSocketSSL::WriteStream( void* lpBuf, TU32 nCount )
{
  return SSL_write( (SSL*)ssl, lpBuf, nCount );
}

void CSocketSSL::LogSSLError()
{
  int n = -1;
  if ( ssl )
    n = SSL_get_error( (SSL*)ssl, -1 );
  LOG( LOG_ERROR, _T( "[sock] SSL Socket error %d" ), n );
}