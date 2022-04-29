#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <boost/certify/extensions.hpp>
#include <boost/certify/https_verification.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>
#include <simdjson.h>

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <cstdlib>
#include <functional>
#include <memory>
#include <string>

namespace po = boost::program_options;
namespace fs = std::filesystem;
namespace beast = boost::beast;
namespace http = beast::http;
namespace websocket = beast::websocket;
namespace asio = boost::asio;
namespace ssl = asio::ssl;
using tcp = boost::asio::ip::tcp;


// Report a failure
void fail(beast::error_code ec, char const* what)
{
    std::cerr << what << ": " << ec.message() << "\n";
}

// Sends a WebSocket message and prints the response
class session : public std::enable_shared_from_this<session>
{
    tcp::resolver m_resolver;
    websocket::stream<beast::ssl_stream<beast::tcp_stream>> m_ws;
    beast::flat_buffer m_buffer;
    std::string m_host;
    std::string m_endpoint;
    std::string m_text;
    http::response<http::string_body> m_res;

public:
    // Resolver and socket require an io_context
    explicit session(asio::io_context& ioc, ssl::context& ctx)
        : m_resolver(asio::make_strand(ioc))
        , m_ws(asio::make_strand(ioc), ctx)
    {
    }

    // Start the asynchronous operation
    void run(char const* host, char const* port, const char* endpoint, char const* text)
    {
        // Save these for later
        m_host = host;
        m_text = text;
        m_endpoint = endpoint;

        // Look up the domain name
        m_resolver.async_resolve(host, port,
            beast::bind_front_handler(&session::on_resolve, shared_from_this())
        );
    }

    void on_resolve( beast::error_code ec, tcp::resolver::results_type results)
    {
        if (ec)
        {
            return fail(ec, "resolve");
        }

        // Set the timeout for the operation
        beast::get_lowest_layer(m_ws).expires_after(std::chrono::seconds(30));

        // Make the connection on the IP address we get from a lookup
        beast::get_lowest_layer(m_ws).async_connect(
            results,
            beast::bind_front_handler(&session::on_connect, shared_from_this())
        );
    }

    void on_connect(beast::error_code ec, tcp::resolver::results_type::endpoint_type ep)
    {
        if (ec)
        {
            return fail(ec, "connect");
        }

        // Update the host_ string. This will provide the value of the
        // Host HTTP header during the WebSocket handshake.
        // See https://tools.ietf.org/html/rfc7230#section-5.4
        m_host += ':' + std::to_string(ep.port());

        // Set a timeout on the operation
        beast::get_lowest_layer(m_ws).expires_after(std::chrono::seconds(30));
        
        // Set SNI Hostname (many hosts need this to handshake successfully)
        if (!SSL_set_tlsext_host_name(
            m_ws.next_layer().native_handle(),
            m_host.c_str()))
        {
            ec = beast::error_code(static_cast<int>(::ERR_get_error()),
                asio::error::get_ssl_category());
            return fail(ec, "connect");
        }        

        // Set suggested timeout settings for the websocket
        m_ws.set_option(websocket::stream_base::timeout::suggested(beast::role_type::client));

        // Set a decorator to change the User-Agent of the handshake
        m_ws.set_option(websocket::stream_base::decorator(
            [](websocket::request_type& req)
            {
                req.set(http::field::user_agent,
                    std::string(BOOST_BEAST_VERSION_STRING) +
                    "boost-beast");
            })
        );

        // Perform the SSL handshake
        m_ws.next_layer().async_handshake(
            ssl::stream_base::client,
            beast::bind_front_handler(
                &session::on_ssl_handshake,
                shared_from_this()));
    } 

