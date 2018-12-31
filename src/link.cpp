#include "link.h"
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <stdarg.h>
#include <sys/socket.h>

namespace sim{

	Link::Link(bool is_server){
		sock = -1;
		noblock_ = false;
		error_ = false;
		remote_ip[0] = '\0';
		remote_port = -1;

		if(is_server){
			//
		}else{

		}
	}

	Link::~Link(){
		this->close();
	}

	void Link::close(){
		if(sock >= 0){
			::close(sock);
			sock = -1;
		}
	}

	void Link::nodelay(bool enable){
		int opt = enable? 1 : 0;
		::setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (void *)&opt, sizeof(opt));
	}

	void Link::keepalive(bool enable){
		int opt = enable? 1 : 0;
		::setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, (void *)&opt, sizeof(opt));
	}

	void Link::noblock(bool enable){
		noblock_ = enable;
		if(enable){
			::fcntl(sock, F_SETFL, O_NONBLOCK | O_RDWR);
		}else{
			::fcntl(sock, F_SETFL, O_RDWR);
		}
	}

	Link* Link::connect(const char *ip, int port){
		Link *link;
		int sock = -1;

		struct sockaddr_in addr;
		bzero(&addr, sizeof(addr));
		addr.sin_family = AF_INET;
		addr.sin_port = htons((short)port);
		inet_pton(AF_INET, ip, &addr.sin_addr);

		if((sock = ::socket(AF_INET, SOCK_STREAM, 0)) == -1){
			goto sock_err;
		}
		if(::connect(sock, (struct sockaddr *)&addr, sizeof(addr)) == -1){
			goto sock_err;
		}

		link = new Link();
		link->sock = sock;
		link->keepalive(true);
		return link;
	sock_err:
		if(sock >= 0){
			::close(sock);
		}
		return NULL;
	}

	Link* Link::listen(const char *ip, int port){
		Link *link;
		int sock = -1;

		int opt = 1;
		struct sockaddr_in addr;
		bzero(&addr, sizeof(addr));
		addr.sin_family = AF_INET;
		addr.sin_port = htons((short)port);
		inet_pton(AF_INET, ip, &addr.sin_addr);

		if((sock = ::socket(AF_INET, SOCK_STREAM, 0)) == -1){
			goto sock_err;
		}
		if(::setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1){
			goto sock_err;
		}
		if(::bind(sock, (struct sockaddr *)&addr, sizeof(addr)) == -1){
			goto sock_err;
		}
		if(::listen(sock, 1024) == -1){
			goto sock_err;
		}

		link = new Link(true);
		link->sock = sock;
		snprintf(link->remote_ip, sizeof(link->remote_ip), "%s", ip);
		link->remote_port = port;
		return link;
	sock_err:
		if(sock >= 0){
			::close(sock);
		}
		return NULL;
	}
        
	Link* Link::accept(){
		Link *link;
		int client_sock;
		struct sockaddr_in addr;
		socklen_t addrlen = sizeof(addr);

		while((client_sock = ::accept(sock, (struct sockaddr *)&addr, &addrlen)) == -1){
			if(errno != EINTR){
				return NULL;
			}
		}
		
		struct linger opt = {1, 0};
		int ret = ::setsockopt(client_sock, SOL_SOCKET, SO_LINGER, (void *)&opt, sizeof(opt));
		if(ret != 0){
			// log_error()
		}

		link = new Link();
		link->sock = client_sock;
		link->keepalive(true);
		inet_ntop(AF_INET, &addr.sin_addr, link->remote_ip, sizeof(link->remote_ip));
		link->remote_port = ntohs(addr.sin_port);
		return link;
	}

	int Link::read(){
		int ret = 0;
		char buf[16 * 1024];
		int want = sizeof(buf);
		while(1){
			int len = ::read(sock, buf ,want);
			if(len == -1){
				if(errno == EINTR){
					continue;
				}else if(errno == EWOULDBLOCK){
					break;
				}else{
					return -1;
				}
			}else{
				if(len == 0){
					return 0;
				}
				ret += len;
				decoder_.push(buf, len);
			}
			if(!noblock_){
				break;
			}
		}
		return ret;
	}

	int Link::write(){
		int ret = 0;
		int want;
		while((want = output_.size()) > 0){
			int len = ::write(sock, output_.data(), want);
			if(len == -1){
				if(errno == EINTR){
					continue;
				}else if(errno == EWOULDBLOCK){
					break;
				}else{
					return -1;
				}
			}else{
				if(len == 0){
					break;
				}
				ret += len;
				output_ = std::string(output_.data() + len,output_.size() - len);
			}
			if(!noblock_){
				break;
			}
		}
		return ret;
	}

	int Link::flush(){
		int len = 0;
		while(!output_.empty()){
			int ret = this->write();
			if(ret == -1){
				return -1;
			}
			len += ret;
		}
		return len;
	}

	int Link::parse(Message *msg){
		return decoder_.parse(msg);
	}
};


