#ifndef __NX_HTTP_H__
#define __NX_HTTP_H__

#include <functional>

#include <nx/config.h>
#include <nx/tcp.hpp>
#include <nx/local_socket.hpp>
#include <nx/request.hpp>
#include <nx/reply.hpp>
#include <nx/handlers.hpp>
#include <nx/cond_var.hpp>
#include <nx/ws.hpp>

namespace nx {

using request_cb = std::function<
    void(request& req, buffer& data, reply& rep)
>;
using reply_cb = std::function<
    void(reply& rep, buffer& data)
>;

struct http_async_tag {};
struct http_sync_tag {};

template <template <typename, typename ...> class T>
class NX_API http : public T<http<T> >
{
public:
    using base_type = T<http<T> >;
    using ws_type = ws<T>;
    //using ws_type = ws;
    using this_type = http<T>;

    using base_type::T;

    http() = default;
    http(request&& req, reply_cb&& cb)
    : req_(std::move(req)),
    reply_cb_(std::move(cb))
    {}

    http(request&& req, reply_cb&& cb, asio::io_service& io)
    : base_type(io),
    req_(std::move(req)),
    reply_cb_(std::move(cb))
    {}

    http(reply&& rep, request_cb&& cb)
    : rep_(std::move(rep)),
    request_cb_(std::move(cb))
    {}

    http(const http& other) = delete;
    http(http&& other) = default;
    http& operator=(const http& other) = delete;
    http& operator=(http&& other) = default;

    void process_request()
    {
        call_or_fail(
            [&]() {
                if (!request_parsed() || this->rbuf().size() < this->req_.content_length()) {
                    // Wait until request is complete
                    return;
                }

                if (this->req_.is_form()) {
                    // Decode additional variables from body
                    // TODO: implement iterator based function to avoid copy
                    std::string body;
                    this->rbuf() >> body;
                    this->req_ << attributes(body, '&');
                }

                auto self = this->ptr();
                if (this->req_.is_upgrade()) {
                    ws_type::server_handshake(this->req_, this->rep_);

                    this->rep_ | [this,self]() mutable {
                        *this << this->rep_;

                        process_upgrade();
                    };

                    // All data arrived, call upper handler
                    this->request_cb_(this->req_, this->rbuf(), this->rep_);
                } else {
                    // Register callback to be called when reply is ready to send
                    // (will be called last by rep.done())
                    this->rep_ << connection_close;

                    this->rep_ | [this,self]() mutable {
                        *this << this->rep_;
                        this->close_after_write();
                        self.reset();
                    };

                    // All data arrived, call upper handler
                    this->request_cb_(this->req_, this->rbuf(), this->rep_);
                }
            }
        );

        if (!this->rep_.postponed()) {
            this->rep_.done();
        }
    }

    bool process_reply()
    {
        try {
            if (!reply_parsed() || this->rbuf().size() < this->rep_.content_length()) {
                // Wait until response is complete
                return false;
            }
        } catch (const http_status& s) {
            this->rep_ << s;
        } catch (const std::exception& e) {
            this->rep_ << BadResponse(e);
        }

        // All data arrived, call upper handler
        this->reply_cb_(this->rep_, this->rbuf());
        this->close();
        return true;
    }

    void send_request()
    {
        this->req_ << header("Host", this->local_str());
        *this << std::move(this->req_);
    }

    using base_type::operator<<;

    http& operator<<(request_cb cb)
    {
        this->request_cb_ = std::move(cb);

        return *this;
    }

    http& operator<<(reply_cb cb)
    {
        this->reply_cb_ = std::move(cb);

        return *this;
    }

    template<
        typename SocketType,
        typename ...Args
    >
    SocketType& upgrade_connection(Args&& ...args)
    {
        auto upg_ct = new_object<SocketType>(this->sock(), std::forward<Args>(args)...);
        auto& upg = *upg_ct;

        return upg;
    }

private:
    bool request_parsed()
    {
        if (!this->parsed_) {
           this-> parsed_ = this->req_.parse(this->rbuf());
        }

        return this->parsed_;
    }

