#include "helloaccessory.h"
#include "main_app.h"
#include <sap.h>
#include <sensor.h>
#include <string.h>
#include <sys/time.h>
#include <math.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

Evas_Object *GLOBAL_DEBUG_BOX;
Evas_Object *start, *stop;
Evas_Object *conform;
Evas_Object *event_label;
sensor_listener_h listener_hrm, listener_accel;		// listeners for sensors

extern int hrm_data;
extern float svm_accel[SVMLEN];
extern double time_in_mill;
extern void on_data_received(sap_socket_h socket, unsigned short int channel_id);

int idx = 0;

float getSVM(float x, float y, float z){
	float square_x = powf(x, 2);
	float square_y = powf(y, 2);
	float square_z = powf(z, 2);

	return sqrtf(square_x + square_y + square_z);
}

void on_sensor_event(sensor_h sensor, sensor_event_s *event, void *user_data)
{
	sensor_type_e type;
    sensor_get_type(sensor, &type);

    struct timeval timeVal;		// to estimate current timestamp

    switch (type) {
    case SENSOR_HRM:
    		hrm_data = (int) event->values[0];
    		char a[100];
    		sprintf(a,"Heart BPM : %.0f", event->values[0]);
    		elm_object_text_set(event_label, a);
    		break;

    case SENSOR_ACCELEROMETER:
			if(idx < SVMLEN) {
				if(idx == 0){
					// convert tv_sec & tv_usec to millisecond
					gettimeofday(&timeVal, NULL);
					time_in_mill = (timeVal.tv_sec)*1000 + (timeVal.tv_usec)/1000;
				}
				svm_accel[idx] = getSVM(event->values[0], event->values[1], event->values[2]);
				idx++;
			} else {
				on_data_received(socket, HELLO_ACC_CHANNELID);
				idx = 0;
			}
			break;
    default:
    		dlog_print(DLOG_ERROR, LOG_TAG, "Not an Sensor event");
    }
}

void _sensor_accuracy_changed_cb(sensor_h sensor, unsigned long long timestamp,
                                 sensor_data_accuracy_e accuracy, void *data)
{
    dlog_print(DLOG_DEBUG, LOG_TAG, "Sensor accuracy change callback invoked");
}

