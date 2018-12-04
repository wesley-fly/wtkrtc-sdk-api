/**
 * @file wtk_http_api.h
 * @brief API for WTK SDK 
 * @author Xiafaon Lee <xiaofan.lee@wattertek.com>
 */

#ifndef WTK_SYNC_HTTP_API_H
#define WTK_SYNC_HTTP_API_H

#include <string>
#include <vector>
#include <map>

#define REST_API_PATH			"/rest/usr"
#define HTTP_TIMEOUT			25

class WtkSyncHttpAPI
{
public:
	WtkSyncHttpAPI();
	~WtkSyncHttpAPI();
	
	static std::map<std::string, std::string> MobileNewCallAPI(std::string callee_pid, std::string caller_pid_cid,char* callid, int calltype, int media_server_index);
	static std::map<std::string, std::string> MobileNewOutboundCall(std::string callee_pid,std::string caller_pid_cid,char* callid,std::string posInfo,std::string msInfo, int calltype);
	static std::map<std::string, std::string> ReportCallStatsAPI(std::string caller_pid_cid,std::string staticsJsonData,std::string serverCallid,int duration);
private:
	static std::string SendHTTPRequest(std::string baseurl,std::string path,std::string param,int trytimes,int tryinterval,std::string capath);
	static std::string HttpPostSync(std::string baseurl,std::string path,int timeout,std::string param,std::string capath, int &retcode);
	static std::string HttpGetSync(std::string baseurl,std::string path,int timeout,std::string param);
};
#endif
