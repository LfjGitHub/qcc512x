/****************************************************************************
Copyright (c) 2014 - 2016 Qualcomm Technologies International, Ltd.

FILE NAME
    sink_ble_gap.c

DESCRIPTION
    BLE GAP functionality
*/

#include "sink_ba_ble_gap.h"
#include "sink_ble.h"
#include "sink_main_task.h"
#include "sink_ba_common.h"
#include "sink_gatt_db.h"
#include "sink_gatt_manager.h"
#include "sink_ba.h"
#include "sink_ble_scanning.h"
#include "sink_ba_broadcaster.h"
#include "sink_ba_receiver.h"
#include "sink_gatt_common.h"

#include <gatt_manager.h>
#include <vm.h>

#include <stdlib.h>

#ifdef ENABLE_BROADCAST_AUDIO
#include "config_definition.h"
#include "sink_ble_config_def.h"
#include <config_store.h>
#include <gatt_broadcast_server_uuids.h>
#include <broadcast_context.h>

#ifdef DEBUG_BA_BLE_GAP
#define BA_BLE_GAP_INFO(x) DEBUG(x)
#define BA_BLE_GAP_ERROR(x) DEBUG(x) TOLERATED_ERROR(x)
#else
#define BA_BLE_GAP_INFO(x)
#define BA_BLE_GAP_ERROR(x)
#endif /* DEBUG_BLE_GAP */

/* Length of Advertisement Flags */
#define FLAGS_DATA_LENGTH (0x02)

/* Length of 16 bit uuid services */
#define BA_RECEIVER_SERVICES_DATA_LENGTH (0x05)
#define BA_BROADCASTER_SERVICES_DATA_LENGTH (0x05)

/******************************************************************************
  Utility function to set the scan filters for broadcaster to scan only for BA receivers 
*/
static void bleBaSetScanFilters(void)
{
    ble_ad_type ad_type = ble_ad_type_service_16bit_uuid;
    uint16 scan_interval = ADV_SCAN_INTERVAL_SLOW;
    uint16 scan_window = ADV_SCAN_WINDOW_SLOW;
    const uint8 broadcast_advert_filter[] = {UUID_BROADCAST_SERVICE & 0xFF,
                                             UUID_BROADCAST_SERVICE >> 8};
    const uint8 broadcast_2_advert_filter[] = {UUID_BROADCAST_SERVICE_2 & 0xFF,
                                     UUID_BROADCAST_SERVICE_2 >> 8};
    
    /* first clear the existing filter */
    ConnectionBleClearAdvertisingReportFilter();

    /* Just update the correct filter for recevier */
    if(BA_RECEIVER_MODE_ACTIVE)
    {
        ad_type = ble_ad_type_service_data;
        scan_interval = ADV_SCAN_INTERVAL_FAST;
        scan_window = ADV_SCAN_WINDOW_FAST;
        /* As receiver, we can also get adverts from different broadcast UUIDs */
        ConnectionBleAddAdvertisingReportFilter(ad_type, 
                                        sizeof(broadcast_2_advert_filter), 
                                        sizeof(broadcast_2_advert_filter), 
                                        broadcast_2_advert_filter);
    }
    
     /* setup advert filter, scan parameters and start scanning.
     * now waiting on a CL_DM_BLE_ADVERTISING_REPORT_IND message. */
    ConnectionBleAddAdvertisingReportFilter(ad_type, 
                                            sizeof(broadcast_advert_filter), 
                                            sizeof(broadcast_advert_filter), 
                                            broadcast_advert_filter);
    
    /* Set the Scan parameters */
    ConnectionDmBleSetScanParametersReq(TRUE, FALSE, FALSE,
                                        scan_interval,
                                        scan_window);
}

/******************************************************************************
  Utility function to set the advertising params for receiver association
*/
static void bleReceiverSetAdParams(uint16 adv_interval_min, uint16 adv_interval_max)
{
    ble_adv_params_t adv_params;

    adv_params.undirect_adv.adv_interval_min = adv_interval_min; 
    adv_params.undirect_adv.adv_interval_max = adv_interval_max;
    adv_params.undirect_adv.filter_policy = ble_filter_none;

    /* configure fast advertising rate */
    ConnectionDmBleSetAdvertisingParamsReq(ble_adv_ind, FALSE, BLE_ADV_CHANNEL_ALL, &adv_params);
}