void _sensor_start_cb(void *data, Evas_Object *obj, void *event_info)
{
    void *user_data = NULL;
    char out[100];

    char *name = NULL;
    char *vendor = NULL;
    float resolution = 0.0;

    float min_hrm = 0.0;
    float max_hrm = 220.0;

    float min_accel = -19.6;
    float max_accel = 19.6;

    // Retrieving a Sensor
    sensor_type_e type_hrm = SENSOR_HRM;
    sensor_type_e type_accel = SENSOR_ACCELEROMETER;
    sensor_h sensor_hrm, sensor_accel;

	bool supported;

    int error = sensor_is_supported(type_hrm, &supported);
    if (error != SENSOR_ERROR_NONE) {
        dlog_print(DLOG_ERROR, LOG_TAG, "sensor_is_supported error: %d", error);
        return;
    }
    if(supported) {
       dlog_print(DLOG_DEBUG, LOG_TAG, "HRM is%s supported", supported ? "" : " not");
       sprintf(out,"HRM is%s supported", supported ? "" : " not");
       elm_object_text_set(event_label, out);
    }

    error = sensor_is_supported(type_accel, &supported);
	if (error != SENSOR_ERROR_NONE) {
		dlog_print(DLOG_ERROR, LOG_TAG, "sensor_is_supported error: %d", error);
		return;
	}
    if(supported) {
       dlog_print(DLOG_DEBUG, LOG_TAG, "ACCELEROMETER is%s supported", supported ? "" : " not");
       sprintf(out,"ACCELEROMETER is%s supported", supported ? "" : " not");
       elm_object_text_set(event_label, out);
    }



    // Get sensor list
    int count;
    sensor_h *list;

    error = sensor_get_sensor_list(type_hrm, &list, &count); 						// hr
    if (error != SENSOR_ERROR_NONE) {
        dlog_print(DLOG_ERROR, LOG_TAG, "sensor_get_sensor_list error: %d", error);
    } else {
        dlog_print(DLOG_DEBUG, LOG_TAG, "Number of sensors: %d", count);
        free(list);
    }

    error = sensor_get_sensor_list(type_accel, &list, &count); 						// accel
    if (error != SENSOR_ERROR_NONE) {
        dlog_print(DLOG_ERROR, LOG_TAG, "sensor_get_sensor_list error: %d", error);
    } else {
        dlog_print(DLOG_DEBUG, LOG_TAG, "Number of sensors: %d", count);
        free(list);
    }



    // Get default sensor
    error = sensor_get_default_sensor(type_hrm, &sensor_hrm);						// hr
    if (error != SENSOR_ERROR_NONE) {
        dlog_print(DLOG_ERROR, LOG_TAG, "sensor_get_default_sensor error: %d", error);
        return;
    }
    dlog_print(DLOG_DEBUG, LOG_TAG, "sensor_get_default_sensor");

    error = sensor_get_default_sensor(type_accel, &sensor_accel);					// accel
    if (error != SENSOR_ERROR_NONE) {
        dlog_print(DLOG_ERROR, LOG_TAG, "sensor_get_default_sensor error: %d", error);
        return;
    }
    dlog_print(DLOG_DEBUG, LOG_TAG, "sensor_get_default_sensor");



    // Registering a Sensor Event
    error = sensor_create_listener(sensor_hrm, &listener_hrm);						// hr
    if (error != SENSOR_ERROR_NONE) {
        dlog_print(DLOG_ERROR, LOG_TAG, "sensor_create_listener error: %d", error);
        return;
    }
    dlog_print(DLOG_DEBUG, LOG_TAG, "sensor_create_listener");

    error = sensor_create_listener(sensor_accel, &listener_accel);					// accel
    if (error != SENSOR_ERROR_NONE) {
        dlog_print(DLOG_ERROR, LOG_TAG, "sensor_create_listener error: %d", error);
        return;
    }
    dlog_print(DLOG_DEBUG, LOG_TAG, "sensor_create_listener");



    // Setting min_interval
    int min_interval = 0;															// hr
    error = sensor_get_min_interval(sensor_hrm, &min_interval);
    if (error != SENSOR_ERROR_NONE) {
        dlog_print(DLOG_ERROR, LOG_TAG, "sensor_get_min_interval error: %d", error);
        return;
    }
    dlog_print(DLOG_DEBUG, LOG_TAG, "Minimum interval of the sensor: %d", min_interval);

    min_interval = 0;																// accel
    error = sensor_get_min_interval(sensor_accel, &min_interval);
    if (error != SENSOR_ERROR_NONE) {
        dlog_print(DLOG_ERROR, LOG_TAG, "sensor_get_min_interval error: %d", error);
        return;
    }
    dlog_print(DLOG_DEBUG, LOG_TAG, "Minimum interval of the sensor: %d", min_interval);



    // Callback for sensor value change
    error = sensor_listener_set_event_cb(listener_hrm, min_interval, on_sensor_event, user_data);		// hr
    if (error != SENSOR_ERROR_NONE) {
        dlog_print(DLOG_ERROR, LOG_TAG, "sensor_listener_set_event_cb error: %d", error);
        return;
    }
    dlog_print(DLOG_DEBUG, LOG_TAG, "sensor_listener_set_event_cb");

    error = sensor_listener_set_event_cb(listener_accel, min_interval, on_sensor_event, user_data);		// accel
    if (error != SENSOR_ERROR_NONE) {
        dlog_print(DLOG_ERROR, LOG_TAG, "sensor_listener_set_event_cb error: %d", error);
        return;
    }
    dlog_print(DLOG_DEBUG, LOG_TAG, "sensor_listener_set_event_cb");



    // Registering the Accuracy Changed Callback
    error = sensor_listener_set_accuracy_cb(listener_hrm, _sensor_accuracy_changed_cb, user_data);		// hr
    if (error != SENSOR_ERROR_NONE) {
        dlog_print(DLOG_ERROR, LOG_TAG, "sensor_listener_set_accuracy_cb error: %d", error);
        return;
    }
    dlog_print(DLOG_DEBUG, LOG_TAG, "sensor_listener_set_accuracy_cb");


    error = sensor_listener_set_accuracy_cb(listener_accel, _sensor_accuracy_changed_cb, user_data);	// accel
    if (error != SENSOR_ERROR_NONE) {
        dlog_print(DLOG_ERROR, LOG_TAG, "sensor_listener_set_accuracy_cb error: %d", error);
        return;
    }
    dlog_print(DLOG_DEBUG, LOG_TAG, "sensor_listener_set_accuracy_cb");



    // Setting sensor interval
    error = sensor_listener_set_interval(listener_hrm, 5000);
    if (error != SENSOR_ERROR_NONE) {
        dlog_print(DLOG_ERROR, LOG_TAG, "sensor_listener_set_interval error: %d", error);
        return;
    }
    dlog_print(DLOG_DEBUG, LOG_TAG, "sensor_listener_set_intervals");

    error = sensor_listener_set_interval(listener_accel, 50);
    if (error != SENSOR_ERROR_NONE) {
        dlog_print(DLOG_ERROR, LOG_TAG, "sensor_listener_set_interval error: %d", error);
        return;
    }
    dlog_print(DLOG_DEBUG, LOG_TAG, "sensor_listener_set_intervals");



    // Setting sensor listener option - always on, power-save etc.
    error = sensor_listener_set_option(listener_hrm, SENSOR_OPTION_ALWAYS_ON);
    if (error != SENSOR_ERROR_NONE) {
        dlog_print(DLOG_ERROR, LOG_TAG, "sensor_listener_set_option error: %d", error);
        return;
    }
    dlog_print(DLOG_DEBUG, LOG_TAG, "sensor_listener_set_option");

    error = sensor_listener_set_option(listener_accel, SENSOR_OPTION_ALWAYS_ON);
    if (error != SENSOR_ERROR_NONE) {
        dlog_print(DLOG_ERROR, LOG_TAG, "sensor_listener_set_option error: %d", error);
        return;
    }
    dlog_print(DLOG_DEBUG, LOG_TAG, "sensor_listener_set_option");



    // Start sensors
    error = sensor_listener_start(listener_hrm);
    if (error != SENSOR_ERROR_NONE) {
        dlog_print(DLOG_ERROR, LOG_TAG, "sensor_listener_start error: %d", error);
        return;
    }
    dlog_print(DLOG_DEBUG, LOG_TAG, "sensor_listener_start");

    error = sensor_listener_start(listener_accel);
    if (error != SENSOR_ERROR_NONE) {
        dlog_print(DLOG_ERROR, LOG_TAG, "sensor_listener_start error: %d", error);
        return;
    }
    dlog_print(DLOG_DEBUG, LOG_TAG, "sensor_listener_start");



    // Read data from listners
    sensor_event_s event_hrm, event_accel;
    error = sensor_listener_read_data(listener_hrm, &event_hrm);
    if (error != SENSOR_ERROR_NONE) {
        dlog_print(DLOG_ERROR, LOG_TAG, "sensor_listener_read_data error: %d", error);
        return;
    }

    error = sensor_listener_read_data(listener_accel, &event_accel);
    if (error != SENSOR_ERROR_NONE) {
        dlog_print(DLOG_ERROR, LOG_TAG, "sensor_listener_read_data error: %d", error);
        return;
    }



    // Get sensor name
    error = sensor_get_name(sensor_hrm, &name);									// hr
    if (error != SENSOR_ERROR_NONE) {
        dlog_print(DLOG_ERROR, LOG_TAG, "sensor_get_name error: %d", error);
        return;
    }
    dlog_print(DLOG_DEBUG, LOG_TAG, "Sensor name: %s", name);

    error = sensor_get_name(sensor_accel, &name); 								// accel
    if (error != SENSOR_ERROR_NONE) {
        dlog_print(DLOG_ERROR, LOG_TAG, "sensor_get_name error: %d", error);
        return;
    }
    dlog_print(DLOG_DEBUG, LOG_TAG, "Sensor name: %s", name);

    free(name);



    // Get sensor vendor
    error = sensor_get_vendor(sensor_hrm, &vendor);								// hr
    if (error != SENSOR_ERROR_NONE) {
        dlog_print(DLOG_ERROR, LOG_TAG, "sensor_get_vendor error: %d", error);
        return;
    }
    dlog_print(DLOG_DEBUG, LOG_TAG, "Sensor vendor: %s", vendor);

    error = sensor_get_vendor(sensor_accel, &vendor); 							// accel
    if (error != SENSOR_ERROR_NONE) {
        dlog_print(DLOG_ERROR, LOG_TAG, "sensor_get_vendor error: %d", error);
        return;
    }
    dlog_print(DLOG_DEBUG, LOG_TAG, "Sensor vendor: %s", vendor);

    free(vendor);



    // Get sensor type
    error = sensor_get_type(sensor_hrm, &type_hrm);								// hr
    if (error != SENSOR_ERROR_NONE) {
        dlog_print(DLOG_ERROR, LOG_TAG, "sensor_get_type error: %d", error);
        return;
    }

    error = sensor_get_type(sensor_accel, &type_accel);							// accel
    if (error != SENSOR_ERROR_NONE) {
        dlog_print(DLOG_ERROR, LOG_TAG, "sensor_get_type error: %d", error);
        return;
    }



    // Get sensor range
    error = sensor_get_min_range(sensor_hrm, &min_hrm);								// hr (min&max)
    if (error != SENSOR_ERROR_NONE) {
        dlog_print(DLOG_ERROR, LOG_TAG, "sensor_get_min_range error: %d", error);
        return;
    }
    dlog_print(DLOG_DEBUG, LOG_TAG, "Minimum range of the sensor: %f", min_hrm);

    error = sensor_get_max_range(sensor_hrm, &max_hrm);
    if (error != SENSOR_ERROR_NONE) {
        dlog_print(DLOG_ERROR, LOG_TAG, "sensor_get_max_range error: %d", error);
        return;
    }
    dlog_print(DLOG_DEBUG, LOG_TAG, "Maximum range of the sensor: %f", max_hrm);


    error = sensor_get_min_range(sensor_accel, &min_accel);							// accel (min&max)
    if (error != SENSOR_ERROR_NONE) {
        dlog_print(DLOG_ERROR, LOG_TAG, "sensor_get_min_range error: %d", error);
        return;
    }
    dlog_print(DLOG_DEBUG, LOG_TAG, "Minimum range of the sensor: %f", min_accel);

    error = sensor_get_max_range(sensor_accel, &max_accel);
    if (error != SENSOR_ERROR_NONE) {
        dlog_print(DLOG_ERROR, LOG_TAG, "sensor_get_max_range error: %d", error);
        return;
    }
    dlog_print(DLOG_DEBUG, LOG_TAG, "Maximum range of the sensor: %f", max_accel);



    // Get resolution
    error = sensor_get_resolution(sensor_hrm, &resolution);							// hr
    if (error != SENSOR_ERROR_NONE) {
        dlog_print(DLOG_ERROR, LOG_TAG, "sensor_get_resolution error: %d", error);
        return;
    }
    dlog_print(DLOG_DEBUG, LOG_TAG, "Resolution of the sensor: %f", resolution);

    error = sensor_get_resolution(sensor_accel, &resolution);						// accel
    if (error != SENSOR_ERROR_NONE) {
        dlog_print(DLOG_ERROR, LOG_TAG, "sensor_get_resolution error: %d", error);
        return;
    }
    dlog_print(DLOG_DEBUG, LOG_TAG, "Resolution of the sensor: %f", resolution);

    elm_object_disabled_set(start, EINA_TRUE);
    elm_object_disabled_set(stop, EINA_FALSE);
}

