#ifndef _GENERAL_DEF_H
#define _GENERAL_DEF_H
typedef enum{
	general_unknown_cmd = 0,

	//--------------------------Global command define(101--130)--------------------------------
	glb_update_cagent_req = 111,
	glb_update_cagent_rep,
	glb_cagent_rename_req = 113,
	glb_cagent_rename_rep,

	glb_get_init_info_rep = 116,
	
	glb_update_cagent_stop_req = 119,
	glb_update_cagent_stop_rep = 120,
	glb_update_cagent_retry_req = 121,
	glb_update_cagent_retry_rep = 122,
	glb_get_handler_list_req = 123,
	glb_get_handler_list_rep = 124,
	glb_server_control_req = 125,

	glb_cagent_heartbeatratequery_req = 127,
	glb_cagent_heartbeatratequery_rep = 128,
	glb_cagent_heartbeatrateupdte_req = 129,
	glb_cagent_heartbeatrateupdte_rep = 130,

	glb_error_rep = 600,
	//------------------------WSN General command define(2051--2100)------------------------------
	general_info_spec_req = 2051,
	general_info_spec_rep = 2052,
	general_start_auto_upload_req = 2053,
	general_start_auto_upload_rep = 2054,
	general_info_upload_rep = 2055,
	general_stop_auto_upload_req = 2056,
	general_stop_auto_upload_rep = 2057,
}susi_comm_cmd_t;

#endif