#include <sstream>

#include <cxxu/logging.hpp>

#include <nx/socket.hpp>

namespace nx {

socket::socket(service& s)
: s_(s),
socket_(s_.io_service())
{
    auto self = shared_from_this();
    s_.register(self);
}

socket::~socket()
{}

void
socket::start()
{}

void
socket::stop()
{
    close();
    s_.unregister(shared_from_this());
}

void
socket::read()
{}

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