void _sensor_stop_cb(void *data, Evas_Object *obj, void *event_info)
{
	// HR Stop
    int error = sensor_listener_unset_event_cb(listener_hrm);
    if (error != SENSOR_ERROR_NONE) {
        dlog_print(DLOG_ERROR, LOG_TAG, "sensor_listener_unset_event_cb error: %d", error);
    }

    error = sensor_listener_stop(listener_hrm);
    if (error != SENSOR_ERROR_NONE) {
        dlog_print(DLOG_ERROR, LOG_TAG, "sensor_listener_stop error: %d", error);
    }

    error = sensor_destroy_listener(listener_hrm);
    if (error != SENSOR_ERROR_NONE) {
        dlog_print(DLOG_ERROR, LOG_TAG, "sensor_destroy_listener error: %d", error);
    }


    // ACCEL STOP
    error = sensor_listener_unset_event_cb(listener_accel);
    if (error != SENSOR_ERROR_NONE) {
        dlog_print(DLOG_ERROR, LOG_TAG, "sensor_listener_unset_event_cb error: %d", error);
    }

    error = sensor_listener_stop(listener_accel);
    if (error != SENSOR_ERROR_NONE) {
        dlog_print(DLOG_ERROR, LOG_TAG, "sensor_listener_stop error: %d", error);
    }

    error = sensor_destroy_listener(listener_accel);
    if (error != SENSOR_ERROR_NONE) {
        dlog_print(DLOG_ERROR, LOG_TAG, "sensor_destroy_listener error: %d", error);
    }

    elm_object_disabled_set(start, EINA_FALSE);
    elm_object_disabled_set(stop, EINA_TRUE);
}

