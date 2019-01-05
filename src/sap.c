#include "helloaccessory.h"
#include "main_app.h"
#include <sensor.h>
#include <sap.h>
#include <glib.h>
#include <string.h>

#define HELLO_ACC_ASPID "/sample/hello"
#define HELLO_ACC_CHANNELID 104

struct priv {
   sap_agent_h agent;
   sap_socket_h socket;
   sap_peer_agent_h peer_agent;
};

static gboolean agent_created = FALSE;

static struct priv priv_data = { 0 };

static void on_service_connection_terminated(sap_peer_agent_h peer_agent,
                    sap_socket_h socket,
                    sap_service_connection_terminated_reason_e result,
                    void *user_data)
{
   switch (result) {
   case SAP_CONNECTION_TERMINATED_REASON_PEER_DISCONNECTED:
      break;

   case SAP_CONNECTION_TERMINATED_REASON_DEVICE_DETACHED:
      break;

   case SAP_CONNECTION_TERMINATED_REASON_UNKNOWN:
      break;
   }


   sap_socket_destroy(priv_data.socket);
   priv_data.socket = NULL;
}

extern struct Data d1;

void on_data_received(sap_socket_h socket, unsigned short int channel_id){
        unsigned int payload_length;

        time_t raw_time; //test
        struct tm* time_info; //test
        time(&raw_time); //test
        time_info = localtime(&raw_time); //test
        int len=snprintf (d1.time_data,0, "%s%d:%s%d:%s%d", time_info->tm_hour<10? "0" : "",time_info->tm_hour, time_info->tm_min<10? "0" : "", time_info->tm_min,time_info->tm_sec<10? "0" : "",time_info->tm_sec);
        d1.time_data=malloc(len+1);
        sprintf(d1.time_data,"%s%d:%s%d:%s%d",time_info->tm_hour<10? "0" : "", time_info->tm_hour, time_info->tm_min<10? "0" : "", time_info->tm_min,time_info->tm_sec<10? "0" : "",time_info->tm_sec);

        if(d1.hrm_data < 1){
          char *msg = (char *) malloc(strlen("Not Ready HRM data"));
          strcpy(msg, "Not Ready HRM data");
          payload_length = strlen(msg);
          sap_socket_send_data(priv_data.socket, HELLO_ACC_CHANNELID, payload_length, msg);
          dlog_print(DLOG_INFO, LOG_TAG, "%s",msg); //test
          free(msg);
       }
        else {
          //d1.sum= g_strdup_printf("%d\t%s\t%.1lf\t%.1lf\t%.1lf", d1.hrm_data, d1.time_data, d1.accel_data[0], d1.accel_data[1], d1.accel_data[2]);//test
          d1.sum= g_strdup_printf("%d %s %.1lf %.1lf %.1lf %.1lf %.1lf %.1lf", d1.hrm_data, d1.time_data, d1.accel_data[0], d1.accel_data[1], d1.accel_data[2], d1.gyro_data[0], d1.gyro_data[1], d1.gyro_data[2]);
          payload_length = strlen(d1.sum);
          sap_socket_send_data(priv_data.socket, HELLO_ACC_CHANNELID, payload_length, d1.sum);
          dlog_print(DLOG_INFO, LOG_TAG, "%s",d1.sum); //test
          g_free(d1.sum);
          //send the buffer to the consumer
       }
}

static void on_service_connection_requested(sap_peer_agent_h peer_agent,
                   sap_socket_h socket,
                   sap_service_connection_result_e result,
                   void *user_data)
{
   priv_data.socket = socket;
   priv_data.peer_agent = peer_agent;

   sap_peer_agent_set_service_connection_terminated_cb
      (priv_data.peer_agent, on_service_connection_terminated, user_data);

   sap_socket_set_data_received_cb(socket, on_data_received, peer_agent);

   sap_peer_agent_accept_service_connection(peer_agent);


}

static void on_agent_initialized(sap_agent_h agent,
             sap_agent_initialized_result_e result,
             void *user_data)
{
   switch (result) {
   case SAP_AGENT_INITIALIZED_RESULT_SUCCESS:

      sap_agent_set_service_connection_requested_cb(agent,
                           on_service_connection_requested,
                           NULL);

      priv_data.agent = agent;
      agent_created = TRUE;
      break;

   case SAP_AGENT_INITIALIZED_RESULT_DUPLICATED:
      break;

   case SAP_AGENT_INITIALIZED_RESULT_INVALID_ARGUMENTS:
      break;

   case SAP_AGENT_INITIALIZED_RESULT_INTERNAL_ERROR:
      break;

   default:
      break;
   }

}

static void on_device_status_changed(sap_device_status_e status, sap_transport_type_e transport_type,
                 void *user_data)
{
   switch (transport_type) {
   case SAP_TRANSPORT_TYPE_BT:
      break;

   case SAP_TRANSPORT_TYPE_BLE:
      break;

   case SAP_TRANSPORT_TYPE_TCP:
      break;

   case SAP_TRANSPORT_TYPE_USB:
      break;

   case SAP_TRANSPORT_TYPE_MOBILE:
      break;

   default:
      break;
   }

   switch (status) {
   case SAP_DEVICE_STATUS_DETACHED:

      if (priv_data.peer_agent) {
         sap_socket_destroy(priv_data.socket);
         priv_data.socket = NULL;
         sap_peer_agent_destroy(priv_data.peer_agent);
         priv_data.peer_agent = NULL;

      }

      break;

   case SAP_DEVICE_STATUS_ATTACHED:
      break;

   default:
      break;
   }
}

gboolean agent_initialize()
{
   int result = 0;

   do {
      result = sap_agent_initialize(priv_data.agent, HELLO_ACC_ASPID, SAP_AGENT_ROLE_PROVIDER,
                     on_agent_initialized, NULL);

   } while (result != SAP_RESULT_SUCCESS);

   return TRUE;
}

void initialize_sap(){
   sap_agent_h agent = NULL;

      sap_agent_create(&agent);

      if (agent == NULL){}
      else{}

      priv_data.agent = agent;

      sap_set_device_status_changed_cb(on_device_status_changed, NULL);

      agent_initialize();
}

