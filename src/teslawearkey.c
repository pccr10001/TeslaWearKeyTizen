#include "teslawearkey.h"
#include "uECC.h"

#include "yaca_crypto.h"
#include "yaca_error.h"

#include "yaca_key.h"
#include "yaca_sign.h"
#include "yaca_simple.h"
#include "yaca_encrypt.h"
#include "yaca_seal.h"

#include "ckmc/ckmc-manager.h"

#include "keymanager.h"

typedef struct appdata {
	Evas_Object *win;
	Evas_Object *conform;
	Evas_Object *label;
} appdata_s;

static void win_delete_request_cb(void *data, Evas_Object *obj,
		void *event_info) {
	ui_app_exit();
}

static void win_back_cb(void *data, Evas_Object *obj, void *event_info) {
	appdata_s *ad = data;
	/* Let window go to hide state. */
	elm_win_lower(ad->win);
}

static void create_base_gui(appdata_s *ad) {
	/* Window */
	/* Create and initialize elm_win.
	 elm_win is mandatory to manipulate window. */
	ad->win = elm_win_util_standard_add(PACKAGE, PACKAGE);
	elm_win_autodel_set(ad->win, EINA_TRUE);

	if (elm_win_wm_rotation_supported_get(ad->win)) {
		int rots[4] = { 0, 90, 180, 270 };
		elm_win_wm_rotation_available_rotations_set(ad->win,
				(const int *) (&rots), 4);
	}

	evas_object_smart_callback_add(ad->win, "delete,request",
			win_delete_request_cb, NULL);
	eext_object_event_callback_add(ad->win, EEXT_CALLBACK_BACK, win_back_cb,
			ad);

	/* Conformant */
	/* Create and initialize elm_conformant.
	 elm_conformant is mandatory for base gui to have proper size
	 when indicator or virtual keypad is visible. */
	ad->conform = elm_conformant_add(ad->win);
	elm_win_indicator_mode_set(ad->win, ELM_WIN_INDICATOR_SHOW);
	elm_win_indicator_opacity_set(ad->win, ELM_WIN_INDICATOR_OPAQUE);
	evas_object_size_hint_weight_set(ad->conform, EVAS_HINT_EXPAND,
	EVAS_HINT_EXPAND);
	elm_win_resize_object_add(ad->win, ad->conform);
	evas_object_show(ad->conform);

	/* Label */
	/* Create an actual view of the base gui.
	 Modify this part to change the view. */
	ad->label = elm_label_add(ad->conform);
	elm_object_text_set(ad->label, "<align=center>Hello Tizen</align>");
	evas_object_size_hint_weight_set(ad->label, EVAS_HINT_EXPAND,
	EVAS_HINT_EXPAND);
	elm_object_content_set(ad->conform, ad->label);

	/* Show window after base gui is set up */
	evas_object_show(ad->win);
}

void print_hex(uint8_t * data, int len) {
	const char * hex = "0123456789ABCDEF";
	char out[len * 2 + 1];
	for (int i = 0; i < len; i++) {
		out[i * 2] = hex[(data[i] >> 4) & 0xF];
		out[i * 2 + 1] = hex[data[i] & 0xF];
	}
	out[len * 2] = '\0';
	dlog_print(DLOG_INFO, LOG_TAG, "%s", out);
}


static void hce_event_cb(nfc_se_h handle, nfc_hce_event_type_e event,
		unsigned char *apdu, unsigned int apdu_len, void *user_data) {
	switch (event) {
	case NFC_HCE_EVENT_DEACTIVATED:
		dlog_print(DLOG_INFO, LOG_TAG,
				"received NFC_HCE_EVENT_DEACTIVATED event on NFC handle %d",
				handle);
		break;

	case NFC_HCE_EVENT_ACTIVATED:
		dlog_print(DLOG_INFO, LOG_TAG,
				"received NFC_HCE_EVENT_ACTIVATED event on NFC handle %d",
				handle);

		break;

	case NFC_HCE_EVENT_APDU_RECEIVED:
		dlog_print(DLOG_INFO, LOG_TAG,
				"received NFC_HCE_EVENT_APDU_RECEIVED event on NFC handle %d",
				handle);

		break;

	default:
		dlog_print(DLOG_INFO, LOG_TAG,
				"received unknown event on NFC handle %d", handle);
		break;
	}
}