void _add_entry_text(const char *text)
{
    Evas_Coord c_y;

    elm_entry_entry_append(GLOBAL_DEBUG_BOX, text);
    elm_entry_entry_append(GLOBAL_DEBUG_BOX, "<br>");
    elm_entry_cursor_end_set(GLOBAL_DEBUG_BOX);
    elm_entry_cursor_geometry_get(GLOBAL_DEBUG_BOX, NULL, &c_y, NULL, NULL);
    elm_scroller_region_show(GLOBAL_DEBUG_BOX, 0, c_y, 0, 0);
}

static void win_delete_request_cb(void *data, Evas_Object *obj, void *event_info)
{
    elm_exit();
}

Eina_Bool _pop_cb(void *data, Elm_Object_Item *item)
{
    elm_win_lower(((appdata_s *)data)->win);
    return EINA_FALSE;
}

Evas_Object *_new_button(appdata_s *ad, Evas_Object *display, char *name, void *cb)
{
    // Create a button
    Evas_Object *bt = elm_button_add(display);
    elm_object_text_set(bt, name);
    evas_object_smart_callback_add(bt, "clicked", (Evas_Smart_Cb) cb, ad);
    evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
    evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
    elm_box_pack_end(display, bt);
    evas_object_show(bt);
    return bt;
}


