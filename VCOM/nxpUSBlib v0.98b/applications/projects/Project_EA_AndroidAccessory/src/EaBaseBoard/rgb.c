/*****************************************************************************
 *
 *   Copyright(C) 2011, Embedded Artists AB
 *   All rights reserved.
 *
 ******************************************************************************
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * products. This software is supplied "AS IS" without any warranties.
 * Embedded Artists AB assumes no responsibility or liability for the
 * use of the software, conveys no license or title under any patent,
 * copyright, or mask work right to the product. Embedded Artists AB
 * reserves the right to make changes in the software without
 * notification. Embedded Artists AB also make no representation or
 * warranty that such application will be suitable for the specified
 * use without further testing or modification.
 **********************************************************************/
/******************************************************************************
 * Includes
 *****************************************************************************/

#include "lpc17xx_gpio.h"
#include "rgb.h"

/******************************************************************************
 * Defines and typedefs
 *****************************************************************************/

/******************************************************************************
 * External global variables
 *****************************************************************************/

/******************************************************************************
 * Local variables
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/

/******************************************************************************
 * Public Functions
 *****************************************************************************/

/******************************************************************************
 *
 * Description:
 *    Initialize RGB driver
 *
 *****************************************************************************/
void rgb_init (void)
{
    GPIO_SetDir( 2, 1, 1 );
    GPIO_SetDir( 0, (1<<26), 1 );
    GPIO_SetDir( 2, (1<<1), 1 );

}


/******************************************************************************
 *
 * Description:
 *    Set LED states
 *
 * Params:
 *    [in]  ledMask  - The mask is used to turn LEDs on or off
 *
 *****************************************************************************/
void rgb_setLeds (uint8_t ledMask)
{
    if ((ledMask & RGB_RED) != 0) {
        GPIO_SetValue( 2, 1);
    } else {
        GPIO_ClearValue( 2, 1 );
    }

    if ((ledMask & RGB_BLUE) != 0) {
        GPIO_SetValue( 0, (1<<26) );
    } else {
        GPIO_ClearValue( 0, (1<<26) );
    }

    if ((ledMask & RGB_GREEN) != 0) {
        GPIO_SetValue( 2, (1<<1) );
    } else {
        GPIO_ClearValue( 2, (1<<1) );
    }

}
