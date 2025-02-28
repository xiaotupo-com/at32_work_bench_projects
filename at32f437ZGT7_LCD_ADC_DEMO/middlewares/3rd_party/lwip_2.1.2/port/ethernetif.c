/**
 * @file
 * Ethernet Interface Skeleton
 *
 */

/*
 * Copyright (c) 2001-2004 Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 */

/*
 * This file is a skeleton for developing Ethernet network interface
 * drivers for lwIP. Add code to the low_level functions and do a
 * search-and-replace for the word "ethernetif" to replace it with
 * something that better describes your network interface.
 */

#include "lwip/opt.h"
#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include "lwip/sys.h"
#include <lwip/stats.h>
#include <lwip/snmp.h>
#include "netif/etharp.h"
#include "netif/ppp/pppoe.h"
#include "err.h"
#include "ethernetif.h"

#include "at32f435_437_emac.h"
#include <string.h>

/* TCP and ARP timeouts */
volatile int tcp_end_time, arp_end_time;

/* Define those to better describe your network interface. */
#define IFNAME0 'a'
#define IFNAME1 't'

#define EMAC_DMARxDesc_FrameLengthShift  16

/**
 * Helper struct to hold private data used to operate your ethernet interface.
 * Keeping the ethernet address of the MAC in this struct is not necessary
 * as it is already kept in the struct netif.
 * But this is only an example, anyway...
 */
struct ethernetif
{
  struct eth_addr *ethaddr;
  /* Add whatever per-interface state that is needed here. */
  int unused;
};

/* Forward declarations. */
err_t  ethernetif_input(struct netif *netif);

#define EMAC_RXBUFNB        6
#define EMAC_TXBUFNB        6

uint8_t MACaddr[6];
emac_dma_desc_type  DMARxDscrTab[EMAC_RXBUFNB], DMATxDscrTab[EMAC_TXBUFNB];/* Ethernet Rx & Tx DMA Descriptors */
uint8_t Rx_Buff[EMAC_RXBUFNB][EMAC_MAX_PACKET_LENGTH], Tx_Buff[EMAC_TXBUFNB][EMAC_MAX_PACKET_LENGTH];/* Ethernet buffers */

extern emac_dma_desc_type  *dma_tx_desc_to_set;
extern emac_dma_desc_type  *dma_rx_desc_to_get;

typedef struct{
  u32 length;
  u32 buffer;
  emac_dma_desc_type *descriptor;
  emac_dma_desc_type *rx_fs_desc;
  emac_dma_desc_type *rx_ls_desc;
  uint32_t g_seg_count;
}FrameTypeDef;

FrameTypeDef rx_frame;

error_status emac_rxpkt_chainmode(void);
u32 emac_getcurrenttxbuffer(void);
error_status emac_txpkt_chainmode(uint32_t FrameLength);


/**
 * Setting the MAC address.
 *
 * @param netif the already initialized lwip network interface structure
 *        for this ethernetif
 */
void lwip_set_mac_address(uint8_t* macadd)
{
  MACaddr[0] = macadd[0];
  MACaddr[1] = macadd[1];
  MACaddr[2] = macadd[2];
  MACaddr[3] = macadd[3];
  MACaddr[4] = macadd[4];
  MACaddr[5] = macadd[5];

  emac_local_address_set(macadd);
}


/**
 * In this function, the hardware should be initialized.
 * Called from ethernetif_init().
 *
 * @param netif the already initialized lwip network interface structure
 *        for this ethernetif
 */
