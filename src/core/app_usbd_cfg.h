
#include "lpc_types.h"
#include "error.h"
#include "usbd/usbd_rom_api.h"

#ifndef __APP_USB_CFG_H_
#define __APP_USB_CFG_H_

/* 
 * Manifest constants used by USBD ROM stack. These values SHOULD NOT BE
 * CHANGED for advance features which require usage of USB_CORE_CTRL_T
 * structure.  Since these are the values used for compiling USB stack.
 */

#define USB_MAX_IF_NUM          8	/*!< Max interface number used for building USBD ROM. DON'T CHANGE. */
#define USB_MAX_EP_NUM          6	/*!< Max number of EP used for building USBD ROM. DON'T CHANGE. */
#define USB_MAX_PACKET0         64	/*!< Max EP0 packet size used for building USBD ROM. DON'T CHANGE. */
#define USB_FS_MAX_BULK_PACKET  64	/*!< MAXP for FS bulk EPs used for building USBD ROM. DON'T CHANGE. */
#define USB_HS_MAX_BULK_PACKET  512	/*!< MAXP for HS bulk EPs used for building USBD ROM. DON'T CHANGE. */
#define USB_DFU_XFER_SIZE       2048	/*!< Max DFU transfer size used for building USBD ROM. DON'T CHANGE. */

#endif /* __APP_USB_CFG_H_ */
