#pragma once
#include <winsock2.h>
#include <string>

//定义文件类型对应的content-type
struct doc_type {
	const char* suffix;
	const char* type;
};

//文件格式
struct doc_type file_type[] = {
	{"html","text/html"},
	{"gif","imag/gif"},
	{"jpg","imag/jpeg"},
	{nullptr,nullptr}
};

const char* http_res_hdr_tmp1 = "HTTP/1.1 200 OK \r\nServer:Win<0.1>\r\n"
"Accept-Ranges:bytes\r\nContent-Length:%d\r\nConnection:close\r\n"
"Content-Type:%s\r\n\r\n";
const char* http_get_type(const char* suffix);			//匹配后缀获得对应的content-type
void http_get_filename(char* buf, int buflen, char* filename, char* suffix);			//解析url获得文件名称
int http_send_response(SOCKET s, char* buf, int buf_len);		//服务器对文件做出响应
extern struct doc_type;
extern struct doc_type file_type[];