static void
low_level_init(struct netif *netif)
{
  uint32_t index = 0;
  
  /* set MAC hardware address length */
  netif->hwaddr_len = ETHARP_HWADDR_LEN;

  /* set MAC hardware address */
  netif->hwaddr[0] =  MACaddr[0];
  netif->hwaddr[1] =  MACaddr[1];
  netif->hwaddr[2] =  MACaddr[2];
  netif->hwaddr[3] =  MACaddr[3];
  netif->hwaddr[4] =  MACaddr[4];
  netif->hwaddr[5] =  MACaddr[5];

  /* maximum transfer unit */
  netif->mtu = 1500;

  /* device capabilities */
  /* don't set NETIF_FLAG_ETHARP if this device is not an ethernet one */
  netif->flags |= NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_LINK_UP;

  /* Initialize Tx Descriptors list: Chain Mode */
  emac_dma_descriptor_list_address_set(EMAC_DMA_TRANSMIT, DMATxDscrTab, &Tx_Buff[0][0], EMAC_TXBUFNB);
  /* Initialize Rx Descriptors list: Chain Mode  */
  emac_dma_descriptor_list_address_set(EMAC_DMA_RECEIVE, DMARxDscrTab, &Rx_Buff[0][0], EMAC_RXBUFNB);

  /* Enable Ethernet Rx interrrupt */
  for(index = 0; index < EMAC_RXBUFNB; index ++)
  {
    emac_dma_rx_desc_interrupt_config(&DMARxDscrTab[index], TRUE);
  }
  
#ifdef CHECKSUM_BY_HARDWARE
  for(index = 0; index < EMAC_TXBUFNB; index ++)
  {
    DMATxDscrTab[index].status |= EMAC_DMATXDESC_CIC_TUI_FULL;
  }
#endif
  
  rx_frame.g_seg_count = 0;

  /* Enable MAC and DMA transmission and reception */
  emac_start();

}

/**
 * This function should do the actual transmission of the packet. The packet is
 * contained in the pbuf that is passed to the function. This pbuf
 * might be chained.
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @param p the MAC packet to send (e.g. IP packet including MAC addresses and type)
 * @return ERR_OK if the packet could be sent
 *         an err_t value if the packet couldn't be sent
 *
 * @note Returning ERR_MEM here if a DMA queue of your MAC is full can lead to
 *       strange results. You might consider waiting for space in the DMA queue
 *       to become availale since the stack doesn't retry to send a packet
 *       dropped because of memory failure (except for the TCP timers).
 */

static err_t
low_level_output(struct netif *netif, struct pbuf *p)
{
  struct pbuf *q;
  err_t errno;
  emac_dma_desc_type *dma_tx_desc;
  uint8_t *buffer;
  uint32_t length = 0;
  uint32_t buffer_offset = 0, payload_offset = 0, copy_count = 0;
 
  dma_tx_desc = dma_tx_desc_to_set;
  buffer =  (uint8_t *)emac_getcurrenttxbuffer();
  
  /* copy data to buffer */
  for(q = p; q != NULL; q = q->next)
  {
    if((dma_tx_desc->status & EMAC_DMATXDESC_OWN) != RESET)
    {
      errno = ERR_USE;
      goto out_error;
    }
    
    copy_count = q->len;
    payload_offset = 0;
    
    while((copy_count + buffer_offset) > EMAC_MAX_PACKET_LENGTH)
    {
      memcpy(buffer + buffer_offset, (uint8_t *)q->payload + payload_offset, (EMAC_MAX_PACKET_LENGTH - buffer_offset));
      dma_tx_desc = (emac_dma_desc_type*)dma_tx_desc->buf2nextdescaddr;
      
      if((dma_tx_desc->status & EMAC_DMATXDESC_OWN) != RESET)
      {
        errno = ERR_USE;
        goto out_error;
      }
      
      buffer = (uint8_t *)dma_tx_desc->buf1addr;
      
      copy_count = copy_count - (EMAC_MAX_PACKET_LENGTH - buffer_offset);
      payload_offset = payload_offset + (EMAC_MAX_PACKET_LENGTH - buffer_offset);
      length = length + (EMAC_MAX_PACKET_LENGTH - buffer_offset);
      buffer_offset = 0;
    }
    
    memcpy(buffer + buffer_offset, (uint8_t *)q->payload + payload_offset, copy_count);
    buffer_offset = buffer_offset + copy_count;
    length = length + copy_count;
  }
  
  emac_txpkt_chainmode(length);
  
  errno = ERR_OK;
  
out_error:
   /* When Tx Buffer unavailable flag is set: clear it and resume transmission */
  if(emac_dma_flag_get(EMAC_DMA_TBU_FLAG))
  {
    /* Clear TBUS ETHERNET DMA flag */
    emac_dma_flag_clear(EMAC_DMA_TBU_FLAG);
    /* Resume DMA transmission*/
    EMAC_DMA->tpd_bit.tpd = 0;
  }
  
  return errno;
}