/******************************************************************************
  Utility function to set the advertising params for broadcasting variant IV
*/
static void bleBroadcasterSetAdParams(uint16 adv_interval_min, uint16 adv_interval_max)
{
    ble_adv_params_t adv_params;

    adv_params.undirect_adv.adv_interval_min = adv_interval_min; 
    adv_params.undirect_adv.adv_interval_max = adv_interval_max;
    adv_params.undirect_adv.filter_policy = ble_filter_none;

    /* configure fast advertising rate */
    ConnectionDmBleSetAdvertisingParamsReq(ble_adv_nonconn_ind, FALSE, BLE_ADV_CHANNEL_ALL, &adv_params);
}

/******************************************************************************
  Utility function to set the service data for broadcasting variant IV/broadcast audio service
*/
static uint16 setupServiceData(uint8 *ad_data, uint16 ad_index, ble_gap_read_name_t reason)
{
    uint16 service_data_len_offset = ad_index++;
    uint16 ad_type_offset = ad_index++;  

    ad_data[ad_index++] = (UUID_BROADCAST_SERVICE & 0xFF);
    ad_data[ad_index++] = (UUID_BROADCAST_SERVICE >> 8);
    if(reason == ble_gap_read_name_associating)
    {
        /* In case we are receiver setting up BA service data for broadcaster to connect
            then, we need all the service data we want to support */
        ad_data[ad_index++] = (UUID_BROADCAST_SERVICE_2 & 0xFF);
        ad_data[ad_index++] = (UUID_BROADCAST_SERVICE_2 >> 8);
        ad_data[service_data_len_offset] = BA_RECEIVER_SERVICES_DATA_LENGTH;
        ad_data[ad_type_offset] = ble_ad_type_service_16bit_uuid;
    }
    else
    {
        broadcast_encr_config_t *config = BroadcastContextGetEncryptionConfig();
        ad_data[ad_type_offset] = ble_ad_type_service_data;
        if(config)
        {
            ad_data[service_data_len_offset] = BA_BROADCASTER_SERVICES_DATA_LENGTH;
            ad_data[ad_index++] = (config->variant_iv & 0xFF);
            ad_data[ad_index++] = ((config->variant_iv >> 8) & 0xFF);
        }
        else
            ad_data[service_data_len_offset] = 0x03;
    }
    return ad_index;
}


/*******************************************************************************
NAME    
    setupLocalNameAdvertisingData

RETURN
    the offset of the last byte to be written in.
    
DESCRIPTION
    Helper function to setup advertising data to advertise the devices local 
    name used by remote devices scanning for BLE services
*/      
static uint16 setupLocalNameAdvertisingData(uint8 *ad_data, uint16 ad_index, uint16 size_local_name, const uint8 * local_name)
{
	uint16 ad_data_free_space = MAX_AD_DATA_SIZE_IN_OCTETS - ad_index;

    /* Is there a local name to be advertised? If so, is there enough free space in AD Data to advertise it? */
    if ( (0 == size_local_name) || ((AD_DATA_HEADER_SIZE+1) >= ad_data_free_space) )
    {
        return 0;
    }
    else if ((AD_DATA_HEADER_SIZE + size_local_name) <= ad_data_free_space)
    {
        /* advertise the name in full */
        ad_data[ad_index] = size_local_name + 1;
        ad_data[ad_index+1] = ble_ad_type_complete_local_name;
    }
    else
    {
        /* Can advertise a shortened local name */
        ad_data[ad_index] = ad_data_free_space - AD_DATA_HEADER_SIZE +1;    
        ad_data[ad_index+1] = ble_ad_type_shortened_local_name;
    }
    
    /* Setup the local name advertising data */
    memmove( ad_data + ad_index + AD_DATA_HEADER_SIZE, local_name, ((ad_data[ad_index] -1) * sizeof(uint8)));
    return ad_data[ad_index] + AD_DATA_HEADER_SIZE + ad_index -1 ;
}

/******************************************************************************/
void gapBaSetupAdData(uint16 size_local_name, const uint8 *local_name, adv_discoverable_mode_t mode, ble_gap_read_name_t reason)
{
    uint16 ad_data_index = 0;
    uint8 *ad_data = malloc( MAX_AD_DATA_SIZE_IN_OCTETS * sizeof(uint8));
    uint8 flags = BLE_FLAGS_DUAL_HOST;

    if( NULL != ad_data )
    {
        /* Setup the flags advertising data */
        if (mode == adv_discoverable_mode_general)
            flags |= BLE_FLAGS_GENERAL_DISCOVERABLE_MODE;
        else if (mode == adv_discoverable_mode_limited)
            flags |= BLE_FLAGS_LIMITED_DISCOVERABLE_MODE;
    
        ad_data[ad_data_index++] = FLAGS_DATA_LENGTH;
        ad_data[ad_data_index++] = ble_ad_type_flags;
        ad_data[ad_data_index++] = flags;
    
        /* service solicitation advert for Broadcast service */
         ad_data_index = setupServiceData(ad_data, ad_data_index, reason);
    
        /* Setup the local name advertising data */
        ad_data_index = setupLocalNameAdvertisingData(ad_data, ad_data_index, size_local_name, local_name );

        /* Register AD data with the Connection library & Tidy up allocated memory*/
        ConnectionDmBleSetAdvertisingDataReq(ad_data_index, ad_data);

        /* free the buffer after use */
        free (ad_data);
    }
    else
    {
        /* Panic, not enough room to BLE Advertise */
    }
}


