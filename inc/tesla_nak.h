#ifndef _TESLA_NAK_H_
#define _TESLA_NAK_H_

#include "stdint.h"
#include "stdbool.h"

const uint8_t INS_GET_PUBLIC_KEY = 0x04;
const uint8_t INS_AUTHENTICATE = 0x11;
const uint8_t INS_GET_CARD_INFO = 0x14;

bool is_selecting(uint8_t *req, int req_len);
void process(uint8_t *req, int req_len, uint8_t *resp, int* resp_len);

bool process_get_info(uint8_t *req, int req_len, uint8_t *resp,
		int* resp_len);
bool process_get_public_key(uint8_t *req, int req_len, uint8_t *resp,
		int* resp_len);
bool process_authenticate(uint8_t *req, int req_len, uint8_t *resp,
		int* resp_len);

#endif
