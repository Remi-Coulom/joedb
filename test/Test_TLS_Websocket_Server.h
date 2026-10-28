#ifndef joedb_Test_TLS_Websocket_Server_declared
#define joedb_Test_TLS_Websocket_Server_declared

#include "joedb/error/assert.h"
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <openssl/pem.h>
#include <openssl/x509v3.h>
#include <openssl/rsa.h>
#include <thread>
#include <memory>
#include <fstream>
#include <filesystem>
#include <cstdlib>

namespace joedb
{
 struct Test_TLS_Certificate
 {
  std::string certificate;
  std::string private_key;

  Test_TLS_Certificate()
  {
   std::unique_ptr<EVP_PKEY_CTX, decltype(&EVP_PKEY_CTX_free)> context
    (EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, nullptr), EVP_PKEY_CTX_free);
   JOEDB_RELEASE_ASSERT(context);
   JOEDB_RELEASE_ASSERT(EVP_PKEY_keygen_init(context.get()) > 0);
   JOEDB_RELEASE_ASSERT(EVP_PKEY_CTX_set_rsa_keygen_bits(context.get(), 2048) > 0);
   EVP_PKEY *raw_key = nullptr;
   JOEDB_RELEASE_ASSERT(EVP_PKEY_keygen(context.get(), &raw_key) > 0);
   std::unique_ptr<EVP_PKEY, decltype(&EVP_PKEY_free)> key(raw_key, EVP_PKEY_free);
   std::unique_ptr<X509, decltype(&X509_free)> cert(X509_new(), X509_free);
   JOEDB_RELEASE_ASSERT(cert);
   JOEDB_RELEASE_ASSERT(X509_set_version(cert.get(), 2) == 1);
   JOEDB_RELEASE_ASSERT(ASN1_INTEGER_set(X509_get_serialNumber(cert.get()), 1) == 1);
   JOEDB_RELEASE_ASSERT(X509_gmtime_adj(X509_getm_notBefore(cert.get()), -60));
   JOEDB_RELEASE_ASSERT(X509_gmtime_adj(X509_getm_notAfter(cert.get()), 3600));
   JOEDB_RELEASE_ASSERT(X509_set_pubkey(cert.get(), key.get()) == 1);
   X509_NAME *name = X509_get_subject_name(cert.get());
   JOEDB_RELEASE_ASSERT(X509_NAME_add_entry_by_txt(name, "CN", MBSTRING_ASC,
    reinterpret_cast<const unsigned char *>("localhost"), -1, -1, 0) == 1);
   JOEDB_RELEASE_ASSERT(X509_set_issuer_name(cert.get(), name) == 1);
   X509V3_CTX extension_context{};
   X509V3_set_ctx(&extension_context, cert.get(), cert.get(), nullptr, nullptr, 0);
   for (const auto &entry: {std::pair<int, const char *>(NID_basic_constraints, "critical,CA:TRUE"),
                           std::pair<int, const char *>(NID_subject_alt_name, "DNS:localhost")})
   {
    std::unique_ptr<X509_EXTENSION, decltype(&X509_EXTENSION_free)> extension
     (X509V3_EXT_conf_nid(nullptr, &extension_context, entry.first, entry.second), X509_EXTENSION_free);
    JOEDB_RELEASE_ASSERT(extension);
    JOEDB_RELEASE_ASSERT(X509_add_ext(cert.get(), extension.get(), -1) == 1);
   }
   JOEDB_RELEASE_ASSERT(X509_sign(cert.get(), key.get(), EVP_sha256()) > 0);
   std::unique_ptr<BIO, decltype(&BIO_free)> cert_bio(BIO_new(BIO_s_mem()), BIO_free);
   std::unique_ptr<BIO, decltype(&BIO_free)> key_bio(BIO_new(BIO_s_mem()), BIO_free);
   JOEDB_RELEASE_ASSERT(cert_bio && key_bio);
   JOEDB_RELEASE_ASSERT(PEM_write_bio_X509(cert_bio.get(), cert.get()) == 1);
   JOEDB_RELEASE_ASSERT(PEM_write_bio_PrivateKey(key_bio.get(), key.get(), nullptr, nullptr, 0, nullptr, nullptr) == 1);
   char *data = nullptr;
   const auto cert_size = BIO_get_mem_data(cert_bio.get(), &data);
   certificate.assign(data, size_t(cert_size));
   const auto key_size = BIO_get_mem_data(key_bio.get(), &data);
   private_key.assign(data, size_t(key_size));
  }
 };

 class Test_TLS_Trust
 {
  private:
   const char *file_name = "websocket_ca.pem.tmp";
   const bool had_previous = std::getenv("SSL_CERT_FILE") != nullptr;
   const std::string previous = had_previous ? std::getenv("SSL_CERT_FILE") : "";

  public:
   explicit Test_TLS_Trust(const std::string &certificate)
   {
    std::ofstream(file_name) << certificate;
    const std::string path = std::filesystem::absolute(file_name).string();
    JOEDB_RELEASE_ASSERT(setenv("SSL_CERT_FILE", path.c_str(), 1) == 0);
   }
   ~Test_TLS_Trust()
   {
    if (had_previous)
     setenv("SSL_CERT_FILE", previous.c_str(), 1);
    else
     unsetenv("SSL_CERT_FILE");
    std::remove(file_name);
   }
 };

 class Test_TLS_Websocket_Server
 {
  private:
   using Socket = boost::asio::ip::tcp::socket;
   struct Session: std::enable_shared_from_this<Session>
   {
    boost::beast::websocket::stream<boost::asio::ssl::stream<Socket>> stream;
    boost::beast::flat_buffer data;
    const bool close_after_echo;
    Session(Socket socket, boost::asio::ssl::context &context, bool close_after_echo):
     stream(std::move(socket), context), close_after_echo(close_after_echo) {}
    void start()
    {
     stream.next_layer().async_handshake(boost::asio::ssl::stream_base::server,
      [self = shared_from_this()](const boost::system::error_code &error)
      {
       if (!error)
        self->stream.async_accept([self](const boost::system::error_code &accept_error)
        {
         if (!accept_error)
          self->read();
        });
      });
    }
    void read()
    {
     stream.async_read(data, [self = shared_from_this()](const boost::system::error_code &error, size_t)
     {
      if (!error)
      {
       self->stream.binary(true);
       self->stream.async_write(self->data.data(),
        [self](const boost::system::error_code &write_error, size_t)
        {
         if (!write_error)
         {
          self->data.consume(self->data.size());
          if (self->close_after_echo)
          {
           boost::system::error_code ignored;
           self->stream.next_layer().next_layer().close(ignored);
          }
          else
           self->read();
         }
        });
      }
     });
    }
   };
   boost::asio::io_context context;
   boost::asio::ssl::context tls{boost::asio::ssl::context::tls_server};
   boost::asio::ip::tcp::acceptor acceptor{context, {boost::asio::ip::address_v4::loopback(), 0}};
   boost::asio::steady_timer timeout{context};
   std::shared_ptr<Session> session;
   std::thread thread;

  public:
   explicit Test_TLS_Websocket_Server(const Test_TLS_Certificate &certificate, bool close_after_echo = false)
   {
    tls.use_certificate_chain(boost::asio::buffer(certificate.certificate));
    tls.use_private_key(boost::asio::buffer(certificate.private_key), boost::asio::ssl::context::pem);
    acceptor.async_accept([this, close_after_echo](const boost::system::error_code &error, Socket socket)
    {
     if (!error)
     {
      session = std::make_shared<Session>(std::move(socket), tls, close_after_echo);
      session->start();
     }
    });
    timeout.expires_after(std::chrono::seconds(5));
    timeout.async_wait([this](const boost::system::error_code &error)
    {
     if (!error && session)
     {
      boost::system::error_code ignored;
      session->stream.next_layer().next_layer().close(ignored);
     }
    });
    thread = std::thread([this]() {context.run();});
   }
   std::string port() const {return std::to_string(acceptor.local_endpoint().port());}
   ~Test_TLS_Websocket_Server()
   {
    context.stop();
    thread.join();
   }
 };
}

#endif
