#ifndef SIM_LINK_H_
#define SIM_LINK_H_

#include <stdio.h>
#include <string>
#include <vector>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include "decoder.h"

namespace sim{

	class Link{
	private:
		int sock;
		bool noblock_;
		bool error_;
		Decoder decoder_;
		std::string output_;
	public:
		char remote_ip[INET_ADDRSTRLEN];
		int remote_port;

		double create_time;
		double active_time;

		Link(bool is_server=false);
		~Link();
		void close();
		void nodelay(bool enable=true);
		void noblock(bool enable=true);
		void keepalive(bool enable=true);

		int fd() const{
			return sock;
		}
		bool error() const{
			return error_;
		}
		void mark_error(){
			error_ = true;
		}

		static Link* connect(const char *ip, int port);
		static Link* listen(const char *ip, int port);
		Link* accept();

		int read();
		int write();
		// flush buffered data to network
		// requires: nonblock
		int flush();
		
		int parse(Message *msg);
	};


}; // namespace sim

#endif
