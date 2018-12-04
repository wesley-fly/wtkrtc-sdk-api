/**
 * @file wtk_http_api.cc
 * @brief API for WTK SDK 
 * @author Xiafaon Lee <xiaofan.lee@wattertek.com>
 */
 
#include "wtk_sync_http_api.h"
#include "connection.h"
#include "restclient.h"
#include "base/logging.h"
#include "system_wrappers/include/sleep.h"
#include "wtkrtc_mediaengine/wtk_media_sdk/common/wtk_media_instance.h"
#include "base/json/json_reader.h"
#include "base/json/json_writer.h"
#include "base/values.h"
#include "wtkrtc_mediaengine/wtk_media_sdk/common/wtk_media_sdk_misc_define.h"

WtkSyncHttpAPI::WtkSyncHttpAPI()
{
}

WtkSyncHttpAPI::~WtkSyncHttpAPI()
{
}
std::map<std::string, std::string> WtkSyncHttpAPI::MobileNewCallAPI(std::string callee_pid, std::string caller_pid_cid,char* callid, int calltype, int media_server_index)
{
	std::map<std::string, std::string> out_map;
	
	WtkMediaInstance *wtkMediaInstance = WtkMediaInstance::SharedWtkMediaInstance();
	if(!wtkMediaInstance)
	{
		DLOG(ERROR) << __FUNCTION__ << ",Couldn't get SharedWtkMediaInstance!";
		out_map[MSG_JSON_KEY_OPER_RESULT] = ERROR_HTTP_INSTANCE;
		return out_map;
	}
	
	UserProInfo tmp_user_profile;
	WtkUserProfile *cur_user_profile = wtkMediaInstance->GetUserProfile();
	cur_user_profile->FindUserProfileBySrcId(caller_pid_cid, &tmp_user_profile);
	
	if(tmp_user_profile.user_cs_server_.length() > 0 && tmp_user_profile.user_cs_port_.length() > 0)
	{
		std::string json_string;
		std::string call_id = callid;
		std::string media_server_port = cur_user_profile->vec_user_media_info_[media_server_index].user_media_server_ + ":" + cur_user_profile->vec_user_media_info_[media_server_index].user_media_port_;
		
		std::unique_ptr<base::DictionaryValue> period_dict0(new base::DictionaryValue());
		period_dict0->SetKey("peer", base::Value(callee_pid));
		period_dict0->SetKey("relay", base::Value(media_server_port));
		period_dict0->SetKey("tag", base::Value(call_id));
		period_dict0->SetKey("calltype", base::Value(calltype));

		std::unique_ptr<base::DictionaryValue> period_dict1(new base::DictionaryValue());
		period_dict1->SetKey("type", base::Value("MakeNewCall"));
		period_dict1->SetWithoutPathExpansion("value", std::move(period_dict0));

		std::unique_ptr<base::DictionaryValue> period_dict2(new base::DictionaryValue());
		period_dict2->SetKey("cid", base::Value(tmp_user_profile.user_cid_));
		period_dict2->SetKey("tag", base::Value(226));
		period_dict2->SetKey("pid", base::Value(tmp_user_profile.user_pid_));
		period_dict2->SetWithoutPathExpansion("cmd", std::move(period_dict1));

		std::unique_ptr<base::DictionaryValue> period_dict3(new base::DictionaryValue());
		period_dict3->SetKey("type", base::Value("SIG"));
		period_dict3->SetWithoutPathExpansion("value", std::move(period_dict2));

		base::DictionaryValue root_dict;
		root_dict.SetKey("uid", base::Value(tmp_user_profile.user_uid_));
		root_dict.SetKey("tok", base::Value(tmp_user_profile.user_token_));
		root_dict.SetWithoutPathExpansion("req", std::move(period_dict3));

		base::JSONWriter::Write(root_dict, &json_string);

		DLOG(INFO) << __FUNCTION__ << ", Request params json_string = " << json_string;

		std::string rest_api_result;
		if(cur_user_profile->user_use_https_)
		{
			std::string base_url = "https://" + tmp_user_profile.user_cs_server_ + ":" + tmp_user_profile.user_cs_port_;
			rest_api_result = SendHTTPRequest(base_url,REST_API_PATH,json_string,cur_user_profile->user_http_retry_times_,cur_user_profile->user_http_retry_wait_time_,cur_user_profile->user_https_ca_path_);
		}
		else
		{
			std::string base_url = "http://" + tmp_user_profile.user_cs_server_ + ":" + tmp_user_profile.user_cs_port_;
			rest_api_result = SendHTTPRequest(base_url,REST_API_PATH,json_string,cur_user_profile->user_http_retry_times_,cur_user_profile->user_http_retry_wait_time_,"");
		}
		DLOG(INFO) << __FUNCTION__ << ", RestAPI result = " << rest_api_result;
		if(rest_api_result.length() > 0)
		{
			std::unique_ptr<base::Value> root = base::JSONReader::Read(rest_api_result);
    		if(root)
    		{
    			base::DictionaryValue* root_dict = nullptr;
    			if(root->GetAsDictionary(&root_dict))
    			{
					std::string ret_api_type;
					root_dict->GetString("rsp.value.rlt.type", &ret_api_type);
					if(!ret_api_type.compare("MakeNewCall.Success"))
					{
						std::string ret_core_server_callid;
						root_dict->GetString("rsp.value.rlt.value.callId", &ret_core_server_callid);
						
						out_map[MSG_JSON_KEY_OPER_RESULT] = ERROR_NONE;
						out_map[MSG_JSON_KEY_CS_CALLID] = ret_core_server_callid;
						return out_map;
					}
					else if(!ret_api_type.compare("MakeNewCall.PeerOffline"))
					{
						out_map[MSG_JSON_KEY_OPER_RESULT] = ERROR_RESTAPI_PEEROFFLINE;
						return out_map;
					}
					else if(!ret_api_type.compare("MakeNewCall.Rejected"))
					{
						out_map[MSG_JSON_KEY_OPER_RESULT] = ERROR_RESTAPI_REJECTED;
						return out_map;
					}
					else if(!ret_api_type.compare("MakeNewCall.PeerNotFound"))
					{
						out_map[MSG_JSON_KEY_OPER_RESULT] = ERROR_RESTAPI_PEERNOTFOUND;
						return out_map;
					}
					else if(!ret_api_type.compare("MakeNewCall.Denied"))
					{
						out_map[MSG_JSON_KEY_OPER_RESULT] = ERROR_RESTAPI_DENIED;
						return out_map;
					}
					else if(!ret_api_type.compare("MakeNewCall.TransmitTimeout"))
					{
						out_map[MSG_JSON_KEY_OPER_RESULT] = ERROR_RESTAPI_TRANSMITTIMEOUT;
						return out_map;
					}
					else
					{
						out_map[MSG_JSON_KEY_OPER_RESULT] = ERROR_RESTAPI_OTHER;
						return out_map;
					}
    			}
				else
				{
					out_map[MSG_JSON_KEY_OPER_RESULT] = ERROR_HTTP_READ_JSON;
					return out_map;
				}
    		}
			else
			{
				out_map[MSG_JSON_KEY_OPER_RESULT] = ERROR_HTTP_READ_JSON;
				return out_map;
			}
		}
		else
		{
			out_map[MSG_JSON_KEY_OPER_RESULT] = ERROR_HTTP_RETRY_TIMEOUT;
			return out_map;
		}
	}
	else
	{
		out_map[MSG_JSON_KEY_OPER_RESULT] = ERROR_HTTP_PROFILE;
		return out_map;
	}

}
std::map<std::string, std::string> WtkSyncHttpAPI::MobileNewOutboundCall(std::string callee_pid,std::string caller_pid_cid,char* callid,std::string posInfo,std::string msInfo, int calltype)
{
	std::map<std::string, std::string> out_map;
	
	WtkMediaInstance *wtkMediaInstance = WtkMediaInstance::SharedWtkMediaInstance();
	if(!wtkMediaInstance)
	{
		DLOG(ERROR) << __FUNCTION__ << ",Couldn't get SharedWtkMediaInstance!";
		out_map[MSG_JSON_KEY_OPER_RESULT] = ERROR_HTTP_INSTANCE;
		return out_map;
	}
	
	UserProInfo tmp_user_profile;
	WtkUserProfile *cur_user_profile = wtkMediaInstance->GetUserProfile();
	cur_user_profile->FindUserProfileBySrcId(caller_pid_cid, &tmp_user_profile);
	
	if(tmp_user_profile.user_cs_server_.length() > 0 && tmp_user_profile.user_cs_port_.length() > 0)
	{
		std::string json_string;
		std::string call_id = callid;
		std::string pos_info = posInfo;
		std::string ms_info = msInfo;
		
		std::unique_ptr<base::DictionaryValue> period_dict0(new base::DictionaryValue());
		period_dict0->SetKey("peer", base::Value(callee_pid));
		period_dict0->SetKey("positionInfo", base::Value(pos_info));
		period_dict0->SetKey("relayServer", base::Value(ms_info));
		period_dict0->SetKey("tag", base::Value(call_id));
		period_dict0->SetKey("calltype", base::Value(calltype));

		std::unique_ptr<base::DictionaryValue> period_dict1(new base::DictionaryValue());
		period_dict1->SetKey("type", base::Value("MakeNewServiceCall"));
		period_dict1->SetWithoutPathExpansion("value", std::move(period_dict0));

		std::unique_ptr<base::DictionaryValue> period_dict2(new base::DictionaryValue());
		period_dict2->SetKey("cid", base::Value(tmp_user_profile.user_cid_));
		period_dict2->SetKey("tag", base::Value(227));
		period_dict2->SetKey("pid", base::Value(tmp_user_profile.user_pid_));
		period_dict2->SetWithoutPathExpansion("cmd", std::move(period_dict1));

		std::unique_ptr<base::DictionaryValue> period_dict3(new base::DictionaryValue());
		period_dict3->SetKey("type", base::Value("SIG"));
		period_dict3->SetWithoutPathExpansion("value", std::move(period_dict2));

		base::DictionaryValue root_dict;
		root_dict.SetKey("uid", base::Value(tmp_user_profile.user_uid_));
		root_dict.SetKey("tok", base::Value(tmp_user_profile.user_token_));
		root_dict.SetWithoutPathExpansion("req", std::move(period_dict3));

		base::JSONWriter::Write(root_dict, &json_string);

		DLOG(INFO) << __FUNCTION__ << ", Request params json_string = " << json_string;

		std::string rest_api_result;
		if(cur_user_profile->user_use_https_)
		{
			std::string base_url = "https://" + tmp_user_profile.user_cs_server_ + ":" + tmp_user_profile.user_cs_port_;
			rest_api_result = SendHTTPRequest(base_url,REST_API_PATH,json_string,cur_user_profile->user_http_retry_times_,cur_user_profile->user_http_retry_wait_time_,cur_user_profile->user_https_ca_path_);
		}
		else
		{
			std::string base_url = "http://" + tmp_user_profile.user_cs_server_ + ":" + tmp_user_profile.user_cs_port_;
			rest_api_result = SendHTTPRequest(base_url,REST_API_PATH,json_string,cur_user_profile->user_http_retry_times_,cur_user_profile->user_http_retry_wait_time_,"");
		}
		DLOG(INFO) << __FUNCTION__ << ", RestAPI result = " << rest_api_result;
		if(rest_api_result.length() > 0)
		{
			std::unique_ptr<base::Value> root = base::JSONReader::Read(rest_api_result);
			if(root)
			{
				base::DictionaryValue* root_dict = nullptr;
				if(root->GetAsDictionary(&root_dict))
				{
					std::string ret_api_type;
					root_dict->GetString("rsp.value.rlt.type", &ret_api_type);
					if(!ret_api_type.compare("MakeNewServiceCall.Success"))
					{
						std::string ret_core_server_callid;
						std::string ret_core_server_ticket;
						root_dict->GetString("rsp.value.rlt.value.callId", &ret_core_server_callid);
						root_dict->GetString("rsp.value.rlt.value.ticket", &ret_core_server_ticket);
						
						out_map[MSG_JSON_KEY_OPER_RESULT] = ERROR_NONE;
						out_map[MSG_JSON_KEY_CS_CALLID] = ret_core_server_callid;
						out_map[MSG_JSON_KEY_CS_TICKETID] = ret_core_server_ticket;
						return out_map;
					}
					else if(!ret_api_type.compare("MakeNewServiceCall.PeerOffline"))
					{
						out_map[MSG_JSON_KEY_OPER_RESULT] = ERROR_RESTAPI_PEEROFFLINE;
						return out_map;
					}
					else if(!ret_api_type.compare("MakeNewServiceCall.Rejected"))
					{
						out_map[MSG_JSON_KEY_OPER_RESULT] = ERROR_RESTAPI_REJECTED;
						return out_map;
					}
					else if(!ret_api_type.compare("MakeNewServiceCall.PeerNotFound"))
					{
						out_map[MSG_JSON_KEY_OPER_RESULT] = ERROR_RESTAPI_PEERNOTFOUND;
						return out_map;
					}
					else if(!ret_api_type.compare("MakeNewServiceCall.Denied"))
					{
						out_map[MSG_JSON_KEY_OPER_RESULT] = ERROR_RESTAPI_DENIED;
						return out_map;
					}
					else if(!ret_api_type.compare("MakeNewServiceCall.TransmitTimeout"))
					{
						out_map[MSG_JSON_KEY_OPER_RESULT] = ERROR_RESTAPI_TRANSMITTIMEOUT;
						return out_map;
					}
					else
					{
						out_map[MSG_JSON_KEY_OPER_RESULT] = ERROR_RESTAPI_OTHER;
						return out_map;
					}
				}
				else
				{
					out_map[MSG_JSON_KEY_OPER_RESULT] = ERROR_HTTP_READ_JSON;
					return out_map;
				}
			}
			else
			{
				out_map[MSG_JSON_KEY_OPER_RESULT] = ERROR_HTTP_READ_JSON;
				return out_map;
			}
		}
		else
		{
			out_map[MSG_JSON_KEY_OPER_RESULT] = ERROR_HTTP_RETRY_TIMEOUT;
			return out_map;
		}
	}
	else
	{
		out_map[MSG_JSON_KEY_OPER_RESULT] = ERROR_HTTP_PROFILE;
		return out_map;
	}
}
std::map<std::string, std::string> WtkSyncHttpAPI::ReportCallStatsAPI(std::string caller_pid_cid,std::string staticsJsonData,std::string serverCallid,int duration)
{
	std::map<std::string, std::string> out_map;
	
	WtkMediaInstance *wtkMediaInstance = WtkMediaInstance::SharedWtkMediaInstance();
	if(!wtkMediaInstance)
	{
		DLOG(ERROR) << __FUNCTION__ << ",Couldn't get SharedWtkMediaInstance!";
		out_map[MSG_JSON_KEY_OPER_RESULT] = ERROR_HTTP_INSTANCE;
		return out_map;
	}
	
	UserProInfo tmp_user_profile;
	WtkUserProfile *cur_user_profile = wtkMediaInstance->GetUserProfile();
	cur_user_profile->FindUserProfileBySrcId(caller_pid_cid, &tmp_user_profile);
	
	if(tmp_user_profile.user_cs_server_.length() > 0 && tmp_user_profile.user_cs_port_.length() > 0)
	{
		std::string json_string;
		
		std::unique_ptr<base::DictionaryValue> period_dict0(new base::DictionaryValue());
		period_dict0->SetKey("callId", base::Value(serverCallid));
		period_dict0->SetKey("duration", base::Value(duration));
		std::unique_ptr<base::DictionaryValue> param_dict = base::DictionaryValue::From(base::JSONReader().Read(staticsJsonData));
		period_dict0->SetWithoutPathExpansion("result", std::move(param_dict));
		
		std::unique_ptr<base::DictionaryValue> period_dict1(new base::DictionaryValue());
		period_dict1->SetKey("type", base::Value("ReportCallResult"));
		period_dict1->SetWithoutPathExpansion("value", std::move(period_dict0));

		std::unique_ptr<base::DictionaryValue> period_dict2(new base::DictionaryValue());
		period_dict2->SetKey("cid", base::Value(tmp_user_profile.user_cid_));
		period_dict2->SetKey("tag", base::Value(488));
		period_dict2->SetKey("pid", base::Value(tmp_user_profile.user_pid_));
		period_dict2->SetWithoutPathExpansion("cmd", std::move(period_dict1));

		std::unique_ptr<base::DictionaryValue> period_dict3(new base::DictionaryValue());
		period_dict3->SetKey("type", base::Value("SIG"));
		period_dict3->SetWithoutPathExpansion("value", std::move(period_dict2));

		base::DictionaryValue root_dict;
		root_dict.SetKey("uid", base::Value(tmp_user_profile.user_uid_));
		root_dict.SetKey("tok", base::Value(tmp_user_profile.user_token_));
		root_dict.SetWithoutPathExpansion("req", std::move(period_dict3));

		base::JSONWriter::Write(root_dict, &json_string);

		DLOG(INFO) << __FUNCTION__ << ", Request params json_string = " << json_string;

		std::string rest_api_result;
		if(cur_user_profile->user_use_https_)
		{
			std::string base_url = "https://" + tmp_user_profile.user_cs_server_ + ":" + tmp_user_profile.user_cs_port_;
			rest_api_result = SendHTTPRequest(base_url,REST_API_PATH,json_string,cur_user_profile->user_http_retry_times_,cur_user_profile->user_http_retry_wait_time_,cur_user_profile->user_https_ca_path_);
		}
		else
		{
			std::string base_url = "http://" + tmp_user_profile.user_cs_server_ + ":" + tmp_user_profile.user_cs_port_;
			rest_api_result = SendHTTPRequest(base_url,REST_API_PATH,json_string,cur_user_profile->user_http_retry_times_,cur_user_profile->user_http_retry_wait_time_,"");
		}
		DLOG(INFO) << __FUNCTION__ << ", RestAPI result = " << rest_api_result;
		if(rest_api_result.length() > 0)
		{
			std::unique_ptr<base::Value> root = base::JSONReader::Read(rest_api_result);
    		if(root)
    		{
    			base::DictionaryValue* root_dict = nullptr;
    			if(root->GetAsDictionary(&root_dict))
    			{
					std::string ret_api_type;
					root_dict->GetString("rsp.value.rlt.type", &ret_api_type);
					if(!ret_api_type.compare("ReportCallResult.Success"))
					{
						out_map[MSG_JSON_KEY_OPER_RESULT] = ERROR_NONE;
						return out_map;
					}
					else if(!ret_api_type.compare("ReportCallResult.CallNotFound"))
					{
						out_map[MSG_JSON_KEY_OPER_RESULT] = ERROR_RESTAPI_LOG_CALLNOTFOUND;
						return out_map;
					}
					else if(!ret_api_type.compare("ReportCallResult.UploadDbError"))
					{
						out_map[MSG_JSON_KEY_OPER_RESULT] = ERROR_RESTAPI_LOG_UPLOADDBERROR;
						return out_map;
					}
					else if(!ret_api_type.compare("ReportCallResult.InvalidFormat"))
					{
						out_map[MSG_JSON_KEY_OPER_RESULT] = ERROR_RESTAPI_LOG_INVALIDFORMAT;
						return out_map;
					}
					else if(!ret_api_type.compare("ReportCallResult.Denied"))
					{
						out_map[MSG_JSON_KEY_OPER_RESULT] = ERROR_RESTAPI_LOG_DENIED;
						return out_map;
					}
					else
					{
						out_map[MSG_JSON_KEY_OPER_RESULT] = ERROR_RESTAPI_OTHER;
						return out_map;
					}
    			}
				else
				{
					out_map[MSG_JSON_KEY_OPER_RESULT] = ERROR_HTTP_READ_JSON;
					return out_map;
				}
    		}
			else
			{
				out_map[MSG_JSON_KEY_OPER_RESULT] = ERROR_HTTP_READ_JSON;
				return out_map;
			}
		}
		else
		{
			out_map[MSG_JSON_KEY_OPER_RESULT] = ERROR_HTTP_RETRY_TIMEOUT;
			return out_map;
		}
	}
	else
	{
		out_map[MSG_JSON_KEY_OPER_RESULT] = ERROR_HTTP_PROFILE;
		return out_map;
	}

}