/********************************************************************************/
void gapBaStartAssociationTimer(void)
{
    MessageCancelFirst(sinkGetBleTask(), BLE_INTERNAL_MESSAGE_ASSOCIATION_SEARCH_TIMEOUT);
    MessageSendLater(sinkGetBleTask(), BLE_INTERNAL_MESSAGE_ASSOCIATION_SEARCH_TIMEOUT, 0, D_SEC(sinkBroadcastAudioGetAssociationTimeOut()));
}

/********************************************************************************/
void gapBaStopAssociationTimer(void)
{
    MessageCancelFirst(sinkGetBleTask(), BLE_INTERNAL_MESSAGE_ASSOCIATION_SEARCH_TIMEOUT);
    gapBaSetAssociationInProgress(FALSE);
}

/*******************************************************************************/
void gapBaRestartAssociationTimer(void)
{
    /* Should be already in association to restart it */
    if(gapBaGetAssociationInProgress())
    {
        MessageSendLater(sinkGetBleTask(), BLE_INTERNAL_MESSAGE_ASSOCIATION_SEARCH_TIMEOUT, 0, D_SEC(sinkBroadcastAudioGetAssociationTimeOut()));
    }
}

/*******************************************************************************/
void gapBaSetAssociationInProgress(bool assoc_in_progress)
{
    GAP.ba_association_in_progress = assoc_in_progress;
}

/*******************************************************************************/
bool gapBaGetAssociationInProgress(void)
{
    return GAP.ba_association_in_progress;
}

/********************************************************************************/
void gapBaSetBroadcastToAdvert(bool requires_advertising)
{
    GAP.adv.ba_advert_active = requires_advertising;
}

/********************************************************************************/
bool gapBaRequiresBroadcastToAdvert(void)
{
    return (GAP.adv.ba_advert_active) ? TRUE : FALSE;
}

/********************************************************************************/
void gapBaSetBroadcastToScan(bool requires_scanning)
{
    GAP.scan.ba_scan_active = requires_scanning;
}

/********************************************************************************/
bool gapBaRequiresBroadcastToScan(void)
{
    return (GAP.scan.ba_scan_active) ? TRUE : FALSE;
}


/********************************************************************************/
bool gapBaStartScanning(void)
{
    BA_BLE_GAP_INFO(("BA BLE: Start scanning:  \n"));
    /* In broadcast mode, check which role we are in and scan accordingly */
    if((BA_BROADCASTER_MODE_ACTIVE) && gapBaGetAssociationInProgress())
    {
        BA_BLE_GAP_INFO((" Connectable BA Association data\n"));
        /* Set the scan filter for scanning only BA receivers */
        bleBaSetScanFilters();
        /* Enable Scanning */
        ConnectionDmBleSetScanEnable(TRUE);
    }
    else if((BA_RECEIVER_MODE_ACTIVE) && gapBaRequiresBroadcastToScan())
    {
        BA_BLE_GAP_INFO((" Non-Conntable Variant IV data\n"));
        /* Start scanning for variant IV */
        bleBaSetScanFilters();
        /* Enable Scanning */
        ConnectionDmBleSetScanEnable(TRUE);
    }
    else
    {
        /* Ignore this scan request, as we are in broadcast mode, but don't require to scan */
        BA_BLE_GAP_INFO(("GAP: IGNORING the scanning, since BA is active & neither are we assocaiting or broadcasting\n"));
        return FALSE;
    }

    return TRUE;
}

