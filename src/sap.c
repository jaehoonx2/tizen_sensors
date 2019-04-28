#include "helloaccessory.h"
#include "main_app.h"
#include <sap.h>
#include <sensor.h>
#include <string.h>
#include <glib.h>

#define HELLO_ACC_ASPID "/sample/hello"

extern int hrm_data;
extern float svm_accel[SVMLEN];
extern double time_in_mill;

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

void on_data_received(sap_socket_h socket, unsigned short int channel_id){

	unsigned int payload_length;
 	char * msg = g_strdup_printf("%d %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %.0lf",
 			hrm_data,
			svm_accel[0],  svm_accel[1],  svm_accel[2],
			svm_accel[3],  svm_accel[4],  svm_accel[5],
			svm_accel[6],  svm_accel[7],  svm_accel[8],
			svm_accel[9],  svm_accel[10],  svm_accel[11],
			svm_accel[12],  svm_accel[13],  svm_accel[14],
			svm_accel[15],  svm_accel[16],  svm_accel[17],
			svm_accel[18],  svm_accel[19],  svm_accel[20],
			time_in_mill);

 	payload_length = strlen(msg);
 	dlog_print(DLOG_INFO, LOG_TAG, "%f %f %f %f %f %f %f %f %f %f",
 			svm_accel[0],  svm_accel[1],  svm_accel[2],
			svm_accel[3],  svm_accel[4],  svm_accel[5],
			svm_accel[6],  svm_accel[7],  svm_accel[8],
			svm_accel[9]);
 	//dlog_print(DLOG_INFO, LOG_TAG, "payload length : %d timestamp : %.0lf", payload_length, time_in_mill);
 	sap_socket_send_data(priv_data.socket, HELLO_ACC_CHANNELID, payload_length, msg);	// send the msg to the Consumer(A)
 	g_free(msg);
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