/**
 * Should allocate a pbuf and transfer the bytes of the incoming
 * packet from the interface into the pbuf.
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @return a pbuf filled with the received packet (including MAC header)
 *         NULL on memory error
 */
static struct pbuf *
low_level_input(struct netif *netif)
{
  struct pbuf *p, *q;
  uint32_t len = 0;
  emac_dma_desc_type *dma_rx_desc;
  uint8_t *buffer;
  uint32_t buffer_offset, payload_offset = 0, copy_count = 0;
  uint32_t index = 0;

  p = NULL;
  
  if(emac_rxpkt_chainmode() != SUCCESS)
  {
    return NULL;
  }
  
  /* Obtain the size of the packet and put it into the "len"
     variable. */
  len = rx_frame.length;
  buffer = (uint8_t *)rx_frame.buffer;
  
  /* We allocate a pbuf chain of pbufs from the pool. */
  if(len > 0)
  {
    p = pbuf_alloc(PBUF_RAW, len, PBUF_POOL);
  }
  
  if(p != NULL)
  {
    dma_rx_desc = rx_frame.rx_fs_desc;
    buffer_offset = 0;
    
    for (q = p; q != NULL; q = q->next)
    {
      copy_count = q->len;
      payload_offset = 0;
      
      while( (copy_count + buffer_offset) > EMAC_MAX_PACKET_LENGTH )
      {
        /* copy data to pbuf */
        memcpy((uint8_t*)q->payload + payload_offset, buffer + buffer_offset, (EMAC_MAX_PACKET_LENGTH - buffer_offset));
        
        /* point to next descriptor */
        dma_rx_desc = (emac_dma_desc_type *)(dma_rx_desc->buf2nextdescaddr);
        buffer = (uint8_t *)(dma_rx_desc->buf1addr);
        
        copy_count = copy_count - (EMAC_MAX_PACKET_LENGTH - buffer_offset);
        payload_offset = payload_offset + (EMAC_MAX_PACKET_LENGTH - buffer_offset);
        buffer_offset = 0;
      }
      
      memcpy((uint8_t*)q->payload + payload_offset, (uint8_t*)buffer + buffer_offset, copy_count);
      buffer_offset = buffer_offset + copy_count;
    }
    
  }
  
  dma_rx_desc = rx_frame.rx_fs_desc;
  for(index = 0; index < rx_frame.g_seg_count; index ++)
  {
    dma_rx_desc->status |= EMAC_DMARXDESC_OWN;
    dma_rx_desc = (emac_dma_desc_type*) (dma_rx_desc->buf2nextdescaddr);
  }
  
  rx_frame.g_seg_count = 0;
  
  /* When Rx Buffer unavailable flag is set: clear it and resume reception */
  if(emac_dma_flag_get(EMAC_DMA_RBU_FLAG))
  {
    /* Clear RBUS ETHERNET DMA flag */
    emac_dma_flag_clear(EMAC_DMA_RBU_FLAG);
    /* Resume DMA reception */
    EMAC_DMA->rpd_bit.rpd = FALSE;
  }
  
  return p;
}

/**
 * This function should be called when a packet is ready to be read
 * from the interface. It uses the function low_level_input() that
 * should handle the actual reception of bytes from the network
 * interface. Then the type of the received packet is determined and
 * the appropriate input function is called.
 *
 * @param netif the lwip network interface structure for this ethernetif
 */
err_t
ethernetif_input(struct netif *netif)
{
  err_t err;
  struct pbuf *p;

  /* move received packet into a new pbuf */
  p = low_level_input(netif);

  /* no packet could be read, silently ignore this */
  if (p == NULL) return ERR_MEM;

  err = netif->input(p, netif);
  if (err != ERR_OK)
  {
    LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_input: IP input error\n"));
    pbuf_free(p);
    p = NULL;
  }

  return err;
}