/********************************************************************************/
void gapBaStopScanning(void)
{
    BA_BLE_GAP_INFO(("BA BLE: Stop scanning \n"));

    /* stop scanning for adverts from CSB receivers. */
    ConnectionDmBleSetScanEnable(FALSE);
    /* cancel any pending CL_DM_BLE_ADVERTISING_REPORT_IND message */
    MessageCancelAll(sinkGetMainTask(), CL_DM_BLE_ADVERTISING_REPORT_IND);
    /* If we stopped scanning, just because we connected to receiver on association,
        then we just need to disable scanning. However we can keep the filters as 
        scanning for new receiver can trigger after we are done with current receiver */
    if(!gapBaGetAssociationInProgress())
    {
        /* clear the advertising report filter */
        ConnectionBleClearAdvertisingReportFilter();
        /* Add the standalone filters back */
        bleAddScanFilters();
    }
}

/********************************************************************************/
void gapBaTriggerAdvertising(void)
{
    BA_BLE_GAP_INFO(("BA BLE: Trigger Advertising after setting the advert params: "));
    if((BA_BROADCASTER_MODE_ACTIVE) && gapBaRequiresBroadcastToAdvert())
    {
        BA_BLE_GAP_INFO((" Non-Conntable Variant IV data\n"));
        /* start the adverts */
        ConnectionDmBleSetAdvertiseEnable(TRUE);
    }
    else if((BA_RECEIVER_MODE_ACTIVE) && gapBaGetAssociationInProgress())
    {
        BA_BLE_GAP_INFO((" Connectable BA Association data\n"));
        /* Start sending advertisments */
        sinkGattManagerStartAdvertising();
    }
    else
    {
        BA_BLE_GAP_INFO((" Ignore this\n"));
        /* Do nothing */
    }
}

/********************************************************************************/
bool gapBaStartAdvertising(void)
{
    BA_BLE_GAP_INFO(("BA BLE: Start Advertising: "));
    if((BA_BROADCASTER_MODE_ACTIVE) && gapBaRequiresBroadcastToAdvert())
    {
        BA_BLE_GAP_INFO((" Non-Conntable Variant IV data\n"));
        bleBroadcasterSetAdParams(ADV_INTERVAL_MIN_SLOW, ADV_INTERVAL_MAX_SLOW);
       /* Some platform requires delay before triggering advertisement. So wait for 
           CL_DM_BLE_SET_ADVERTISING_PARAMS_CFM to trigger advertisement */
    }
    else if((BA_RECEIVER_MODE_ACTIVE) && gapBaGetAssociationInProgress())
    {
        BA_BLE_GAP_INFO((" Connectable BA Association data\n"));
        /* Update the advertisement data for BA */
        bleReceiverSetAdParams(ADV_INTERVAL_MIN_FAST, ADV_INTERVAL_MAX_FAST);
       /* Some platform requires delay before triggering advertisement. So wait for 
           CL_DM_BLE_SET_ADVERTISING_PARAMS_CFM to trigger advertisement */
    }
    else if(sinkBroadcastAudioIsActive() && sinkBleGapIsBondable())
    {
        BA_BLE_GAP_INFO((" Connectable standalone bondable data\n"));
        /* Allow standalone advertisements only in bondable state of broadcast mode */
        /* Change advertising params */
        gapSetAdvertisingParamsDefault();
        /* Start sending advertisments */
        sinkGattManagerStartAdvertising();        
    }
    else
    {
        /* Just ignore */
        BA_BLE_GAP_INFO(("GAP: IGNORING the advertsising, since BA is active & we are not neither are we broadcasting, associating or bonding\n"));
        return FALSE;
    }
    return TRUE;
}

/********************************************************************************/
static void gapBaStopAdvertising(void)
{
    BA_BLE_GAP_INFO(("BA BLE: Stop Advertising\n"));
    ConnectionDmBleSetAdvertiseEnable(FALSE);
}

/******************************************************************************/
void gapBaStartAssociation(void)
{
    bool is_asso_started = FALSE;
    BA_BLE_GAP_INFO(("GAP sinkBaBleGapStartAssociation\n"));

    /* Now in assoicating state */
    sinkBleSetGapState(ble_gap_state_bondable_associating_scanning_advertising);
    gapBaSetAssociationInProgress(TRUE);
    
    if(BA_BROADCASTER_MODE_ACTIVE)
    {
        is_asso_started = gapStartScanning(TRUE);
    }
    else if(BA_RECEIVER_MODE_ACTIVE)
    {
        /* Stop scanning for BA and non BA adverts */
        gapStopScanning();
        sinkBleGapStartReadLocalName(ble_gap_read_name_associating);
        is_asso_started = TRUE;
    }
    else
        /* Just igore, we can't be in this standalone mode to do association */
        return;

    /* If scanning started, start the timer */
    if (is_asso_started)
    {
        /* Indicate on LED that association is started */
        MessageSend(sinkGetMainTask(), EventSysBAAssociationStart, 0);
        /* Start association timer */
        gapBaStartAssociationTimer();
    }
}

