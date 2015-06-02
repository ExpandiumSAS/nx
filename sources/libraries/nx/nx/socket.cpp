#include <sstream>

#include <cxxu/logging.hpp>

#include <nx/socket.hpp>

namespace nx {

socket::socket(service& s)
: s_(s),
socket_(s_.io_service())
{}

socket::~socket()
{}

void
socket::start()
{
    s_.register(shared_from_this());
    read();
}

void
socket::stop()
{
    s_.unregister(shared_from_this());
    close();
}

void
socket::read()
{
    auto self(shared_from_this());

    socket_.async_read_some(
        boost::asio::buffer(buffer_),
        [this, self](boost::system::error_code ec, std::size_t count) {

        if (!ec) {
            request_parser::result_type result;
          std::tie(result, std::ignore) = request_parser_.parse(
              request_, buffer_.data(), buffer_.data() + bytes_transferred);

          if (result == request_parser::good)
          {
            request_handler_.handle_request(request_, reply_);
            do_write();
          }
          else if (result == request_parser::bad)
          {
            reply_ = reply::stock_reply(reply::bad_request);
            do_write();
          }
          else
          {
            do_read();
          }
        } else if (ec != boost::asio::error::operation_aborted) {
            connection_manager_.stop(shared_from_this());
        }
    }
    );
}

void
socket::write()
{}

void
socket::close()
{
    if (!is_open()) {
        return;
    }

    boost::system::error_code ec;

    socket_.shutdown(asio::ip::tcp::socket::shutdown_both, ec);

    if (ec) {
        cxxu::warning() << "failed to shutdown on " << desc() << ": " << ec;
    }

    socket_.close(ec);

    if (ec) {
        cxxu::warning() << "failed to close on " << desc() << ": " << ec;
    }
}

std::string
socket::desc() const
{
    std::ostringstream oss;
    boost::system::error_code ec;

    auto ep = socket_.local_endpoint(ec);

    oss << "local(" << (ec ? "n/a" : ep) << ")";

    ep = socket_.remote_endpoint(ec);

    oss << " remote(" << (ec ? "n/a" : ep) << ")";

    return oss.str();
}

} // namespace nx
