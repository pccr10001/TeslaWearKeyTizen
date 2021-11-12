#include "tesla_nak.h"
#include "uECC.h"
#include "keymanager.h"


void process(uint8_t *req, int req_len, uint8_t *resp, int* resp_len) {

	// check selecting applet
	if (is_selecting(req, req_len)) {
		*resp_len = 2;
		resp[0] = 0x90;
		resp[1] = 0x00;
		return;
	}

	//check CLA
	if ((req[0] & 0x80) != 0x80) {
		*resp_len = 2;
		resp[0] = 0x6E;
		resp[1] = 0x00;
		return;
	}

	switch (req[1]) {
	case INS_GET_CARD_INFO:
		process_get_info(req, req_len, resp, resp_len);
		break;
	case INS_GET_PUBLIC_KEY:
		process_get_public_key(req, req_len, resp, resp_len);
		break;
	case INS_AUTHENTICATE:
		process_authenticate(req, req_len, resp, resp_len);
		break;
	}

}

bool is_selecting(uint8_t *req, int req_len) {
	if (req_len < 14) {
		return false;
	}
	if (req[0] == 0x00 && req[1] == 0xa4 && req[2] == 0x04 && req[3] == 0x00) {
		return true;
	}
	return false;
}

bool process_get_info(uint8_t *req, int req_len, uint8_t *resp, int* resp_len) {
	*resp_len = 4;
	resp[0]=0x00;
	resp[1]=0x01;
	resp[2]=0x90;
	resp[3]=0x00;
	return true;
}
bool process_get_public_key(uint8_t *req, int req_len, uint8_t *resp,
		int* resp_len) {



}
bool process_authenticate(uint8_t *req, int req_len, uint8_t *resp,
		int* resp_len) {
}

