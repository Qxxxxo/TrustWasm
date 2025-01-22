#include <tee_internal_api.h>
#include <tee_internal_api_extensions.h>
#include <string.h>
#include <data_sharing_ta.h>

TEE_Result TA_CreateEntryPoint(void)
{
	DMSG("has been called");

	return TEE_SUCCESS;
}


void TA_DestroyEntryPoint(void)
{
	DMSG("has been called");
}


TEE_Result TA_OpenSessionEntryPoint(uint32_t param_types,
		TEE_Param __maybe_unused params[4],
		void __maybe_unused **sess_ctx)
{
	uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE);

	DMSG("has been called");

	if (param_types != exp_param_types)
		return TEE_ERROR_BAD_PARAMETERS;

	/* Unused parameters */
	(void)&params;
	(void)&sess_ctx;

	/*
	 * The DMSG() macro is non-standard, TEE Internal API doesn't
	 * specify any means to logging from a TA.
	 */
	IMSG("Hello from data sharing ta!\n");

	/* If return value != TEE_SUCCESS the session will not be created. */
	return TEE_SUCCESS;
}

/*
 * Called when a session is closed, sess_ctx hold the value that was
 * assigned by TA_OpenSessionEntryPoint().
 */
void TA_CloseSessionEntryPoint(void __maybe_unused *sess_ctx)
{
	(void)&sess_ctx; /* Unused parameter */
	IMSG("Goodbye from data sharing ta!\n");
}

static TEE_Result get_str_data(uint32_t param_types,
	TEE_Param params[4])
{
	char * str_data="this is test data";
	uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_OUTPUT,
						   TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE);

	DMSG("has been called");

	if (param_types != exp_param_types)
		return TEE_ERROR_BAD_PARAMETERS;

	if(params[0].memref.size<strlen(str_data)+1){
		// return short buffer according to specification
		return TEE_ERROR_SHORT_BUFFER;
	}
	DMSG("copy data to %p",(void *)params[0].memref.buffer);
	memset(params[0].memref.buffer,0,strlen(str_data)+1);
	// copy data
	memcpy(params[0].memref.buffer,str_data,strlen(str_data));
	params[0].memref.size=strlen(str_data);
	
	return TEE_SUCCESS;
}

TEE_Result TA_InvokeCommandEntryPoint(void __maybe_unused *sess_ctx,
			uint32_t cmd_id,
			uint32_t param_types, TEE_Param params[4])
{
	(void)&sess_ctx; /* Unused parameter */

	switch (cmd_id) {
	case TA_DATA_SHARING_CMD_GET_STR_DATA:
		return get_str_data(param_types, params);
	default:
		return TEE_ERROR_BAD_PARAMETERS;
	}
}
