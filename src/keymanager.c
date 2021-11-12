#include "keymanager.h"
#include "ckmc/ckmc-manager.h"

 bool save_key(char *data, int len) {
	int ret = CKMC_ERROR_NONE;

	ckmc_key_s key;
	ckmc_policy_s store_policy;
	char* alias = "tesla_nak";

	key.raw_key = (unsigned char *) data;
	key.key_size = len;
	key.key_type = CKMC_KEY_AES;    // Only AES type can be saved.
	key.password = NULL;
	store_policy.password = NULL;
	store_policy.extractable = true;

	ret = ckmc_save_key(alias, key, store_policy);
	if (CKMC_ERROR_NONE != ret) {
		dlog_print(DLOG_ERROR, LOG_TAG, "ckmc_save_key failed :%d", ret);
		return false;
	}
	return true;
}

 bool get_key(char* data) {
	int ret = CKMC_ERROR_NONE;

	ckmc_key_s *key;
	char* alias = "tesla_nak";
	char* key_password = NULL;

	ret = ckmc_get_key(alias, key_password, &key);
	if (CKMC_ERROR_NONE != ret) {
		return false;
	}

	if (data == NULL) {
		goto exit;
	}

	for (int i = 0; i < key->key_size; i++) {
		data[i] = key->raw_key[i];
	}

	exit:
	ckmc_key_free(key);
	return true;
}