static bool initialize_nfc() {
	int ret = NFC_ERROR_NONE;
	bool r = false;
	nfc_se_card_emulation_mode_type_e ce_type;

	ret = nfc_manager_initialize();
	if (ret != NFC_ERROR_NONE) {
		dlog_print(DLOG_ERROR, LOG_TAG, "nfc_manager_initialize failed : %d",
				ret);
		return false;
	}

	ret = nfc_se_get_card_emulation_mode(&ce_type);
	if (ret != NFC_ERROR_NONE) {
		dlog_print(DLOG_ERROR, LOG_TAG,
				"nfc_se_get_card_emulation_mode failed : %d", ret);
		return false;
	}

	if (ce_type == NFC_SE_CARD_EMULATION_MODE_OFF) {
		nfc_se_enable_card_emulation();
		if (ret != NFC_ERROR_NONE) {
			dlog_print(DLOG_ERROR, LOG_TAG,
					"nfc_se_enable_card_emulation failed : %d", ret);
			return false;
		}
	}

	ret = nfc_manager_set_hce_event_cb(hce_event_cb, NULL);
	if (ret != NFC_ERROR_NONE) {
		dlog_print(DLOG_ERROR, LOG_TAG,
				"nfc_manager_set_hce_event_cb failed : %d", ret);
		return false;
	}
	dlog_print(DLOG_INFO, LOG_TAG, "nfc_manager_set_hce_event_cb succeeded");

	return true;

}

int rng(uint8_t *dest, unsigned size) {
	for (int i = 0; i < size; i++) {
		dest[i] = rand() & 0xFF;
	}
	return 1;
}

static bool initialize_crypto() {
	uECC_set_rng(&rng);
}



static void generate_keypair() {
	uint8_t priv[32];
	uint8_t pub[65];

	uECC_make_key(pub, priv, uECC_secp256r1());
	save_key((char*) priv, 32);

}

static bool app_create(void *data) {
	/* Hook to take necessary actions before main event loop starts
	 Initialize UI resources and application's data
	 If this function returns true, the main loop of application starts
	 If this function returns false, the application is terminated */
	appdata_s *ad = data;

	create_base_gui(ad);

	initialize_nfc();
	initialize_crypto();
	if (!get_key(NULL)) {
		generate_keypair();
		dlog_print(DLOG_INFO, LOG_TAG, "Private key generated.");
	}

	return true;
}

static void app_control(app_control_h app_control, void *data) {
	/* Handle the launch request. */
}

static void app_pause(void *data) {
	/* Take necessary actions when application becomes invisible. */
}

static void app_resume(void *data) {
	/* Take necessary actions when application becomes visible. */
}

static void app_terminate(void *data) {
	/* Release all resources. */
}

static void ui_app_lang_changed(app_event_info_h event_info, void *user_data) {
	/*APP_EVENT_LANGUAGE_CHANGED*/
	char *locale = NULL;
	system_settings_get_value_string(SYSTEM_SETTINGS_KEY_LOCALE_LANGUAGE,
			&locale);
	elm_language_set(locale);
	free(locale);
	return;
}

static void ui_app_orient_changed(app_event_info_h event_info, void *user_data) {
	/*APP_EVENT_DEVICE_ORIENTATION_CHANGED*/
	return;
}

static void ui_app_region_changed(app_event_info_h event_info, void *user_data) {
	/*APP_EVENT_REGION_FORMAT_CHANGED*/
}

static void ui_app_low_battery(app_event_info_h event_info, void *user_data) {
	/*APP_EVENT_LOW_BATTERY*/
}

static void ui_app_low_memory(app_event_info_h event_info, void *user_data) {
	/*APP_EVENT_LOW_MEMORY*/
}

int main(int argc, char *argv[]) {
	appdata_s ad = { 0, };
	int ret = 0;

	ui_app_lifecycle_callback_s event_callback = { 0, };
	app_event_handler_h handlers[5] = { NULL, };

	event_callback.create = app_create;
	event_callback.terminate = app_terminate;
	event_callback.pause = app_pause;
	event_callback.resume = app_resume;
	event_callback.app_control = app_control;

	ui_app_add_event_handler(&handlers[APP_EVENT_LOW_BATTERY],
			APP_EVENT_LOW_BATTERY, ui_app_low_battery, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_LOW_MEMORY],
			APP_EVENT_LOW_MEMORY, ui_app_low_memory, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_DEVICE_ORIENTATION_CHANGED],
			APP_EVENT_DEVICE_ORIENTATION_CHANGED, ui_app_orient_changed, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_LANGUAGE_CHANGED],
			APP_EVENT_LANGUAGE_CHANGED, ui_app_lang_changed, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_REGION_FORMAT_CHANGED],
			APP_EVENT_REGION_FORMAT_CHANGED, ui_app_region_changed, &ad);

	ret = ui_app_main(argc, argv, &event_callback, &ad);
	if (ret != APP_ERROR_NONE) {
		dlog_print(DLOG_ERROR, LOG_TAG, "app_main() is failed. err = %d", ret);
	}

	return ret;
}