/**
 * Should be called at the beginning of the program to set up the
 * network interface. It calls the function low_level_init() to do the
 * actual setup of the hardware.
 *
 * This function should be passed as a parameter to netif_add().
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @return ERR_OK if the loopif is initialized
 *         ERR_MEM if private data couldn't be allocated
 *         any other err_t on error
 */
err_t
ethernetif_init(struct netif *netif)
{
  struct ethernetif *ethernetif;

  LWIP_ASSERT("netif != NULL", (netif != NULL));

  ethernetif = mem_malloc(sizeof(struct ethernetif));
  if (ethernetif == NULL)
  {
    LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_init: out of memory\n"));
    return ERR_MEM;
  }

#if LWIP_NETIF_HOSTNAME
  /* Initialize interface hostname */
  netif->hostname = "lwip";
#endif /* LWIP_NETIF_HOSTNAME */

  /*
   * Initialize the snmp variables and counters inside the struct netif.
   * The last argument should be replaced with your link speed, in units
   * of bits per second.
   */
  NETIF_INIT_SNMP(netif, snmp_ifType_ethernet_csmacd, 100000000);

  netif->state = ethernetif;
  netif->name[0] = IFNAME0;
  netif->name[1] = IFNAME1;
  /* We directly use etharp_output() here to save a function call.
   * You can instead declare your own function an call etharp_output()
   * from it if you have to do some checks before sending (e.g. if link
   * is available...) */
  netif->output = etharp_output;
  netif->linkoutput = low_level_output;

  ethernetif->ethaddr = (struct eth_addr *)&(netif->hwaddr[0]);

  /* initialize the hardware */
  low_level_init(netif);

  return ERR_OK;
}

/*******************************************************************************
* Function Name  : emac_rxpkt_chainmode
* Description    : Receives a packet.
* Input          : None
* Output         : None
* Return         : ERROR: in case of Tx desc owned by DMA
*                  SUCCESS: for correct transmission
*******************************************************************************/
error_status emac_rxpkt_chainmode(void)
{
  /* Check if the descriptor is owned by the ETHERNET DMA (when set) or CPU (when reset) */
  if((dma_rx_desc_to_get->status & EMAC_DMARXDESC_OWN) != (u32)RESET)
  {    
    /* return error: own bit set */
    return ERROR;
  }
  if((dma_rx_desc_to_get->status & EMAC_DMARXDESC_LS) != (u32)RESET)
  {
    rx_frame.g_seg_count ++;
    if(rx_frame.g_seg_count == 1)
    {
      rx_frame.rx_fs_desc = dma_rx_desc_to_get;
    }
    rx_frame.rx_ls_desc = dma_rx_desc_to_get;
    rx_frame.length = ((dma_rx_desc_to_get->status & EMAC_DMARXDESC_FL) >> EMAC_DMARxDesc_FrameLengthShift) - 4;
    rx_frame.buffer = rx_frame.rx_fs_desc->buf1addr;
    
    /* Selects the next DMA Rx descriptor list for next buffer to read */
    dma_rx_desc_to_get = (emac_dma_desc_type*) (dma_rx_desc_to_get->buf2nextdescaddr);
    
    return SUCCESS;
  }
  else if((dma_rx_desc_to_get->status & EMAC_DMARXDESC_FS) != (u32)RESET)
  {
    rx_frame.g_seg_count = 1;
    rx_frame.rx_fs_desc = dma_rx_desc_to_get;
    rx_frame.rx_ls_desc = NULL;
    dma_rx_desc_to_get = (emac_dma_desc_type*) (dma_rx_desc_to_get->buf2nextdescaddr);
  }
  else
  {
    rx_frame.g_seg_count ++;
    dma_rx_desc_to_get = (emac_dma_desc_type*) (dma_rx_desc_to_get->buf2nextdescaddr);
  }

  return ERROR;
}