void _create_new_cd_display(appdata_s *ad, char *name, void *cb)
{
    // Create main box
    Evas_Object *box = elm_box_add(conform);
    elm_object_content_set(conform, box);
    elm_box_horizontal_set(box, EINA_FALSE);
    evas_object_size_hint_align_set(box, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_show(box);

    start = _new_button(ad, box, "Start", _sensor_start_cb);

    event_label = elm_label_add(box);
    elm_object_text_set(event_label, "Tizen Sensors App");
    elm_box_pack_end(box, event_label);
    evas_object_show(event_label);

    stop = _new_button(ad, box, "Stop", _sensor_stop_cb);
}

static void create_base_gui(appdata_s *ad)
{
    // Setting the window
    ad->win = elm_win_util_standard_add(PACKAGE, PACKAGE);
    elm_win_conformant_set(ad->win, EINA_TRUE);
    elm_win_autodel_set(ad->win, EINA_TRUE);
    elm_win_indicator_mode_set(ad->win, ELM_WIN_INDICATOR_SHOW);
    elm_win_indicator_opacity_set(ad->win, ELM_WIN_INDICATOR_OPAQUE);
    evas_object_smart_callback_add(ad->win, "delete, request", win_delete_request_cb, NULL);

    /* Create conformant */
    conform = elm_conformant_add(ad->win);

    evas_object_size_hint_weight_set(conform, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    elm_win_resize_object_add(ad->win, conform);
    evas_object_show(conform);

    // Create a naviframe
    ad->navi = elm_naviframe_add(conform);
    evas_object_size_hint_align_set(ad->navi, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_size_hint_weight_set(ad->navi, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

    elm_object_content_set(conform, ad->navi);
    evas_object_show(ad->navi);

    // Fill the list with items
    //create_buttons_in_main_window(ad);
    _create_new_cd_display(ad, "Sensor", _pop_cb);

    eext_object_event_callback_add(ad->navi, EEXT_CALLBACK_BACK, eext_naviframe_back_cb, NULL);

    // Show the window after base gui is set up
    evas_object_show(ad->win);
}

static bool app_create(void *data)
{
    /*
     * Hook to take necessary actions before main event loop starts
     * Initialize UI resources and application's data
     * If this function returns true, the main loop of application starts
     * If this function returns false, the application is terminated
     */
    create_base_gui((appdata_s *)data);
    initialize_sap();
    elm_object_text_set(event_label, "Tizen Sensors App");

    return true;
}

int main(int argc, char *argv[])
{
    appdata_s ad;
    memset(&ad, 0x00, sizeof(appdata_s));

    ui_app_lifecycle_callback_s event_callback;
    memset(&event_callback, 0x00, sizeof(ui_app_lifecycle_callback_s));

    event_callback.create = app_create;

    int ret = ui_app_main(argc, argv, &event_callback, &ad);
    if (ret != APP_ERROR_NONE)
        dlog_print(DLOG_ERROR, LOG_TAG, "ui_app_main() failed with error: %d", ret);

    return ret;
}
