#pragma once
#include <string>
int apiUpload(std::string& post_data, std::string& str_json);

int apiUploadInit(const std::string& dfs_path_client, const std::string& storage_web_server_ip, const std::string& storage_web_server_port, const std::string& shorturl_server_address, const std::string& access_token);