    void on_ssl_handshake(beast::error_code ec)
    {
        if (ec)
        {
            return fail(ec, "ssl_handshake");
        }

        // Turn off the timeout on the tcp_stream, because
        // the websocket stream has its own timeout system.
        beast::get_lowest_layer(m_ws).expires_never();

        // Set suggested timeout settings for the websocket
        m_ws.set_option(
            websocket::stream_base::timeout::suggested(
                beast::role_type::client));

        // Set a decorator to change the User-Agent of the handshake
        m_ws.set_option(websocket::stream_base::decorator(
            [](websocket::request_type& req)
            {
                req.set(http::field::user_agent,
                    std::string(BOOST_BEAST_VERSION_STRING) +
                    "boost-beast");
            }));

        // Perform the websocket handshake
        m_ws.async_handshake(m_host, m_endpoint,
            beast::bind_front_handler(
                &session::on_handshake,
                shared_from_this()));
    }

    void on_handshake(beast::error_code ec)
    {
        if (ec)
        {
            std::cout << m_res;
            return fail(ec, "handshake");
        }

        // Send the message
        m_ws.async_write(
            asio::buffer(m_text),
            beast::bind_front_handler(&session::on_write, shared_from_this())
        );
    }

    void on_write(beast::error_code ec, std::size_t bytes_transferred)
    {
        boost::ignore_unused(bytes_transferred);
        if (ec)
        {
            return fail(ec, "write");
        }

        // Read a message into our buffer
        m_ws.async_read(m_buffer,
            beast::bind_front_handler(&session::on_read, shared_from_this()));
    }

    void on_read(beast::error_code ec, std::size_t bytes_transferred)
    {
        boost::ignore_unused(bytes_transferred);
        if (ec)
        {
            return fail(ec, "read");
        }

        // Close the WebSocket connection
        m_ws.async_close(
            websocket::close_code::normal,
            beast::bind_front_handler(&session::on_close, shared_from_this())
        );
    }

    void on_close(beast::error_code ec)
    {
        if (ec)
        {
            return fail(ec, "close");
        }

        // If we get here then the connection is closed gracefully

        // The make_printable() function helps print a ConstBufferSequence
        std::cout << beast::make_printable(m_buffer.data()) << std::endl;
    }
};

namespace
{ 
    struct ProgramOptions
    {
        bool show_help_only{ false };
        fs::path input_path;
        fs::path output_path;
        fs::path primary_k_matrix_path;
        fs::path secondary_k_matrix_path;
        boost::optional<int> index_base{};
        bool production_tools_mode{ false };
    };

    ProgramOptions parse_commandline_arguments(int argc, char* argv[])
    {
        ProgramOptions options;
        po::options_description commandline_options("\nBioVolume Reconstruct allowed arguments");
        commandline_options.add_options()
            ("help,h", 
                "Produce this help message.");

        po::variables_map vm;
        store(po::command_line_parser(argc, argv).options(commandline_options).run(), vm);
        notify(vm);
        
        if (vm.count("help"))
        {
            options.show_help_only = true;
            std::cout << commandline_options << std::endl;
            return options;
        }
        
        return options;
    } 
}
 

int main(int argc, char* argv[])
{
    try
    {
        auto options = parse_commandline_arguments(argc, argv);
        if (options.show_help_only)
        {
            return 0;
        }

        std::cout << "Running..." << std::endl;

        auto const host = "www.bitmex.com";
        auto const port = "443";
        auto const endpoint = "/realtime";
        auto const text = "help";
        //auto const text2 = "issa ok";

        // The io_context is required for all I/O
        asio::io_context context;
        ssl::context ctx(ssl::context::tlsv12_client);
        ctx.set_verify_mode(ssl::context::verify_peer);
        boost::certify::enable_native_https_server_verification(ctx);

        // Launch the asynchronous operation
        std::make_shared<session>(context, ctx)->run(host, port, endpoint, text);
        //std::make_shared<session>(context)->run(host, port, text2);

        // Run the I/O service. The call will return when
        // the socket is closed.
        context.run();
    }    
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return -1;
    }
    catch  ( ... )
    {
        std::cerr << "Unknown exception." << std::endl;
        return -2;
    }

    return 0;
}