    bool reply_parsed()
    {
        if (!this->parsed_) {
           this-> parsed_ = this->rep_.parse(this->rbuf());
        }

        return this->parsed_;
    }

    void call_or_fail(void_cb cb)
    {
       try {
            cb();
        } catch (const http_status& s) {
            this->rep_ << s;
        } catch (const std::exception& e) {
            std::cout << "BadRequest by " << e.what() << std::endl;
            this->rep_ << BadRequest(e);
        }
    }

    void process_upgrade()
    {
        auto self = this->ptr();
        async() << [this,self]() {
            this->cancel();

            auto& w = this->upgrade_connection<ws_type>();
            w.set_callbacks(rep_.websocket_callback());

            this->dispose();
            w.start();
        };
    }

    bool parsed_ = false;
    request req_;
    reply rep_;
    request_cb request_cb_;
    reply_cb reply_cb_;
};

using http_tcp = http<tcp_base>;
using http_local = http<local_socket_base>;

template <typename OnRequest>
endpoint
serve(http_tcp& h, const endpoint& ep, OnRequest&& cb)
{
    return
        serve(
            h,
            ep,
            [cb = std::move(cb)](http_tcp& c) {
                c << std::move(cb);
            },
            [](http_tcp& c) {
                c.process_request();
            }
        );
}


template <typename OnReply>
http_tcp&
async_connect(const endpoint& ep, request&& req, OnReply&& cb)
{
    auto p = new_object<http_tcp>(std::move(req), std::move(cb));
    auto& h = *p;

    h[tags::on_read] = [](http_tcp& t) {
        t.process_reply();
    };

    return connect(
        h,
        ep,
        [](http_tcp& t) {
            t.send_request();
        }
    );
}

template <typename OnReply>
http_tcp&
sync_connect(const endpoint& ep, request&& req, OnReply&& cb, int32_t timeout_s)
{
    auto t = std::make_shared<task>();
    auto p = new_object<http_tcp>(std::move(req), std::move(cb), t->get_io_service());

    auto& h = *p;
    cond_var cv;

    h[tags::on_read] = [&cv](http_tcp& t) {
        if (t.process_reply()) {
            cv.notify();
        }
    };

    auto& result = connect(
        h,
        ep,
        [](http_tcp& t) {
            t.send_request();
        }
    );

    if (timeout_s > 0) {
        cv.wait_for((uint64_t)timeout_s * 1000);
    } else {
        cv.wait();
    }
    t->stop();

    return result;
}

// fonctions temporaires en attendant que je template
template <typename OnRequest>
endpoint_local
serve(http_local& h, const endpoint_local& ep, OnRequest&& cb)
{
    return
        serve(
            h,
            ep,
            [cb = std::move(cb)](http_local& c) {
                c << std::move(cb);
            },
            [](http_local& c) {
                c.process_request();
            }
        );
}


template <typename OnReply>
http_local&
async_connect(const endpoint_local& ep, request&& req, OnReply&& cb)
{
    auto p = new_object<http_local>(std::move(req), std::move(cb));
    auto& h = *p;

    h[tags::on_read] = [](http_local& t) {
        t.process_reply();
    };

    return connect(
        h,
        ep,
        [](http_local& t) {
            t.send_request();
        }
    );
}

template <typename OnReply>
http_local&
sync_connect(const endpoint_local& ep, request&& req, OnReply&& cb, int32_t timeout_s)
{
    auto t = std::make_shared<task>();
    auto p = new_object<http_local>(std::move(req), std::move(cb), t->get_io_service());

    auto& h = *p;
    cond_var cv;

    h[tags::on_read] = [&cv](http_local& t) {
        if (t.process_reply()) {
            cv.notify();
        }
    };

    auto& result = connect(
        h,
        ep,
        [](http_local& t) {
            t.send_request();
        }
    );

    if (timeout_s > 0) {
        cv.wait_for((uint64_t)timeout_s * 1000);
    } else {
        cv.wait();
    }
    t->stop();

    return result;
}

} // namespace nx

#endif // __NX_HTTP_H__