/******************************************************************************/
void gapBaStopAssociation(void)
{
    BA_BLE_GAP_INFO(("GAP gapBaStopAssociation\n"));

    /* Reset the association flag */
    gapBaSetAssociationInProgress(FALSE);
        
    if(BA_BROADCASTER_MODE_ACTIVE)
        gapStopScanning();
    else if(BA_RECEIVER_MODE_ACTIVE)
    {
        gapStopAdvertising(gapGetAdvSpeed());
        /* In case we are in receiver role, then we need to indicate that Receiver is not associated */
        MessageSend(sinkGetMainTask() , EventSysBAReceiverNotAssociated, NULL ) ;
    }

    /* Indicate on LED that association is ended */
    MessageSend(sinkGetMainTask(), EventSysBAAssociationEnd, 0);
}

/******************************************************************************/
void gapBaStartBroadcast(void)
{
    BA_BLE_GAP_INFO(("GAP gapBaStartBroadcast\n"));

    if(BA_BROADCASTER_MODE_ACTIVE)
    {
        /* start reading local name to do non-connectable IV adverts */
        gapBaSetBroadcastToAdvert(TRUE);
        sinkBleGapStartReadLocalName(ble_gap_read_name_broadcasting);
    }
    else if(BA_RECEIVER_MODE_ACTIVE)
    {
        /* First check if we already have an associated broadcaster if yes then start scanning 
            else remain in same state */
        if(!sinkReceiverIsAssociated())
            return;

        /* Going to scan for fresh Variant IV */
        sinkReceiverResetVariantIv();
        gapBaSetBroadcastToScan(TRUE);
        gapStartScanning(TRUE);
    }

    /* We need to be in scanning/advertising state */
    sinkBleSetGapState(ble_gap_state_scanning_advertising);
}

/******************************************************************************/
void gapBaStopBroadcast(void)
{
    BA_BLE_GAP_INFO(("GAP gapBaStopBroadcast\n"));

    if(BA_BROADCASTER_MODE_ACTIVE)
    {
        /* Looks like user pressed bondable button. Stop non-conn IV adverts */
        gapBaSetBroadcastToAdvert(FALSE);
        gapBaStopAdvertising();
    }
    else if(BA_RECEIVER_MODE_ACTIVE)
    {
        /* Stop scanning for non-conn IV */
        gapBaSetBroadcastToScan(FALSE);
        gapBaStopScanning();
    }
    else
    {
        BA_BLE_GAP_INFO(("GAP Switch to standalone mode, start normal advertising/scanning\n"));
        /* If there is mode switch from BA mode to standalone
             then we need to stop broadcasting and start standalone 
             advertising/scanning */
        /* We might be broadcasting IV */
        gapBaSetBroadcastToAdvert(FALSE);
        gapBaStopAdvertising();

        /* Or scanning for non-conn IV */
        gapBaSetBroadcastToScan(FALSE);
        gapBaStopScanning();

        /* Now start standalone adverts */
        sinkBleSetGapState(ble_gap_state_scanning_advertising);
        gapStartScanning(TRUE);
        sinkBleGapStartReadLocalName(ble_gap_read_name_advertising);

        return;
    }

    /* In case we are stopping broadcasting, then we need to move to following state:
        1. Idle -> If there is no peripheral connection
        2. Fully Connected (1 link reserved for association) -> If there is one peripheral link
    */
    sinkBleSetGapState(ble_gap_state_idle);
    if(gattCommonGetNumOfConn(ble_gap_role_peripheral) == (MAX_BLE_CONNECTIONS - 1))
        sinkBleSetGapState(ble_gap_state_fully_connected);
}

/******************************************************************************/
void gapBaActionCancelAssociation(void)
{
    /* First stop association */
    gapBaStopAssociation();

    /* If receiver role, then based on which state association we called, move to that state
        back */
    if(BA_RECEIVER_MODE_ACTIVE)
    {
        if(gattCommonGetNumberOfConn() == (MAX_BLE_CONNECTIONS-1))
            sinkBleSetGapState(ble_gap_state_fully_connected);
        else if(!ConnectionDmBleCheckTdlDeviceAvailable())
            sinkBleSetGapState(ble_gap_state_idle);
        else
            /* We need to be in scanning/advertising state */
            sinkBleSetGapState(ble_gap_state_scanning_advertising);
    }
    /* Nothing happened after association, lets move to scan_advert state */
    gapBaStartBroadcast();
}

#endif /* ENABLE_BROADCAST_AUDIO */

