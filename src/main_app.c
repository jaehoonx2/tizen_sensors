#include "helloaccessory.h"
#include "main_app.h"
#include "sap.h"						// originally <sap.h>
#include <sensor.h>
#include <string.h>
#include <sys/time.h>

#define HELLO_ACC_CHANNELID 104			// should be modified later (duplicated)

Evas_Object *GLOBAL_DEBUG_BOX;
Evas_Object *start, *stop;
Evas_Object *conform;
sensor_listener_h listener; 			// hr
sensor_listener_h listener1;			// accel
Evas_Object *event_label;

int idx = 0;

extern int hrm_data;
extern float accel_data[ACCLEN];
extern double time_in_mill;
extern void on_data_received(sap_socket_h socket, unsigned short int channel_id);

void on_sensor_event(sensor_h sensor, sensor_event_s *event, void *user_data)
{
	sensor_type_e type;
    sensor_get_type(sensor, &type);

    struct timeval timeVal;

    switch (type) {
    case SENSOR_HRM:
    		hrm_data = (int) event->values[0];
    		char a[100];
    		sprintf(a,"Heart BPM : %.0f", event->values[0]);
    		elm_object_text_set(event_label, a);
    		break;

    case SENSOR_ACCELEROMETER:
			if(idx < ACCLEN) {
				if(idx == 0){
					// convert tv_sec & tv_usec to millisecond
					gettimeofday(&timeVal, NULL);
					time_in_mill = (timeVal.tv_sec)*1000 + (timeVal.tv_usec)/1000;
				}
				accel_data[idx] = event->values[0];
				accel_data[idx+1] = event->values[1];
				accel_data[idx+2] = event->values[2];
				idx += 3;
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
    void *user_data = NULL; //hr
    void *user_data1 = NULL; //accel
    char out[100]; //hr
    char out1[100]; //accel

    // Retrieving a Sensor
    sensor_type_e type = SENSOR_HRM; //hr
    sensor_h sensor; //hr

    sensor_type_e type1 = SENSOR_ACCELEROMETER; //accel
    sensor_h sensor1; //accel

    bool supported; //hr
    int error = sensor_is_supported(type, &supported);
    if (error != SENSOR_ERROR_NONE) {
        dlog_print(DLOG_ERROR, LOG_TAG, "sensor_is_supported error: %d", error);
        return;
    }

    bool supported1; //accel
    int error1 = sensor_is_supported(type1, &supported1);
    if (error1 != SENSOR_ERROR_NONE) {
        dlog_print(DLOG_ERROR, LOG_TAG, "sensor_is_supported error: %d", error1);
        return;
    }

    if(supported) //hr
    {
       dlog_print(DLOG_DEBUG, LOG_TAG, "HRM is%s supported", supported ? "" : " not");
       sprintf(out,"HRM is%s supported", supported ? "" : " not");
       elm_object_text_set(event_label, out);
    }

    if(supported1) //accel
    {
       dlog_print(DLOG_DEBUG, LOG_TAG, "ACCELEROMETER is%s supported", supported1 ? "" : " not");
       sprintf(out1,"ACCELEROMETER is%s supported", supported1 ? "" : " not");
       elm_object_text_set(event_label, out1);
    }

    // Get sensor list
    int count; //hr
    sensor_h *list; //hr

    int count1; //accel
    sensor_h *list1; //aceel

    error = sensor_get_sensor_list(type, &list, &count); //hr
    if (error != SENSOR_ERROR_NONE) {
        dlog_print(DLOG_ERROR, LOG_TAG, "sensor_get_sensor_list error: %d", error);
    } else {
        dlog_print(DLOG_DEBUG, LOG_TAG, "Number of sensors: %d", count);
        free(list);
    }

    error = sensor_get_default_sensor(type, &sensor);
    if (error != SENSOR_ERROR_NONE) {
        dlog_print(DLOG_ERROR, LOG_TAG, "sensor_get_default_sensor error: %d", error);
        return;
    }
    dlog_print(DLOG_DEBUG, LOG_TAG, "sensor_get_default_sensor");

    error1 = sensor_get_sensor_list(type1, &list1, &count1); //accel
    if (error1 != SENSOR_ERROR_NONE) {
        dlog_print(DLOG_ERROR, LOG_TAG, "sensor_get_sensor_list error: %d", error1);
    } else {
        dlog_print(DLOG_DEBUG, LOG_TAG, "Number of sensors: %d", count1);
        free(list1);
    }

    error1 = sensor_get_default_sensor(type1, &sensor1);
    if (error1 != SENSOR_ERROR_NONE) {
        dlog_print(DLOG_ERROR, LOG_TAG, "sensor_get_default_sensor error: %d", error1);
        return;
    }
    dlog_print(DLOG_DEBUG, LOG_TAG, "sensor_get_default_sensor");

    // Registering a Sensor Event
    error = sensor_create_listener(sensor, &listener); //hr
    if (error != SENSOR_ERROR_NONE) {
        dlog_print(DLOG_ERROR, LOG_TAG, "sensor_create_listener error: %d", error);
        return;
    }
    dlog_print(DLOG_DEBUG, LOG_TAG, "sensor_create_listener");

    int min_interval = 0;
    error = sensor_get_min_interval(sensor, &min_interval);
    if (error != SENSOR_ERROR_NONE) {
        dlog_print(DLOG_ERROR, LOG_TAG, "sensor_get_min_interval error: %d", error);
        return;
    }
    dlog_print(DLOG_DEBUG, LOG_TAG, "Minimum interval of the sensor: %d", min_interval);

    error1 = sensor_create_listener(sensor1, &listener1); //accel
    if (error1 != SENSOR_ERROR_NONE) {
        dlog_print(DLOG_ERROR, LOG_TAG, "sensor_create_listener error: %d", error1);
        return;
    }
    dlog_print(DLOG_DEBUG, LOG_TAG, "sensor_create_listener");

    int min_interval1 = 0;
    error1 = sensor_get_min_interval(sensor1, &min_interval1);
    if (error1 != SENSOR_ERROR_NONE) {
        dlog_print(DLOG_ERROR, LOG_TAG, "sensor_get_min_interval error: %d", error1);
        return;
    }
    dlog_print(DLOG_DEBUG, LOG_TAG, "Minimum interval of the sensor: %d", min_interval1);

    // Callback for sensor value change
    error = sensor_listener_set_event_cb(listener, min_interval, on_sensor_event, user_data); //hr
    if (error != SENSOR_ERROR_NONE) {
        dlog_print(DLOG_ERROR, LOG_TAG, "sensor_listener_set_event_cb error: %d", error);
        return;
    }
    dlog_print(DLOG_DEBUG, LOG_TAG, "sensor_listener_set_event_cb");

    error1 = sensor_listener_set_event_cb(listener1, min_interval1, on_sensor_event, user_data1); //accel
    if (error1 != SENSOR_ERROR_NONE) {
        dlog_print(DLOG_ERROR, LOG_TAG, "sensor_listener_set_event_cb error: %d", error1);
        return;
    }
    dlog_print(DLOG_DEBUG, LOG_TAG, "sensor_listener_set_event_cb");

    // Registering the Accuracy Changed Callback
    //hr
    error = sensor_listener_set_accuracy_cb(listener, _sensor_accuracy_changed_cb, user_data);
    if (error != SENSOR_ERROR_NONE) {
        dlog_print(DLOG_ERROR, LOG_TAG, "sensor_listener_set_accuracy_cb error: %d", error);
        return;
    }
    dlog_print(DLOG_DEBUG, LOG_TAG, "sensor_listener_set_accuracy_cb");

    error = sensor_listener_set_interval(listener, 25);
    if (error != SENSOR_ERROR_NONE) {
        dlog_print(DLOG_ERROR, LOG_TAG, "sensor_listener_set_interval error: %d", error);
        return;
    }
    dlog_print(DLOG_DEBUG, LOG_TAG, "sensor_listener_set_intervals");

    error = sensor_listener_set_option(listener, SENSOR_OPTION_ALWAYS_ON);
    if (error != SENSOR_ERROR_NONE) {
        dlog_print(DLOG_ERROR, LOG_TAG, "sensor_listener_set_option error: %d", error);
        return;
    }
    dlog_print(DLOG_DEBUG, LOG_TAG, "sensor_listener_set_option");

    error = sensor_listener_start(listener);
    if (error != SENSOR_ERROR_NONE) {
        dlog_print(DLOG_ERROR, LOG_TAG, "sensor_listener_start error: %d", error);
        return;
    }
    dlog_print(DLOG_DEBUG, LOG_TAG, "sensor_listener_start");

    //accel
    error1 = sensor_listener_set_accuracy_cb(listener1, _sensor_accuracy_changed_cb, user_data1);
    if (error1 != SENSOR_ERROR_NONE) {
        dlog_print(DLOG_ERROR, LOG_TAG, "sensor_listener_set_accuracy_cb error: %d", error1);
        return;
    }
    dlog_print(DLOG_DEBUG, LOG_TAG, "sensor_listener_set_accuracy_cb");

    error1 = sensor_listener_set_interval(listener1, 25);
    if (error1 != SENSOR_ERROR_NONE) {
        dlog_print(DLOG_ERROR, LOG_TAG, "sensor_listener_set_interval error: %d", error1);
        return;
    }
    dlog_print(DLOG_DEBUG, LOG_TAG, "sensor_listener_set_intervals");

    error1 = sensor_listener_set_option(listener1, SENSOR_OPTION_ALWAYS_ON);
    if (error1 != SENSOR_ERROR_NONE) {
        dlog_print(DLOG_ERROR, LOG_TAG, "sensor_listener_set_option error: %d", error1);
        return;
    }
    dlog_print(DLOG_DEBUG, LOG_TAG, "sensor_listener_set_option");

    error1 = sensor_listener_start(listener1);
    if (error1 != SENSOR_ERROR_NONE) {
        dlog_print(DLOG_ERROR, LOG_TAG, "sensor_listener_start error: %d", error1);
        return;
    }
    dlog_print(DLOG_DEBUG, LOG_TAG, "sensor_listener_start");

    sensor_event_s event; //hr
    error = sensor_listener_read_data(listener, &event);
    if (error != SENSOR_ERROR_NONE) {
        dlog_print(DLOG_ERROR, LOG_TAG, "sensor_listener_read_data error: %d", error);
        return;
    }

    sensor_event_s event1; //accel
    error1 = sensor_listener_read_data(listener1, &event1);
    if (error1 != SENSOR_ERROR_NONE) {
        dlog_print(DLOG_ERROR, LOG_TAG, "sensor_listener_read_data error: %d", error1);
        return;
    }

    switch (type) {

    case SENSOR_HRM:
       dlog_print(DLOG_INFO, LOG_TAG, "%.0f" , event.values[0]);
       sprintf(out,"%.0f", event.values[0]);
       elm_object_text_set(event_label, out);
       break;

    case SENSOR_ACCELEROMETER:
       dlog_print(DLOG_INFO, LOG_TAG, " X:%f Y:%f Z:%f" , event.values[0], event.values[1], event.values[2]);
       sprintf(out1,"X:%f Y:%f Z:%f", event.values[0], event.values[1], event.values[2]);
       elm_object_text_set(event_label, out1);
       break;

    default:
        dlog_print(DLOG_ERROR, LOG_TAG, "Not an Sensor event");
    }

    dlog_print(DLOG_DEBUG, LOG_TAG, out); //hr
    dlog_print(DLOG_DEBUG, LOG_TAG, out1); //accel

    char *name = NULL; //hr
    char *vendor = NULL;
    float min_range = 0.0;
    float max_range = 220.0;
    float resolution = 0.0;


    char *name1 = NULL; //accel
    char *vendor1 = NULL;
    float min_range1 = -19.6;
    float max_range1 = 19.6;
    float resolution1 = 0.0;

    error = sensor_get_name(sensor, &name); //hr
    if (error != SENSOR_ERROR_NONE) {
        dlog_print(DLOG_ERROR, LOG_TAG, "sensor_get_name error: %d", error);
        return;
    }
    dlog_print(DLOG_DEBUG, LOG_TAG, "Sensor name: %s", name);
    free(name);

    error1 = sensor_get_name(sensor1, &name1); //accel
    if (error1 != SENSOR_ERROR_NONE) {
        dlog_print(DLOG_ERROR, LOG_TAG, "sensor_get_name error: %d", error1);
        return;
    }
    dlog_print(DLOG_DEBUG, LOG_TAG, "Sensor name: %s", name1);
    free(name1);

    error = sensor_get_vendor(sensor, &vendor); //hr
    if (error != SENSOR_ERROR_NONE) {
        dlog_print(DLOG_ERROR, LOG_TAG, "sensor_get_vendor error: %d", error);
        return;
    }
    dlog_print(DLOG_DEBUG, LOG_TAG, "Sensor vendor: %s", vendor);
    free(vendor);

    error1 = sensor_get_vendor(sensor1, &vendor1); //accel
    if (error1 != SENSOR_ERROR_NONE) {
        dlog_print(DLOG_ERROR, LOG_TAG, "sensor_get_vendor error: %d", error1);
        return;
    }
    dlog_print(DLOG_DEBUG, LOG_TAG, "Sensor vendor: %s", vendor1);
    free(vendor1);

    error = sensor_get_type(sensor, &type); //hr
    if (error != SENSOR_ERROR_NONE) {
        dlog_print(DLOG_ERROR, LOG_TAG, "sensor_get_type error: %d", error);
        return;
    }

    error1 = sensor_get_type(sensor1, &type1); //accel
    if (error1 != SENSOR_ERROR_NONE) {
        dlog_print(DLOG_ERROR, LOG_TAG, "sensor_get_type error: %d", error1);
        return;
    }

    dlog_print(DLOG_DEBUG, LOG_TAG, "Sensor type: %s",
            type == SENSOR_ACCELEROMETER               ? "Accelerometer"
          : type == SENSOR_GRAVITY                     ? "Gravity sensor"
          : type == SENSOR_LINEAR_ACCELERATION         ? "Linear acceleration sensor"
          : type == SENSOR_MAGNETIC                    ? "Magnetic sensor"
          : type == SENSOR_ROTATION_VECTOR             ? "Rotation Vector sensor"
          : type == SENSOR_ORIENTATION                 ? "Orientation sensor"
          : type == SENSOR_GYROSCOPE                   ? "Gyroscope sensor"
          : type == SENSOR_LIGHT                       ? "Light sensor"
          : type == SENSOR_PROXIMITY                   ? "Proximity sensor"
          : type == SENSOR_PRESSURE                    ? "Pressure sensor"
          : type == SENSOR_ULTRAVIOLET                 ? "Ultraviolet sensor"
          : type == SENSOR_TEMPERATURE                 ? "Temperature sensor"
          : type == SENSOR_HUMIDITY                    ? "Humidity sensor"
          : type == SENSOR_HRM                         ? "Heart Rate Monitor sensor (Since Tizen 2.3.1)"
          : type == SENSOR_HRM_LED_GREEN               ? "HRM (LED Green) sensor (Since Tizen 2.3.1)"
          : type == SENSOR_HRM_LED_IR                  ? "HRM (LED IR) sensor (Since Tizen 2.3.1)"
          : type == SENSOR_HRM_LED_RED                 ? "HRM (LED RED) sensor (Since Tizen 2.3.1)"
          : type == SENSOR_LAST                        ? "End of sensor enum values" : "Custom sensor");

    error = sensor_get_min_range(sensor, &min_range); //hr
    if (error != SENSOR_ERROR_NONE) {
        dlog_print(DLOG_ERROR, LOG_TAG, "sensor_get_min_range error: %d", error);
        return;
    }
    dlog_print(DLOG_DEBUG, LOG_TAG, "Minimum range of the sensor: %f", min_range);

    error1 = sensor_get_min_range(sensor1, &min_range1); //accel
    if (error1 != SENSOR_ERROR_NONE) {
        dlog_print(DLOG_ERROR, LOG_TAG, "sensor_get_min_range error: %d", error1);
        return;
    }
    dlog_print(DLOG_DEBUG, LOG_TAG, "Minimum range of the sensor: %f", min_range1);

    error = sensor_get_max_range(sensor, &max_range); //hr
    if (error != SENSOR_ERROR_NONE) {
        dlog_print(DLOG_ERROR, LOG_TAG, "sensor_get_max_range error: %d", error);
        return;
    }
    dlog_print(DLOG_DEBUG, LOG_TAG, "Maximum range of the sensor: %f", max_range);

    error1 = sensor_get_max_range(sensor1, &max_range1); //accel
    if (error1 != SENSOR_ERROR_NONE) {
        dlog_print(DLOG_ERROR, LOG_TAG, "sensor_get_max_range error: %d", error1);
        return;
    }
    dlog_print(DLOG_DEBUG, LOG_TAG, "Maximum range of the sensor: %f", max_range1);

    error = sensor_get_resolution(sensor, &resolution); //hr
    if (error != SENSOR_ERROR_NONE) {
        dlog_print(DLOG_ERROR, LOG_TAG, "sensor_get_resolution error: %d", error);
        return;
    }
    dlog_print(DLOG_DEBUG, LOG_TAG, "Resolution of the sensor: %f", resolution);

    elm_object_disabled_set(start, EINA_TRUE);
    elm_object_disabled_set(stop, EINA_FALSE);

    error1 = sensor_get_resolution(sensor1, &resolution1); //accel
    if (error1 != SENSOR_ERROR_NONE) {
        dlog_print(DLOG_ERROR, LOG_TAG, "sensor_get_resolution error: %d", error1);
        return;
    }
    dlog_print(DLOG_DEBUG, LOG_TAG, "Resolution of the sensor: %f", resolution1);

    elm_object_disabled_set(start, EINA_TRUE);
    elm_object_disabled_set(stop, EINA_FALSE);
}

void _sensor_stop_cb(void *data, Evas_Object *obj, void *event_info)
{
    int error = sensor_listener_unset_event_cb(listener); //hr
    if (error != SENSOR_ERROR_NONE) {
        dlog_print(DLOG_ERROR, LOG_TAG, "sensor_listener_unset_event_cb error: %d", error);
    }

    error = sensor_listener_stop(listener);
    if (error != SENSOR_ERROR_NONE) {
        dlog_print(DLOG_ERROR, LOG_TAG, "sensor_listener_stop error: %d", error);
    }

    error = sensor_destroy_listener(listener);
    if (error != SENSOR_ERROR_NONE) {
        dlog_print(DLOG_ERROR, LOG_TAG, "sensor_destroy_listener error: %d", error);
    }

    elm_object_disabled_set(start, EINA_FALSE);
    elm_object_disabled_set(stop, EINA_TRUE);

    int error1 = sensor_listener_unset_event_cb(listener1); //accel
    if (error1 != SENSOR_ERROR_NONE) {
        dlog_print(DLOG_ERROR, LOG_TAG, "sensor_listener_unset_event_cb error: %d", error1);
    }

    error1 = sensor_listener_stop(listener1);
    if (error1 != SENSOR_ERROR_NONE) {
        dlog_print(DLOG_ERROR, LOG_TAG, "sensor_listener_stop error: %d", error1);
    }

    error1 = sensor_destroy_listener(listener1);
    if (error1 != SENSOR_ERROR_NONE) {
        dlog_print(DLOG_ERROR, LOG_TAG, "sensor_destroy_listener error: %d", error1);
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