std::string WtkSyncHttpAPI::SendHTTPRequest(std::string baseurl,std::string path,std::string param,int trytimes,int tryinterval,std::string capath)
{
	std::string recData;
	int ret = -1;
	
	while (trytimes > 0)
	{
		DLOG(INFO) << __FUNCTION__ << ", trytimes = " << trytimes << "......";
		recData = HttpPostSync(baseurl, path, HTTP_TIMEOUT, param,capath, ret);
		DLOG(INFO) << __FUNCTION__ << ", and this time ret = " << ret;

		if (recData.length() > 0 && ret != CURLE_OPERATION_TIMEDOUT && ret != -1)
		{
			break;
		}

		if (trytimes > 1 )
		{
			webrtc::SleepMs(tryinterval*1000);
		}
		trytimes--;
	}

	return recData;
}
std::string WtkSyncHttpAPI::HttpPostSync(std::string baseurl,std::string path,int timeout,std::string param,std::string capath, int &retcode)
{
	RestClient::init();
	RestClient::Connection* conn = new RestClient::Connection(baseurl);
	conn->SetTimeout(timeout);
	conn->SetUserAgent("app/wtk-sdk-client");
	//https
	if(capath.length() > 0)
	{
		conn->SetCAInfoFilePath(capath);
		//conn->SetCertPath(REST_API_CAPATH);
		//conn->SetCertType(REST_API_CATYPE);
		//conn->SetKeyPath(REST_API_KEYPATH);
		//conn->SetKeyPassword(REST_API_KEYPASS);
	}
	conn->AppendHeader("Content-Type", "application/json");
	RestClient::Response ret = conn->post(path, param);
	RestClient::disable();
	
	retcode = ret.code;

	return ret.body;
}

std::string WtkSyncHttpAPI::HttpGetSync(std::string baseurl,std::string path,int timeout,std::string param)
{
	return "";
}