/*******************************************************************************
* Function Name  : emac_txpkt_chainmode
* Description    : Transmits a packet, from application buffer, pointed by ppkt.
* Input          : - FrameLength: Tx Packet size.
* Output         : None
* Return         : ERROR: in case of Tx desc owned by DMA
*                  SUCCESS: for correct transmission
*******************************************************************************/
error_status emac_txpkt_chainmode(uint32_t FrameLength)
{
  uint32_t buf_cnt = 0, index = 0;
  
  /* Check if the descriptor is owned by the ETHERNET DMA (when set) or CPU (when reset) */
  if((dma_tx_desc_to_set->status & EMAC_DMATXDESC_OWN) != (u32)RESET)
  {
    /* Return ERROR: OWN bit set */
    return ERROR;
  }
  
  if(FrameLength == 0)
  {
    return ERROR;
  }
  
  if(FrameLength > EMAC_MAX_PACKET_LENGTH)
  {
    buf_cnt = FrameLength / EMAC_MAX_PACKET_LENGTH;
    if(FrameLength % EMAC_MAX_PACKET_LENGTH)
    {
      buf_cnt += 1;
    }
  }
  else
  {
    buf_cnt = 1;
  }
  
  if(buf_cnt == 1)
  {
    /* Setting the last segment and first segment bits (in this case a frame is transmitted in one descriptor) */
    dma_tx_desc_to_set->status |= EMAC_DMATXDESC_LS | EMAC_DMATXDESC_FS;
    
    /* Setting the Frame Length: bits[12:0] */
    dma_tx_desc_to_set->controlsize = (FrameLength & EMAC_DMATXDESC_TBS1);
    
    /* Set Own bit of the Tx descriptor Status: gives the buffer back to ETHERNET DMA */
    dma_tx_desc_to_set->status |= EMAC_DMATXDESC_OWN;
    
    /* Selects the next DMA Tx descriptor list for next buffer to send */
    dma_tx_desc_to_set = (emac_dma_desc_type*) (dma_tx_desc_to_set->buf2nextdescaddr);
  }
  else
  {
    for(index = 0; index < buf_cnt; index ++)
    {
      /* clear first and last segments */
      dma_tx_desc_to_set->status &= ~(EMAC_DMATXDESC_LS | EMAC_DMATXDESC_FS);
      
      /* set first segments */
      if(index == 0)
      {
        dma_tx_desc_to_set->status |= EMAC_DMATXDESC_FS;
      }
      
      /* set size */
      dma_tx_desc_to_set->controlsize = (EMAC_MAX_PACKET_LENGTH & EMAC_DMATXDESC_TBS1);
      
      /* set last segments */
      if(index == (buf_cnt - 1))
      {
        dma_tx_desc_to_set->status |= EMAC_DMATXDESC_LS;
        dma_tx_desc_to_set->controlsize = ((FrameLength - ((buf_cnt-1) * EMAC_MAX_PACKET_LENGTH)) & EMAC_DMATXDESC_TBS1);
      }
      
      /* Set Own bit of the Tx descriptor Status: gives the buffer back to ETHERNET DMA */
      dma_tx_desc_to_set->status |= EMAC_DMATXDESC_OWN;
      
      /* Selects the next DMA Tx descriptor list for next buffer to send */
      dma_tx_desc_to_set = (emac_dma_desc_type*) (dma_tx_desc_to_set->buf2nextdescaddr);
      
    }
  }
  
  /* When Tx Buffer unavailable flag is set: clear it and resume transmission */
  if(emac_dma_flag_get(EMAC_DMA_TBU_FLAG))
  {
    /* Clear TBUS ETHERNET DMA flag */
    emac_dma_flag_clear(EMAC_DMA_TBU_FLAG);
    /* Resume DMA transmission*/
    EMAC_DMA->tpd_bit.tpd = 0;
  }
  
  return SUCCESS;
}


/*******************************************************************************
* Function Name  : emac_getcurrenttxbuffer
* Description    : Return the address of the buffer pointed by the current descritor.
* Input          : None
* Output         : None
* Return         : Buffer address
*******************************************************************************/
u32 emac_getcurrenttxbuffer(void)
{
  /* Return Buffer address */
  return (dma_tx_desc_to_set->buf1addr);
}
